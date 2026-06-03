#ifndef KEY_H
#define KEY_H

#include "keysym.h"

namespace freewb
{
class Key
{
public:
    static FreewbKeySym keySymFromUniqueName(const char *uniqueName);
    static const char *keySymToName(FreewbKeySym sym);
    static const char *keySymToUniqueName(FreewbKeySym sym);

    static bool isModifierKeySym(FreewbKeySym sym);
    static FreewbKeyState modifierStateFromKeySym(FreewbKeySym sym);
    static bool isKeyAZ(FreewbKeySym sym, FreewbKeyState state);
    static bool isKeyaz(FreewbKeySym sym, FreewbKeyState state);
    static bool isKey09(FreewbKeySym sym, FreewbKeyState state);
    static const char *readKeyString(const char *str);
    static bool isSpecialCommitCharacter(FreewbKeySym sym, FreewbKeyState state);
};
} // namespace freewb

#endif