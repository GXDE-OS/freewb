#include <iostream>

#include "chttrans.h"

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

void ensureAvailable(freewb::Chttrans &cht)
{
    if (!cht.available())
    {
        cht.changeAvailable();
    }
}

bool test_simp_to_trad()
{
    freewb::Chttrans cht;
    ensureAvailable(cht);
    const std::string in = "汉";
    std::string out = in;
    cht.simpToTrad(out);
    std::cout << "  [simp->trad] in=\"" << in << "\" out=\"" << out << "\" expected=\"漢\"\n";
    CHECK(out == "漢", "simpToTrad(\"汉\") should be \"漢\"");
    return true;
}

bool test_disable_switch()
{
    freewb::Chttrans cht;
    ensureAvailable(cht);
    cht.changeAvailable();
    const std::string in = "汉";
    std::string out = in;
    cht.simpToTrad(out);
    std::cout << "  [disabled] in=\"" << in << "\" out=\"" << out << "\" expected=\"" << in << "\"\n";
    CHECK(out == in, "conversion should bypass when unavailable");
    return true;
}

} // namespace

int main()
{
    std::cout << "freewb-chttrans-test: OpenCC simp/trad conversion\n";
    const bool a = test_simp_to_trad();
    const bool b = test_disable_switch();
    std::cout << "  test_simp_to_trad: " << (a ? "ok" : "FAIL") << '\n';
    std::cout << "  test_disable_switch: " << (b ? "ok" : "FAIL") << '\n';
    if (a && b)
    {
        std::cout << "freewb-chttrans-test: all passed\n";
        return 0;
    }
    std::cerr << "freewb-chttrans-test: failed\n";
    return 1;
}
