#ifndef KEY_H
#define KEY_H

#include "keysym.h"

namespace freewb
{
class Key
{
public:
    static FreewbKeySym keySymFromString(const char *keyString);
    static const char *keySymToString(FreewbKeySym sym);
    static bool isModifierKeySym(FreewbKeySym sym);
    static bool isKeyAZ(FreewbKeySym sym, FreewbKeyState state);
    static bool isKeyaz(FreewbKeySym sym, FreewbKeyState state);
    static bool isKey09(FreewbKeySym sym, FreewbKeyState state);
};
} // namespace freewb

#endif