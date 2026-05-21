#include "special.h"

#include <cctype>
#include <ctime>
#include <string>

namespace freewb
{

namespace
{

const char *const kDigitHan[] = {"〇", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};
const char *const kTensPrefixHan[] = {"", "", "二", "三", "四", "五", "六", "七", "八", "九", ""};
const char *const kWeekdayHan[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
const char *const kWeekdayEn[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *const kUnitHan[] = {"", "十", "百", "千", "万", "十", "百", "千", "亿"};
const char *const kMoneyUnitHan[] = {"", "拾", "佰", "仟", "万", "拾", "佰", "仟", "亿"};
const char *const kMoneyDigitHan[] = {"零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖", "拾"};
constexpr const char kMoneyYuan[] = "圆";
constexpr const char kMoneyJiao[] = "角";
constexpr const char kMoneyFen[] = "分";
constexpr const char kMoneyZheng[] = "整";
constexpr const char kDateYearSuffix[] = "年";
constexpr const char kDateMonthSuffix[] = "月";
constexpr const char kDateDaySuffix[] = "日";
constexpr const char kDateSep[] = "-";
constexpr const char kNumZeroHan[] = "零";
constexpr const char kNumTenHan[] = "十";
constexpr const char kTimeSpecChars[] = "YyMmDdHhSsWw";
constexpr int kMaxQuickDigitLen = 9;

std::string moneyHanZeroWith(const char *suffix)
{
    std::string label;
    label.append(kMoneyDigitHan[0]);
    label.append(suffix);
    return label;
}

const std::string kMoneyLabelZeroFen = moneyHanZeroWith(kMoneyFen);
const std::string kMoneyLabelZeroJiao = moneyHanZeroWith(kMoneyJiao);
const std::string kMoneyLabelZeroYuan = moneyHanZeroWith(kMoneyYuan);

void trimMoneySuffix(std::string &out)
{
    if (const std::size_t pos = out.find(kMoneyLabelZeroFen); pos != std::string::npos)
    {
        out.erase(pos);
    }
    if (const std::size_t pos = out.find(kMoneyLabelZeroJiao); pos != std::string::npos)
    {
        if (out.size() >= pos + 4U && out.substr(out.size() - 2U) == kMoneyJiao)
        {
            out.erase(out.size() - 2U);
        }
        else
        {
            out.erase(pos, kMoneyLabelZeroJiao.size());
        }
    }
    if (const std::size_t pos = out.find(kMoneyLabelZeroYuan); pos != std::string::npos)
    {
        out.erase(pos, kMoneyLabelZeroYuan.size());
    }
    const std::size_t yuanPos = out.find(kMoneyYuan);
    if (yuanPos != std::string::npos && yuanPos + std::char_traits<char>::length(kMoneyYuan) < out.size())
    {
        const std::size_t jiaoPos = out.find(kMoneyJiao, yuanPos + std::char_traits<char>::length(kMoneyYuan));
        const std::size_t fenPos = out.find(kMoneyFen, yuanPos + std::char_traits<char>::length(kMoneyYuan));
        if (jiaoPos == std::string::npos && fenPos != std::string::npos)
        {
            const std::string tail = out.substr(fenPos);
            out.resize(yuanPos + std::char_traits<char>::length(kMoneyYuan));
            out.append(kMoneyYuan);
            out.append(kMoneyDigitHan[0]);
            out.append(tail);
        }
    }
    if (out.size() >= 2U)
    {
        const std::string tail2 = out.substr(out.size() - 2U);
        if (tail2 == kMoneyYuan || tail2 == kMoneyJiao)
        {
            out.append(kMoneyZheng);
        }
    }
}

} // namespace

const char *Special::name() const
{
    return "core:special";
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
            out.append(kNumZeroHan);
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
        out.append(kNumTenHan);
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

bool Special::isAsciiDigitBody(const std::string &text)
{
    if (text.empty())
    {
        return false;
    }
    int dotCount = 0;
    for (const unsigned char c : text)
    {
        if (std::isdigit(c) != 0)
        {
            continue;
        }
        if (c == '.' && dotCount == 0)
        {
            ++dotCount;
            continue;
        }
        return false;
    }
    return true;
}

void Special::expandTimePlaceholders(std::string &text, const std::tm &tm)
{
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
        const std::string chunk = expandTimeSpec(spec, next, tm, consumedNext);
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

void Special::format(std::string &text) const
{
    if (!available_)
    {
        return;
    }

    if (hasTimePlaceholder(text))
    {
        const std::time_t nowSec = std::time(nullptr);
        std::tm tm{};
        localtime_r(&nowSec, &tm);
        expandTimePlaceholders(text, tm);
    }

    if (isAsciiDigitBody(text))
    {
        const std::string money = formatMoney(text);
        if (!money.empty())
        {
            text = money;
        }
    }
}

std::string Special::formatMoney(const std::string &body) const
{
    if (body.empty())
    {
        return {};
    }

    std::size_t intLen = 0;
    for (const char c : body)
    {
        if (c == '.')
        {
            break;
        }
        if (std::isdigit(static_cast<unsigned char>(c)) == 0)
        {
            return {};
        }
        ++intLen;
        if (intLen > static_cast<std::size_t>(kMaxQuickDigitLen))
        {
            return {};
        }
    }
    if (intLen == 0U)
    {
        return {};
    }

    std::string out;
    out.reserve(64);
    int i = static_cast<int>(intLen);
    std::size_t pos = 0;
    while (pos < body.size() && body[pos] != '.')
    {
        const char digit = body[pos];
        const char next = (pos + 1 < body.size() && body[pos + 1] != '.') ? body[pos + 1] : '\0';
        if (digit != '0' || (next != '\0' && next != '0' && i != 5))
        {
            out.append(kMoneyDigitHan[digit - '0']);
        }
        if (digit != '0' || i == 5)
        {
            out.append(kMoneyUnitHan[i - 1]);
        }
        ++pos;
        --i;
    }
    if (pos < body.size() && body[pos] == '.' && pos + 1 >= body.size())
    {
        return {};
    }

    out.append(kMoneyYuan);
    int fracIdx = 0;
    if (pos < body.size() && body[pos] == '.')
    {
        ++pos;
    }
    while (pos < body.size())
    {
        if (std::isdigit(static_cast<unsigned char>(body[pos])) == 0 || fracIdx >= 2)
        {
            return {};
        }
        out.append(kMoneyDigitHan[body[pos] - '0']);
        out.append(fracIdx == 0 ? kMoneyJiao : kMoneyFen);
        ++pos;
        ++fracIdx;
    }

    trimMoneySuffix(out);
    return out;
}

std::string Special::formatNumber(const std::string &body) const
{
    if (body.empty())
    {
        return {};
    }
    for (const char c : body)
    {
        if (std::isdigit(static_cast<unsigned char>(c)) == 0)
        {
            return {};
        }
    }
    if (body.size() > static_cast<std::size_t>(kMaxQuickDigitLen))
    {
        return {};
    }

    std::string out;
    out.reserve(64);
    int i = static_cast<int>(body.size());
    for (std::size_t pos = 0; pos < body.size(); ++pos)
    {
        const char c = body[pos];
        const char next = (pos + 1 < body.size()) ? body[pos + 1] : '\0';
        if (c == '0' && next != '0' && i != 5)
        {
            out.append(kNumZeroHan);
        }
        else if (c != '0')
        {
            out.append(kDigitHan[c - '0']);
        }
        if (c != '0' || i == 5)
        {
            out.append(kUnitHan[i - 1]);
        }
        --i;
    }
    if (out.empty())
    {
        return {};
    }
    return out;
}

std::string Special::formatNumberSimple(const std::string &body) const
{
    if (body.empty())
    {
        return {};
    }
    std::string out;
    out.reserve(body.size() * 4U);
    for (const char c : body)
    {
        if (std::isdigit(static_cast<unsigned char>(c)) == 0)
        {
            return {};
        }
        out.append(kDigitHan[c - '0']);
        if (out.size() > 64U)
        {
            return {};
        }
    }
    return out;
}

std::string Special::formatDate(const std::string &body, int flg) const
{
    if (body.empty() || flg < 0 || flg > 2)
    {
        return {};
    }

    std::string out;
    out.reserve(32);
    std::size_t pos = 0;
    int i = 0;
    bool hasYear = true;

    while (pos < body.size() && body[pos] != '.')
    {
        if (std::isdigit(static_cast<unsigned char>(body[pos])) == 0 || i >= 4)
        {
            return {};
        }
        if (flg != 0)
        {
            out.push_back(body[pos]);
        }
        else
        {
            out.append(kDigitHan[body[pos] - '0']);
        }
        ++pos;
        ++i;
    }
    if (i == 3)
    {
        return {};
    }

    const std::size_t yearStart = 0;
    if (i == 2)
    {
        if (body[yearStart] > '1' || (body[yearStart] == '1' && body[yearStart + 1] > '2'))
        {
            return {};
        }
        if (flg == 0)
        {
            out.clear();
            std::size_t t = yearStart;
            int ti = 0;
            while (t < pos)
            {
                if (body[t] != '0')
                {
                    if (ti == 0 && body[t] == '1')
                    {
                        out.append(kNumTenHan);
                    }
                    else
                    {
                        out.append(kDigitHan[body[t] - '0']);
                    }
                }
                ++t;
                ++ti;
            }
        }
    }

    if (i != 4)
    {
        hasYear = false;
        out.append(flg == 2 ? kDateSep : kDateMonthSuffix);
    }
    else
    {
        hasYear = true;
        out.append(flg == 2 ? kDateSep : kDateYearSuffix);
    }

    i = 0;
    const std::size_t monthStart = pos;
    if (pos < body.size() && body[pos] == '.')
    {
        ++pos;
    }
    while (pos < body.size() && body[pos] != '.')
    {
        if (std::isdigit(static_cast<unsigned char>(body[pos])) == 0 || i >= 2)
        {
            return {};
        }
        if (flg != 0)
        {
            out.push_back(body[pos]);
        }
        else if (i == 0 && pos + 1 < body.size() && body[pos + 1] != '.')
        {
            if (body[pos] > '1')
            {
                out.append(kDigitHan[body[pos] - '0']);
            }
            if (body[pos] > '0')
            {
                out.append(kNumTenHan);
            }
        }
        else if (body[pos] > '0')
        {
            out.append(kDigitHan[body[pos] - '0']);
        }
        ++pos;
        ++i;
    }
    if (i == 2)
    {
        if (hasYear && (body[monthStart] > '1' || (body[monthStart] == '1' && body[monthStart + 1] > '2')))
        {
            return {};
        }
        if (!hasYear && (body[monthStart] > '3' || (body[monthStart] == '3' && body[monthStart + 1] > '1')))
        {
            return {};
        }
    }

    if (hasYear)
    {
        out.append(flg == 2 ? kDateSep : kDateMonthSuffix);
    }
    else
    {
        if (pos >= body.size() || body[pos] != '.' || pos + 1 >= body.size())
        {
            if (flg != 2)
            {
                out.append(kDateDaySuffix);
            }
            return out;
        }
        return {};
    }

    if (pos < body.size() && body[pos] == '.')
    {
        ++pos;
    }
    const std::size_t dayStart = pos;
    i = 0;
    while (pos < body.size() && body[pos] != '.')
    {
        if (std::isdigit(static_cast<unsigned char>(body[pos])) == 0 || i >= 2)
        {
            return {};
        }
        if (flg != 0)
        {
            out.push_back(body[pos]);
        }
        else if (i == 0 && pos + 1 < body.size() && body[pos + 1] != '.')
        {
            if (body[pos] > '1')
            {
                out.append(kDigitHan[body[pos] - '0']);
            }
            if (body[pos] > '0')
            {
                out.append(kNumTenHan);
            }
        }
        else if (body[pos] > '0')
        {
            out.append(kDigitHan[body[pos] - '0']);
        }
        ++pos;
        ++i;
    }
    if (i == 0 || pos < body.size())
    {
        return {};
    }
    if (i == 2 && (body[dayStart] > '3' || (body[dayStart] == '3' && body[dayStart + 1] > '1')))
    {
        return {};
    }
    if (flg != 2)
    {
        out.append(kDateDaySuffix);
    }
    return out;
}

namespace
{

void appendUnique(std::vector<std::string> &out, const std::string &value)
{
    if (value.empty())
    {
        return;
    }
    for (const std::string &existing : out)
    {
        if (existing == value)
        {
            return;
        }
    }
    out.push_back(value);
}

} // namespace

std::vector<std::string> Special::formatSpecialValues(const std::string &body) const
{
    std::vector<std::string> out;
    if (!available_ || body.empty())
    {
        return out;
    }

    appendUnique(out, formatMoney(body));

    // 与旧版 quickTable[0]：matchMoney，否则 matchtimeManul(flg=0)
    if (const std::string number = formatNumber(body); !number.empty())
    {
        appendUnique(out, number);
    }
    else
    {
        appendUnique(out, formatDate(body, 0));
    }

    // quickTable[1]：matchNumber，否则 matchtimeManul(flg=1) 阿拉伯数字分段日期
    if (const std::string number = formatNumber(body); !number.empty())
    {
        appendUnique(out, number);
    }
    else
    {
        appendUnique(out, formatDate(body, 1));
    }

    // quickTable[2]：matchNumberSim，否则 matchtimeManul(flg=2)
    if (const std::string numberSimple = formatNumberSimple(body); !numberSimple.empty())
    {
        appendUnique(out, numberSimple);
    }
    else
    {
        appendUnique(out, formatDate(body, 2));
    }

    return out;
}

} // namespace freewb
