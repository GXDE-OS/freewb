#ifndef CHARWIDTH_H
#define CHARWIDTH_H

#include <string>

#include "ifreewb.h"
#include "keysym.h"

namespace freewb
{

class CharWidth final : public IFreewb
{
public:
    CharWidth();
    ~CharWidth() override = default;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    void loadSettings();

    std::string convert(FreewbKeySym keysym, FreewbKeyState state) const;
    bool overridesChinesePunc(FreewbKeySym sym) const;
    bool isTopCommitKey(FreewbKeySym keysym, FreewbKeyState state) const;
    void convertString(std::string &text) const;

private:
    static bool isPrintableAscii(FreewbKeySym sym);
    static const char *fullWidthForAscii(unsigned char ch);
    const char *fullWidthIfEnabled(unsigned char ch) const;
    bool shouldConvert(FreewbKeySym sym) const;

private:
    bool available_ = false;
    bool spaceFullWhenCharHalf_ = false;
};

} // namespace freewb

#endif /* CHARWIDTH_H */
