#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>

#include "cli.hh"
#include "gpt.hh"
#include "mbr.hh"

int
main(int argc, char const *argv[])
{
	srand(time(nullptr));
	
	std::optional<cli::args> args = cli::cli_parse(argc, argv);
	if (!args) {
		std::cerr << "main: failed to parse CLI arguments!\n";
		return 1;
	}
	
	switch (args->part_scheme) {
	case cli::PS_MBR:
		if (mbr::mbr_create(*args)) {
			std::cerr << "main: failed to create MBR disk image!\n";
			return 1;
		}
		break;
	case cli::PS_GPT:
		if (gpt::gpt_create(*args)) {
			std::cerr << "main: failed to create GPT disk image!\n";
			return 1;
		}
		break;
	}
	
	return 0;
}
