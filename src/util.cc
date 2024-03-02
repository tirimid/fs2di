#include "util.hh"

#include <cstring>
#include <iostream>

namespace util {

chs_addr
lba_to_chs(uint64_t lba, size_t spt, size_t hpc)
{
	chs_addr chs = {
		.sector = lba % spt + 1,
		.cyl = lba / (hpc * spt),
		.head = lba / spt % hpc,
	};
	
	return chs;
}

void
wr_8(std::ofstream &fout, uint8_t data)
{
	char data_c = data;
	fout.write(&data_c, 1);
}

void
wr_str(std::ofstream &fout, char const *str)
{
	fout.write(str, strlen(str));
}

void
wr_16_be(std::ofstream &fout, uint16_t data)
{
	wr_8(fout, data >> 8);
	wr_8(fout, data & 0xff);
}

void
wr_24_be(std::ofstream &fout, uint32_t data)
{
	wr_16_be(fout, data >> 8);
	wr_8(fout, data & 0xff);
}

void
wr_32_be(std::ofstream &fout, uint32_t data)
{
	wr_16_be(fout, data >> 16);
	wr_16_be(fout, data & 0xffff);
}

void
wr_64_be(std::ofstream &fout, uint64_t data)
{
	wr_32_be(fout, data >> 32);
	wr_32_be(fout, data & 0xffffffff);
}

void
wr_16_le(std::ofstream &fout, uint16_t data)
{
	wr_8(fout, data & 0xff);
	wr_8(fout, data >> 8);
}

void
wr_24_le(std::ofstream &fout, uint32_t data)
{
	wr_8(fout, data & 0xff);
	wr_16_le(fout, data >> 8);
}

void
wr_32_le(std::ofstream &fout, uint32_t data)
{
	wr_16_le(fout, data & 0xffff);
	wr_16_le(fout, data >> 16);
}

void
wr_64_le(std::ofstream &fout, uint64_t data)
{
	wr_16_le(fout, data & 0xffffffff);
	wr_16_le(fout, data >> 32);
}

void
pad_align(std::ofstream &fout, size_t bndry, uint8_t data)
{
	long pos = fout.tellp();
	if (pos < 0) {
		std::cerr << "util: file tell failed, the integrity of the generated image cannot be trusted!\n";
		return;
	}
	
	for (size_t i = 0, need = bndry - pos % bndry; i < need; ++i)
		wr_8(fout, data);
}

}
