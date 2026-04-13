#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <fstream>
#include <string>

namespace freewb
{
void readNulTerminatedField(std::ifstream &in, std::string &out);
bool readU32(std::ifstream &in, uint32_t &out);
bool readExact(std::ifstream &in, void *dst, std::streamsize len);

size_t utf8CharCount(const std::string &s);

const std::string userFreewbPath();

} // namespace freewb

#endif