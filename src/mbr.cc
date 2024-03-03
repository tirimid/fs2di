#include "mbr.hh"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

#include "util.hh"

// wikipedia says these values are very common for SPT and HPC.
// assume the drive uses them.
#define CHS_SPT 63
#define CHS_HPC 16

namespace mbr {

struct part_tab_ent {
	uint8_t drive_attr;
	uint8_t first_head, first_sector_cyl, first_cyl;
	uint8_t type;
	uint8_t last_head, last_sector_cyl, last_cyl;
	uint32_t start_lba;
	uint32_t nsector;
};

struct mbr_arg {
	std::filesystem::path path;
	bool boot;
	uint8_t type;
};

static part_tab_ent pte_create(uint32_t lba, uint32_t nsector, bool bootable, uint8_t type);
static void wr_pte(std::ofstream &fout, part_tab_ent const &ent);
static std::optional<std::vector<mbr_arg>> mbr_args_from_rest(std::vector<std::string> const &rest);

int
mbr_create(cli::args const &args)
{
	// validate and parse command line rest args.
	if (!args.rest.size() || args.rest.size() % 3) {
		std::cerr << "mbr: rest args must take format of (<image> <boot/no-boot> <type>)+!\n";
		return 1;
	}
	
	auto mbr_args_opt = mbr_args_from_rest(args.rest);
	if (!mbr_args_opt) {
		std::cerr << "mbr: invalid rest args!\n";
		return 1;
	}
	std::vector<mbr_arg> mbr_args = *mbr_args_opt;
	
	if (mbr_args.size() > 4) {
		std::cerr << "mbr: MBR does not support >4 partitions!\n";
		return 1;
	}
	
	std::ofstream fout{args.out, std::ios::binary};
	if (!fout) {
		std::cerr << "mbr: failed to open file for writing - " << args.out << "!\n";
		return 1;
	}
	
	// skip first sector and come back later after obtaining partition
	// parameters.
	util::wr_8(fout, 0x0);
	util::pad_align(fout, args.blk_size, 0x0);
	
	// determine partition size parameters and copy images.
	uint32_t part_lba[4], part_nsector[4] = {0};
	for (size_t i = 0; i < mbr_args.size(); ++i) {
		std::ifstream fin{mbr_args[i].path, std::ios::binary};
		if (!fin) {
			std::cerr << "mbr: failed to open file for reading - " << mbr_args[i].path << "!\n";
			return 1;
		}
		
		fin.seekg(0, std::ios::end);
		long fin_size = fin.tellg();
		fin.seekg(0, std::ios::beg);
		if (fin_size < 0 || fin_size % args.blk_size) {
			std::cerr << "mbr: either could not tell file or size is non-block-multiple - " << mbr_args[i].path << "!\n";
			std::cerr << "\tyou cannot trust the integrity of the generated image\n";
		}
		
		// align partition before doing anything.
		util::pad_align(fout, args.part_align * args.blk_size, 0x0);
		long pos = fout.tellp();
		if (pos < 0) {
			std::cerr << "mbr: LBA-determining file tell failed!\n";
			return 1;
		}
		part_lba[i] = pos / args.blk_size;
		
		// copy filesystem image to partition.
		// assume filesystem image terminates on a sector boundary and
		// perform a very dumb (but fast) copy.
		// if it doesn't work, whatever - the user was warned.
		char *cp_buf = new char[args.blk_size];
		while (fin.read(cp_buf, args.blk_size)) {
			++part_nsector[i];
			fout.write(cp_buf, args.blk_size);
		}
		delete[] cp_buf;
	}
	
	// create MBR partition table data.
	std::vector<part_tab_ent> part_tab;
	for (size_t i = 0; i < mbr_args.size(); ++i) {
		part_tab_ent pte = pte_create(part_lba[i], part_nsector[i],
		                              mbr_args[i].boot,
		                              mbr_args[i].type);
		
		part_tab.push_back(pte);
	}
	
	// write MBR to disk image.
	fout.seekp(0, std::ios_base::beg);
	util::wr_8(fout, 0x0);
	util::pad_align(fout, args.blk_size - 72, 0x0);
	
	util::wr_32_le(fout, rand()); // generate a (probably) unique disk ID.
	util::wr_16_le(fout, 0x0); // reserved.
	
	for (part_tab_ent const &ent : part_tab)
		wr_pte(fout, ent);
	
	util::pad_align(fout, args.blk_size - 2, 0x0);
	util::wr_16_le(fout, 0xaa55); // boot signature.
	
	return 0;
}

static part_tab_ent
pte_create(uint32_t lba, uint32_t nsector, bool bootable, uint8_t type)
{
	util::chs_addr first = util::lba_to_chs(lba, CHS_SPT, CHS_HPC);
	util::chs_addr last = util::lba_to_chs(lba + nsector - 1, CHS_SPT, CHS_HPC);
	
	part_tab_ent ent = {
		.drive_attr = static_cast<uint8_t>(0x80 * bootable),
		.first_head = static_cast<uint8_t>(first.head),
		.first_sector_cyl = static_cast<uint8_t>(first.sector & 0x3f | (first.cyl & 0x300) >> 2),
		.first_cyl = static_cast<uint8_t>(first.cyl),
		.type = type,
		.last_head = static_cast<uint8_t>(last.head),
		.last_sector_cyl = static_cast<uint8_t>(last.sector & 0x3f | (last.cyl & 0x300) >> 2),
		.last_cyl = static_cast<uint8_t>(last.cyl),
		.start_lba = lba,
		.nsector = nsector,
	};
	
	return ent;
}

static void
wr_pte(std::ofstream &fout, part_tab_ent const &ent)
{
	util::wr_8(fout, ent.drive_attr);
	
	util::wr_8(fout, ent.first_head);
	util::wr_8(fout, ent.first_sector_cyl);
	util::wr_8(fout, ent.first_cyl);
	
	util::wr_8(fout, ent.type);
	
	util::wr_8(fout, ent.last_head);
	util::wr_8(fout, ent.last_sector_cyl);
	util::wr_8(fout, ent.last_cyl);
	
	util::wr_32_le(fout, ent.start_lba);
	util::wr_32_le(fout, ent.nsector);
}

static std::optional<std::vector<mbr_arg>>
mbr_args_from_rest(std::vector<std::string> const &rest)
{
	std::vector<mbr_arg> args;
	
	for (size_t i = 0; i < rest.size() / 3; ++i) {
		mbr_arg arg;
		
		if (rest[3 * i + 1] == "boot")
			arg.boot = true;
		else if (rest[3 * i + 1] == "no-boot")
			arg.boot = false;
		else {
			std::cerr << "mbr: expected boot/no-boot, found - " << rest[3 * i + 1] << "!\n";
			return std::nullopt;
		}
		
		long type = strtol(rest[3 * i + 2].c_str(), nullptr, 0);
		if (type <= 0x0 || type >= 0x100) {
			std::cerr << "mbr: invalid partition type - " << rest[3 * i + 2] << "!\n";
			return std::nullopt;
		}
		
		arg.type = type;
		arg.path = rest[3 * i];
		
		args.push_back(arg);
	}
	
	return args;
}

}
