#ifndef UTIL_HH
#define UTIL_HH

#include <cstddef>
#include <cstdint>
#include <fstream>

namespace util {

struct chs_addr {
	size_t sector, cyl, head;
};

chs_addr lba_to_chs(uint64_t lba, size_t spt, size_t hpc);

void wr_8(std::ofstream &fout, uint8_t data);
void wr_str(std::ofstream &fout, char const *str);
void wr_16_le(std::ofstream &fout, uint16_t data);
void wr_24_le(std::ofstream &fout, uint32_t data);
void wr_32_le(std::ofstream &fout, uint32_t data);
void wr_64_le(std::ofstream &fout, uint64_t data);
void wr_16_be(std::ofstream &fout, uint16_t data);
void wr_24_be(std::ofstream &fout, uint16_t data);
void wr_32_be(std::ofstream &fout, uint32_t data);
void wr_64_be(std::ofstream &fout, uint64_t data);
void pad_align(std::ofstream &fout, size_t bndry, uint8_t data);

}

#endif
