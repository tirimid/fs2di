#ifndef CLI_HH
#define CLI_HH

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cli {

enum part_scheme {
	PS_MBR = 0,
	PS_GPT,
};

struct args {
	std::vector<std::string> rest;
	std::filesystem::path out;
	size_t blk_size;
	unsigned char part_scheme;
};

std::optional<args> cli_parse(int argc, char const *argv[]);

}

#endif
