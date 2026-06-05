#ifndef GB2312FILTER_H
#define GB2312FILTER_H

#include <iconv.h>

#include <cstddef>
#include <string>

namespace freewb
{

class Gb2312Filter
{
public:
    Gb2312Filter();
    ~Gb2312Filter();

    Gb2312Filter(const Gb2312Filter &) = delete;
    Gb2312Filter &operator=(const Gb2312Filter &) = delete;

    /** iconv 不可用或字符在 GB2312 内时为 true。 */
    bool isGb2312(const std::string &utf8Char) const;

private:
    bool valid() const;
    bool convertUtf8Char(const char *utf8, std::size_t len) const;

private:
    iconv_t conv_ = reinterpret_cast<iconv_t>(-1);
};

} // namespace freewb

#endif // GB2312FILTER_H
