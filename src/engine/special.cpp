#include "special.h"

#include <ctime>
#include <string>

namespace freewb
{

const char *const kDigitHan[] = {"〇", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};
const char *const kTensPrefixHan[] = {"", "", "二", "三", "四", "五", "六", "七", "八", "九", ""};
const char *const kWeekdayHan[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
const char *const kWeekdayEn[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
constexpr const char kTimeSpecChars[] = "YyMmDdHhSsWw";

const char *Special::name() const
{
    return "engine:special";
}

bool Special::available() const
{
    return available_;
}

void Special::changeAvailable()
{
    available_ = !available_;
}

bool Special::isTimeSpec(char c)
{
    for (const char spec : kTimeSpecChars)
    {
        if (c == spec)
        {
            return true;
        }
    }
    return false;
}

bool Special::scanTimePlaceholder(const std::string &text, std::size_t dollarPos, std::size_t &specPos)
{
    const std::size_t n = text.size();
    if (dollarPos >= n || text[dollarPos] != '$')
    {
        return false;
    }
    std::size_t j = dollarPos + 1;
    if (j < n && text[j] == '0')
    {
        ++j;
    }
    if (j >= n || !isTimeSpec(text[j]))
    {
        return false;
    }
    specPos = j;
    return true;
}

std::string Special::chineseYear(int year)
{
    std::string out;
    if (year <= 0)
    {
        return out;
    }
    out.reserve(16);
    for (int y = year, divisor = 1000; divisor > 0; divisor /= 10)
    {
        out.append(kDigitHan[y / divisor]);
        y %= divisor;
    }
    return out;
}

std::string Special::chineseBelow100(int value, bool zeroWhenZero)
{
    std::string out;
    if (value == 0)
    {
        if (zeroWhenZero)
        {
            out.append("零");
        }
        return out;
    }
    if (value >= 10)
    {
        const char *tens = kTensPrefixHan[value / 10];
        if (tens[0] != '\0')
        {
            out.append(tens);
        }
        out.append("十");
    }
    const int ones = value % 10;
    if (ones != 0)
    {
        out.append(kDigitHan[ones]);
    }
    return out;
}

std::string Special::expandTimeSpec(char spec, char next, const std::tm &tm, bool &consumedNext)
{
    consumedNext = false;
    const int year = tm.tm_year + 1900;
    const int month = tm.tm_mon + 1;
    const int day = tm.tm_mday;
    const int wday = tm.tm_wday;

    switch (spec)
    {
    case 'Y':
        return chineseYear(year);
    case 'y':
        return std::to_string(year);
    case 'M':
        if (next == 'I')
        {
            consumedNext = true;
            return chineseBelow100(tm.tm_min, true);
        }
        return chineseBelow100(month, false);
    case 'm':
        if (next == 'i')
        {
            consumedNext = true;
            return std::to_string(tm.tm_min);
        }
        return std::to_string(month);
    case 'D':
        return chineseBelow100(day, false);
    case 'd':
        return std::to_string(day);
    case 'H':
        return chineseBelow100(tm.tm_hour, true);
    case 'h':
        return std::to_string(tm.tm_hour);
    case 'S':
        return chineseBelow100(tm.tm_sec, true);
    case 's':
        return std::to_string(tm.tm_sec);
    case 'W':
        return (wday >= 0 && wday < 7) ? kWeekdayHan[wday] : std::string{};
    case 'w':
        return (wday >= 0 && wday < 7) ? kWeekdayEn[wday] : std::string{};
    default:
        return {};
    }
}

bool Special::hasTimePlaceholder(const std::string &text) const
{
    const std::size_t n = text.size();
    for (std::size_t i = 0; i + 1 < n; ++i)
    {
        std::size_t specPos = 0;
        if (scanTimePlaceholder(text, i, specPos))
        {
            return true;
        }
    }
    return false;
}

void Special::format(std::string &text) const
{
    if (!available_ || !hasTimePlaceholder(text))
    {
        return;
    }

    const std::time_t nowSec = std::time(nullptr);
    std::tm tm{};
    localtime_r(&nowSec, &tm);

    std::string out;
    out.reserve(text.size() + 32);

    const std::size_t n = text.size();
    for (std::size_t i = 0; i < n;)
    {
        if (text[i] != '$')
        {
            out.push_back(text[i++]);
            continue;
        }

        const std::size_t dollar = i;
        std::size_t specPos = 0;
        if (!scanTimePlaceholder(text, dollar, specPos))
        {
            out.push_back('$');
            ++i;
            continue;
        }

        const char spec = text[specPos];
        const char next = (specPos + 1 < n) ? text[specPos + 1] : '\0';
        bool consumedNext = false;
        std::string chunk = expandTimeSpec(spec, next, tm, consumedNext);
        if (chunk.empty())
        {
            out.push_back('$');
            for (std::size_t k = dollar + 1; k <= specPos; ++k)
            {
                out.push_back(text[k]);
            }
            i = specPos + 1;
            continue;
        }

        out.append(chunk);
        i = specPos + 1;
        if (consumedNext)
        {
            ++i;
        }
    }

    text = std::move(out);
}

} // namespace freewb
