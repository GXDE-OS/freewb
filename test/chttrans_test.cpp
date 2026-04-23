#include <iostream>

#include "chttrans.h"

namespace
{

#define CHECK(cond, msg)                                                                                           \
    do                                                                                                             \
    {                                                                                                              \
        if (!(cond))                                                                                               \
        {                                                                                                          \
            std::cerr << "FAIL " << __FILE__ << ':' << __LINE__ << ": " << (msg) << '\n';                       \
            return false;                                                                                          \
        }                                                                                                          \
    } while (0)

bool test_simp_to_trad()
{
    freewb::Chttrans cht;
    const std::string in = "汉";
    std::string out = in;
    cht.simpToTrad(out);
    std::cout << "  [simp->trad] in=\"" << in << "\" out=\"" << out
              << "\" expected=\"漢\"\n";
    CHECK(out == "漢", "simpToTrad(\"汉\") should be \"漢\"");
    return true;
}

bool test_trad_to_simp()
{
    freewb::Chttrans cht;
    const std::string in = "體";
    std::string out = in;
    cht.tradToSimp(out);
    std::cout << "  [trad->simp] in=\"" << in << "\" out=\"" << out
              << "\" expected=\"体\"\n";
    CHECK(out == "体", "tradToSimp(\"體\") should be \"体\"");
    return true;
}

bool test_disable_switch()
{
    freewb::Chttrans cht;
    cht.changeAvailable();
    const std::string in = "汉";
    std::string out = in;
    cht.simpToTrad(out);
    std::cout << "  [disabled] in=\"" << in << "\" out=\"" << out
              << "\" expected=\"" << in << "\"\n";
    CHECK(out == in, "conversion should bypass when unavailable");
    return true;
}

bool test_invalid_profile_path()
{
    freewb::Chttrans cht;
    const std::string in = "汉";
    std::string out = in;
    cht.simpToTrad(out);
    std::cout << "  [invalid-profile] available=" << (cht.available() ? "true" : "false")
              << " in=\"" << in << "\" out=\"" << out << "\" expected=\"" << in
              << "\"\n";
    CHECK(!cht.available(), "available() should be false for invalid profiles");    
    return true;
}

bool test_existing_dir_missing_profile_file()
{
    freewb::Chttrans cht;
    const std::string in = "體";
    std::string out = in;
    cht.tradToSimp(out);
    std::cout << "  [missing-file-in-existing-dir] available=" << (cht.available() ? "true" : "false")
              << " in=\"" << in << "\" out=\"" << out << "\" expected=\"" << in << "\"\n";
    CHECK(!cht.available(), "available() should be false when profile files are missing");
    return true;
}

} // namespace

int main()
{
    std::cout << "freewb-chttrans-test: OpenCC simp/trad conversion\n";
    const bool a = test_simp_to_trad();
    const bool b = test_trad_to_simp();
    const bool c = test_disable_switch();
    const bool d = test_invalid_profile_path();
    const bool e = test_existing_dir_missing_profile_file();
    std::cout << "  test_simp_to_trad: " << (a ? "ok" : "FAIL") << '\n';
    std::cout << "  test_trad_to_simp: " << (b ? "ok" : "FAIL") << '\n';
    std::cout << "  test_disable_switch: " << (c ? "ok" : "FAIL") << '\n';
    std::cout << "  test_invalid_profile_path: " << (d ? "ok" : "FAIL") << '\n';
    std::cout << "  test_existing_dir_missing_profile_file: "
              << (e ? "ok" : "FAIL") << '\n';
    if (a && b && c && d && e)
    {
        std::cout << "freewb-chttrans-test: all passed\n";
        return 0;
    }
    std::cerr << "freewb-chttrans-test: failed\n";
    return 1;
}
