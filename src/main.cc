#include <iostream>
#include <optional>

#include "cli.hh"
#include "gpt.hh"

int
main(int argc, char const *argv[])
{
	std::optional<cli::args> args = cli::cli_parse(argc, argv);
	if (!args) {
		std::cerr << "main: failed to parse CLI arguments!\n";
		return 1;
	}
	
	switch (args->part_type) {
	case cli::PT_GPT:
		if (gpt::gpt_create(*args)) {
			std::cerr << "main: failed to create GPT disk image!\n";
			return 1;
		}
		break;
	}
	
	return 0;
}
