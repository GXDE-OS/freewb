#include "gb2312filter.h"

#include <cerrno>
#include <cstring>

#include "log.h"

namespace freewb
{

Gb2312Filter::Gb2312Filter()
{
    conv_ = iconv_open("GB2312", "UTF-8");
    if (conv_ == reinterpret_cast<iconv_t>(-1))
    {
        FREEWB_WARN("Gb2312Filter: iconv_open(GB2312, UTF-8) failed: {}", std::strerror(errno));
    }
}

Gb2312Filter::~Gb2312Filter()
{
    if (conv_ != reinterpret_cast<iconv_t>(-1))
    {
        iconv_close(conv_);
    }
}

bool Gb2312Filter::valid() const
{
    return conv_ != reinterpret_cast<iconv_t>(-1);
}

bool Gb2312Filter::isGb2312(const std::string &utf8Char) const
{
    if (!valid() || utf8Char.empty())
    {
        return true;
    }

    iconv(conv_, nullptr, nullptr, nullptr, nullptr);

    char *inbuf = const_cast<char *>(utf8Char.data());
    std::size_t inbytes = utf8Char.size();
    unsigned char outbuf[8];
    char *outptr = reinterpret_cast<char *>(outbuf);
    std::size_t outbytes = sizeof(outbuf);

    const std::size_t rc = iconv(conv_, &inbuf, &inbytes, &outptr, &outbytes);
    if (rc == static_cast<std::size_t>(-1))
    {
        return false;
    }
    return inbytes == 0;
}

} // namespace freewb
