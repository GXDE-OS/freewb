/**
 * Special：用户词库时间模板展开（format）测试。
 */

#include <cstdio>
#include <ctime>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "special.h"

namespace
{

#define CHECK(cond, msg)                                                                                                         \
    do                                                                                                                           \
    {                                                                                                                            \
        if (!(cond))                                                                                                             \
        {                                                                                                                        \
            std::cerr << "FAIL " << __FILE__ << ':' << __LINE__ << ": " << (msg) << '\n';                                        \
            return false;                                                                                                        \
        }                                                                                                                        \
    } while (0)

std::string expectedTodayYmdArabic()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d年%d月%d日", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
    return buf;
}

std::string expectedTodayWeekdayHan()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    static const char *kWeekdayHan[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
    if (local.tm_wday >= 0 && local.tm_wday < 7)
    {
        return kWeekdayHan[local.tm_wday];
    }
    return {};
}

bool containsNoDollarPlaceholder(const std::string &s)
{
    for (std::size_t i = 0; i + 1 < s.size(); ++i)
    {
        if (s[i] != '$')
        {
            continue;
        }
        std::size_t j = i + 1;
        if (j < s.size() && s[j] == '0')
        {
            ++j;
        }
        if (j < s.size())
        {
            const char c = s[j];
            if (c == 'Y' || c == 'y' || c == 'M' || c == 'm' || c == 'D' || c == 'd' || c == 'H' || c == 'h' ||
                c == 'S' || c == 's' || c == 'W' || c == 'w')
            {
                return false;
            }
        }
    }
    return true;
}

bool test_plain_text_unchanged()
{
    freewb::Special special;
    std::string plain = "hello";
    special.format(plain);
    CHECK(plain == "hello", "plain text unchanged");
    return true;
}

bool test_no_placeholder_unchanged()
{
    freewb::Special special;
    std::string joke = "嘦巭好，兲嫑跑*_*!";
    special.format(joke);
    CHECK(joke == "嘦巭好，兲嫑跑*_*!", "text without $ placeholder unchanged");
    return true;
}

bool test_ymd_arabic_template()
{
    freewb::Special special;
    std::string ymd = "$y年$m月$d日";
    special.format(ymd);
    CHECK(ymd == expectedTodayYmdArabic(), "$y$m$d expands to today (Arabic digits)");
    CHECK(containsNoDollarPlaceholder(ymd), "no time placeholder left in ymd result");
    return true;
}

bool test_week_han_template()
{
    freewb::Special special;
    std::string week = "$W";
    special.format(week);
    CHECK(week == expectedTodayWeekdayHan(), "$W expands to Chinese weekday");
    CHECK(containsNoDollarPlaceholder(week), "no time placeholder left in week result");
    return true;
}

bool test_unavailable_skips_format()
{
    freewb::Special special;
    special.changeAvailable();
    std::string ymd = "$y年$m月$d日";
    special.format(ymd);
    CHECK(ymd == "$y年$m月$d日", "format skipped when Special unavailable");
    return true;
}

bool test_format_money_digit()
{
    freewb::Special special;
    std::string text = "1";
    special.format(text);
    CHECK(text.find("壹") != std::string::npos, "format(1) expands to money form");
    return true;
}

bool test_format_special_values_for_one()
{
    freewb::Special special;
    const std::vector<std::string> cands = special.formatSpecialValues("1");
    CHECK(!cands.empty(), "formatSpecialValues(1) not empty");
    CHECK(cands[0].find("壹") != std::string::npos, "first special value is money form");
    return true;
}

bool test_format_special_values_for_date()
{
    freewb::Special special;
    const std::vector<std::string> cands = special.formatSpecialValues("2005.8.26");
    CHECK(cands.size() == 3U, "formatSpecialValues(2005.8.26) has three candidates like legacy");
    return true;
}

} // namespace

int main()
{
    int failed = 0;
    if (!test_plain_text_unchanged())
    {
        ++failed;
    }
    if (!test_no_placeholder_unchanged())
    {
        ++failed;
    }
    if (!test_ymd_arabic_template())
    {
        ++failed;
    }
    if (!test_week_han_template())
    {
        ++failed;
    }
    if (!test_unavailable_skips_format())
    {
        ++failed;
    }
    if (!test_format_money_digit())
    {
        ++failed;
    }
    if (!test_format_special_values_for_one())
    {
        ++failed;
    }
    if (!test_format_special_values_for_date())
    {
        ++failed;
    }

    if (failed == 0)
    {
        std::cout << "special_test: all passed\n";
        return 0;
    }
    std::cerr << "special_test: " << failed << " failure(s)\n";
    return 1;
}
