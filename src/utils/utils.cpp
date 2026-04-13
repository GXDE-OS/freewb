#include "utils.h"

#include <vector>

namespace freewb
{
void readNulTerminatedField(std::ifstream &in, std::string &out)
{
    uint32_t n = 0;
    if (!readU32(in, n))
    {
        return;
    }
    std::vector<char> buf(static_cast<size_t>(n) + 1U);
    in.read(buf.data(), static_cast<std::streamsize>(n) + 1);
    if (!in || static_cast<uint32_t>(in.gcount()) != n + 1U)
    {
        return;
    }
    out.assign(buf.data());
}

bool readU32(std::ifstream &in, uint32_t &out)
{
    unsigned char b[4];
    in.read(reinterpret_cast<char *>(b), 4);
    if (!in || in.gcount() != 4)
    {
        return false;
    }
    out = static_cast<uint32_t>(b[0]) | (static_cast<uint32_t>(b[1]) << 8) | (static_cast<uint32_t>(b[2]) << 16) | (static_cast<uint32_t>(b[3]) << 24);
    return true;
}

bool readExact(std::ifstream &in, void *dst, std::streamsize len)
{
    in.read(static_cast<char *>(dst), len);
    return in && in.gcount() == len;
}

const std::string userFreewbPath()
{
    const char *home = std::getenv("HOME");
    if (!home || !*home)
    {
        return {};
    }
    return std::string(home) + "/.local/freewb";
}

size_t utf8CharCount(const std::string &s)
{
    size_t n = 0;
    const char *p = s.c_str();
    while (*p)
    {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c < 0x80U)
        {
            ++p;
        }
        else if ((c >> 5) == 6U)
        {
            p += 2;
        }
        else if ((c >> 4) == 14U)
        {
            p += 3;
        }
        else if ((c >> 3) == 30U)
        {
            p += 4;
        }
        else
        {
            ++p;
            continue;
        }
        ++n;
    }
    return n;
}

} // namespace freewb