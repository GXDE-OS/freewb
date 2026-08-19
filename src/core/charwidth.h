#ifndef CHARWIDTH_H
#define CHARWIDTH_H

#include <string>

#include "ifreewb.h"
#include "keysym.h"

namespace freewb
{

class Freewb;

class CharWidth final : public IFreewb
{
public:
    explicit CharWidth(Freewb *freewb = nullptr);
    ~CharWidth() override = default;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    void loadSettings();

    bool spaceFullWhenCharHalf() const;
    void convertString(std::string &text) const;

    /** 全角开启且引擎未占用该字母时，经 commit 上屏以触发全角转换。 */
    bool processKey(FreewbKeySym keysym, FreewbKeyState state);

private:
    static const char *fullWidthForAscii(unsigned char ch);
    const char *fullWidthIfEnabled(unsigned char ch) const;
    void notifyToolbarProperty() const;

private:
    Freewb *freewb_ = nullptr;
    bool available_ = false;
    bool spaceFullWhenCharHalf_ = false;
};

} // namespace freewb

#endif /* CHARWIDTH_H */
