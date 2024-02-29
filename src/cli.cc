#include "cli.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace cli {

struct opt_args {
	std::optional<std::filesystem::path> out;
	std::optional<size_t> blk_size;
	std::optional<unsigned char> part_type;
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
	
	if (!opt_args.blk_size) {
		std::cerr << "cli: no block size specified!\n";
		std::cerr << "\tuse --blk-size=(size) to specify one\n";
		return std::nullopt;
	}
	
	if (!opt_args.part_type) {
		std::cerr << "cli: no partition scheme type specified!\n";
		std::cerr << "\tuse --part-type=(type) to specify one\n";
		return std::nullopt;
	}
	
	args args = {
		.out = *opt_args.out,
		.blk_size = *opt_args.blk_size,
		.part_type = *opt_args.part_type,
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
		int blk_size = atoi(val.c_str());
		if (!blk_size) {
			std::cerr << "cli: invalid block size - " << val << "!\n";
			return 1;
		}
		out.blk_size = blk_size;
	} else if (key == "out")
		out.out = val;
	else if (key == "part-type") {
		if (val == "gpt")
			out.part_type = PT_GPT;
		else {
			std::cerr << "cli: unrecognized part-type - " << val << "!\n";
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
	std::cout << "usage:\n";
	std::cout << '\t' << p_name << " <options> <files...>\n";
	std::cout << "options:\n";
	std::cout << "\t--blk-type=(size)   linear block size\n";
	std::cout << "\t--help, -h          display general help information\n";
	std::cout << "\t--out=(file)        file where disk image will be written\n";
	std::cout << "\t--part-type=(type)  partition scheme type\n";
}

}
