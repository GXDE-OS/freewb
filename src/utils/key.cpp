#include "key.h"

#include <cstddef>
#include <cstring>

namespace freewb
{
FreewbKeySym Key::keySymFromUniqueName(const char *uniqueName)
{
    if (!uniqueName)
    {
        return FreewbKey_None;
    }
    const auto n = sizeof(FreewbKeyNameList) / sizeof(FreewbKeyNameList[0]);
    for (std::size_t i = 0; i < n; ++i)
    {
        if (std::strcmp(uniqueName, FreewbKeyNameList[i].uniqueName) == 0)
        {
            return FreewbKeyNameList[i].sym;
        }
    }
    return FreewbKey_None;
}

const char *Key::keySymToName(FreewbKeySym sym)
{
    const auto n = sizeof(FreewbKeyNameList) / sizeof(FreewbKeyNameList[0]);
    for (std::size_t i = 0; i < n; ++i)
    {
        if (FreewbKeyNameList[i].sym == sym)
        {
            return FreewbKeyNameList[i].name;
        }
    }
    return "";
}

const char *Key::keySymToUniqueName(FreewbKeySym sym)
{
    const auto n = sizeof(FreewbKeyNameList) / sizeof(FreewbKeyNameList[0]);
    for (std::size_t i = 0; i < n; ++i)
    {
        if (FreewbKeyNameList[i].sym == sym)
        {
            return FreewbKeyNameList[i].uniqueName;
        }
    }
    return "";
}

bool Key::isModifierKeySym(FreewbKeySym sym)
{
    switch (sym)
    {
    case FreewbKey_Shift_L:
    case FreewbKey_Shift_R:
    case FreewbKey_Control_L:
    case FreewbKey_Control_R:
    case FreewbKey_Meta_L:
    case FreewbKey_Meta_R:
    case FreewbKey_Alt_L:
    case FreewbKey_Alt_R:
    case FreewbKey_Super_L:
    case FreewbKey_Super_R:
    case FreewbKey_Hyper_L:
    case FreewbKey_Hyper_R:
    case FreewbKey_Shift_Lock:
    case FreewbKey_Caps_Lock:
    case FreewbKey_Num_Lock:
    case FreewbKey_Scroll_Lock:
    case FreewbKey_ISO_Level3_Shift:
    case FreewbKey_ISO_Level5_Shift:
    case FreewbKey_ISO_Group_Shift:
        return true;
    default:
        return false;
    }
}

bool Key::isKeyAZ(FreewbKeySym sym, FreewbKeyState state)
{
    return (!state && sym >= FreewbKey_A && sym <= FreewbKey_Z);
}

bool Key::isKeyaz(FreewbKeySym sym, FreewbKeyState state)
{
    return (!state && sym >= FreewbKey_a && sym <= FreewbKey_z);
}

bool Key::isKey09(FreewbKeySym sym, FreewbKeyState state)
{
    return !state && ((sym >= FreewbKey_0 && sym <= FreewbKey_9) || (state && sym >= FreewbKey_KP_0 && sym <= FreewbKey_KP_9));
}

const char *Key::readKeyString(const char *str)
{
    const char *ctrl = strstr(str, "CTRL+");
    if (ctrl != NULL)
    {
        return ctrl + 5;
    }
    const char *alt = strstr(str, "ALT+");
    if (alt != NULL)
    {
        return alt + 4;
    }
    const char *shift = strstr(str, "SHIFT+");
    if (shift != NULL)
    {
        return shift + 6;
    }
    const char *super = strstr(str, "SUPER+");
    if (super != NULL)
    {
        return super + 6;
    }
    return str;
}

bool Key::isSpecialCommitCharacter(FreewbKeySym sym, FreewbKeyState state)
{
    return (!state && (sym == FreewbKey_comma || sym == FreewbKey_period || sym == FreewbKey_slash
        || sym == FreewbKey_semicolon || sym == FreewbKey_quoteright || sym == FreewbKey_bracketleft || sym == FreewbKey_bracketright || sym == FreewbKey_backslash));
}
} // namespace freewb