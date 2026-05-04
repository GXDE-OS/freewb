#include <cstring>
#include <iomanip>
#include <iostream>

#include "key.h"

namespace
{

bool streq(const char *a, const char *b)
{
    return std::strcmp(a, b) == 0;
}

#define CHECK(cond, msg)                                                                                                         \
    do                                                                                                                           \
    {                                                                                                                            \
        if (!(cond))                                                                                                             \
        {                                                                                                                        \
            std::cerr << "FAIL " << __FILE__ << ':' << __LINE__ << ": " << (msg) << '\n';                                        \
            return false;                                                                                                        \
        }                                                                                                                        \
    } while (0)

bool test_keySymFromString()
{
    std::cout << "\n== keySymFromString ==\n";

    std::cout << "  [case] null pointer -> expect FreewbKey_None\n";
    CHECK(freewb::Key::keySymFromUniqueName(nullptr) == FreewbKey_None, "null -> None");
    std::cout << "  [ok]   nullptr -> FreewbKey_None\n";

    std::cout << "  [case] \"\" -> expect FreewbKey_None\n";
    CHECK(freewb::Key::keySymFromUniqueName("") == FreewbKey_None, "empty -> None");
    std::cout << "  [ok]   \"\" -> FreewbKey_None\n";

    std::cout << "  [case] unknown label -> expect FreewbKey_None\n";
    CHECK(freewb::Key::keySymFromUniqueName("no_such_key_name") == FreewbKey_None, "unknown -> None");
    std::cout << "  [ok]   \"no_such_key_name\" -> FreewbKey_None\n";

    std::cout << "  [case] single-letter keys a / A\n";
    CHECK(freewb::Key::keySymFromUniqueName("a") == FreewbKey_a, "a");
    CHECK(freewb::Key::keySymFromUniqueName("A") == FreewbKey_A, "A");
    std::cout << "  [ok]   \"a\" -> FreewbKey_a, \"A\" -> FreewbKey_A\n";

    std::cout << "  [case] KEY_ESC / KEY_SPACE\n";
    CHECK(freewb::Key::keySymFromUniqueName("KEY_ESC") == FreewbKey_Escape, "KEY_ESC");
    CHECK(freewb::Key::keySymFromUniqueName("KEY_SPACE") == FreewbKey_space, "KEY_SPACE");
    std::cout << "  [ok]   KEY_ESC -> Escape, KEY_SPACE -> space\n";

    std::cout << "  [case] prefix \"KEY\" must not match any KEY_* entry\n";
    CHECK(freewb::Key::keySymFromUniqueName("KEY") == FreewbKey_None, "prefix KEY must not match KEY_*");
    std::cout << "  [ok]   \"KEY\" -> FreewbKey_None (no false prefix match)\n";

    std::cout << "  keySymFromString: finished\n";
    return true;
}

bool test_keySymToString()
{
    std::cout << "\n== keySymToString ==\n";

    std::cout << "  [case] FreewbKey_Escape -> name in table\n";
    const char *esc = freewb::Key::keySymToUniqueName(FreewbKey_Escape);
    CHECK(streq(esc, "KEY_ESC"), "Escape -> KEY_ESC");
    std::cout << "  [ok]   FreewbKey_Escape -> \"" << esc << "\"\n";

    std::cout << "  [case] FreewbKey_None -> first table name for None is empty\n";
    const char *none = freewb::Key::keySymToUniqueName(FreewbKey_None);
    CHECK(streq(none, ""), "None -> empty string in table");
    std::cout << "  [ok]   FreewbKey_None -> \"(empty)\"\n";

    std::cout << "  [case] unknown sym -> empty C string\n";
    const auto bogus = static_cast<FreewbKeySym>(0xdeadbeef);
    const char *unknown = freewb::Key::keySymToUniqueName(bogus);
    CHECK(streq(unknown, ""), "unknown sym -> empty");
    std::cout << "  [ok]   sym 0x" << std::hex << std::uppercase << bogus << std::dec << " -> \"(empty)\"\n";

    std::cout << "  keySymToString: finished\n";
    return true;
}

bool test_roundTrip()
{
    std::cout << "\n== roundTrip (sym -> string -> sym) ==\n";

    const auto sym = FreewbKey_Page_Down;
    const char *name = freewb::Key::keySymToUniqueName(sym);
    std::cout << "  [info] sym=FreewbKey_Page_Down (0x" << std::hex << std::uppercase << static_cast<unsigned long>(sym)
              << std::dec << ") name=\"" << (name ? name : "(null)") << "\"\n";

    CHECK(name && *name, "Page_Down has a name in table");
    const auto back = freewb::Key::keySymFromUniqueName(name);
    CHECK(back == sym, "toString then fromString");
    std::cout << "  [ok]   keySymFromString(name) == original sym\n";

    std::cout << "  roundTrip: finished\n";
    return true;
}

} // namespace

int main()
{
    std::cout << "freewb-key-test: Key::keySymFromUniqueName / keySymToString\n";

    const bool a = test_keySymFromString();
    const bool b = test_keySymToString();
    const bool c = test_roundTrip();

    if (a && b && c)
    {
        std::cout << "\nkey_test: all passed\n";
        return 0;
    }

    std::cerr << "\nkey_test: failed (keySymFromString=" << (a ? "ok" : "FAIL") << ", keySymToString=" << (b ? "ok" : "FAIL")
              << ", roundTrip=" << (c ? "ok" : "FAIL") << ")\n";
    return 1;
}