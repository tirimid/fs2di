#include "cli.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>

#define BLK_SIZE_DEFAULT 512
#define PART_SCHEME_DEFAULT PS_MBR
#define PART_ALIGN_DEFAULT 2048

namespace cli {

struct opt_args {
	std::optional<std::filesystem::path> out;
	std::optional<size_t> blk_size;
	std::optional<unsigned char> part_scheme;
	std::optional<size_t> part_align;
};

static int short_flag(char f, opt_args &out, std::string const &p_name);
static int long_flag(std::string const &f, opt_args &out, std::string const &p_name);
static int long_opt(std::string const &o, opt_args &out);
static void usage(std::string const &p_name);

std::optional<args>
cli_parse(int argc, char const *argv[])
{
	opt_args opt_args;
	
	int i;
	for (i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "--")) {
			++i;
			break;
		} else if (argv[i][0] != '-')
			break;
		
		if (argv[i][1] != '-') {
			for (int j = 1; argv[i][j]; ++j) {
				if (short_flag(argv[i][j], opt_args, argv[0]))
					return std::nullopt;
			}
		} else {
			if (strchr(argv[i], '=')) {
				if (long_opt(argv[i] + 2, opt_args))
					return std::nullopt;
			} else {
				if (long_flag(argv[i] + 2, opt_args, argv[0]))
					return std::nullopt;
			}
		}
	}
	
	if (!opt_args.out) {
		std::cerr << "cli: no output file specified!\n";
		std::cerr << "\tuse --out=(file) to specify one\n";
		return std::nullopt;
	}
	
	if (!opt_args.blk_size)
		opt_args.blk_size = BLK_SIZE_DEFAULT;
	
	if (!opt_args.part_scheme)
		opt_args.part_scheme = PART_SCHEME_DEFAULT;
	
	if (!opt_args.part_align)
		opt_args.part_align = PART_ALIGN_DEFAULT;
	
	args args = {
		.out = *opt_args.out,
		.blk_size = *opt_args.blk_size,
		.part_scheme = *opt_args.part_scheme,
		.part_align = *opt_args.part_align,
	};
	
	while (i < argc)
		args.rest.push_back(argv[i++]);
	
	return args;
}

static int
short_flag(char f, opt_args &out, std::string const &p_name)
{
	switch (f) {
	case 'h':
		usage(p_name);
		exit(0);
	default:
		std::cerr << "cli: unrecognized flag - " << f << "!\n";
		return 1;
	}
}

static int
long_flag(std::string const &f, opt_args &out, std::string const &p_name)
{
	if (f == "help") {
		usage(p_name);
		exit(0);
	} else {
		std::cerr << "cli: unrecognized flag - " << f << "!\n";
		return 1;
	}
}

static int
long_opt(std::string const &o, opt_args &out)
{
	size_t key_len = 0;
	while (o[key_len] != '=')
		++key_len;
	
	std::string key = o.substr(0, key_len);
	std::string val = o.substr(key_len + 1, o.length() - key_len - 1);
	
	if (key == "blk-size") {
		long blk_size = strtol(val.c_str(), nullptr, 0);
		if (!blk_size) {
			std::cerr << "cli: invalid block size - " << val << "!\n";
			return 1;
		}
		if (blk_size != 512) {
			std::cerr << "cli: only 512-byte blocks are currently supported!\n";
			return 1;
		}
		out.blk_size = blk_size;
	} else if (key == "out")
		out.out = val;
	else if (key == "part-align") {
		long align = strtol(val.c_str(), nullptr, 0);
		if (!align) {
			std::cerr << "cli: invalid partition alignment - " << val << "!\n";
			return 1;
		}
		out.part_align = align;
	} else if (key == "part-scheme") {
		if (val == "mbr")
			out.part_scheme = PS_MBR;
		else if (val == "gpt")
			out.part_scheme = PS_GPT;
		else {
			std::cerr << "cli: unrecognized partitioning scheme - " << val << "!\n";
			return 1;
		}
	} else {
		std::cerr << "cli: unrecognized option - " << key << "!\n";
		return 1;
	}
	
	return 0;
}

static void
usage(std::string const &p_name)
{
	std::cout << "for more information on fs2di and examples, see the\n";
	std::cout << "documentation at https://tirimid.net/software/fs2di.html\n";
	std::cout << '\n';
	std::cout << "usage:\n";
	std::cout << '\t' << p_name << " <options> ...\n";
	std::cout << "options:\n";
	std::cout << "\t--blk-size=(size)     linear block size in bytes\n";
	std::cout << "\t--help, -h            display general help information\n";
	std::cout << "\t--out=(file)          file where disk image will be written\n";
	std::cout << "\t--part-align=(align)  minimum partition alignment in blocks\n";
	std::cout << "\t--part-scheme=(type)  partitioning scheme\n";
}

}
