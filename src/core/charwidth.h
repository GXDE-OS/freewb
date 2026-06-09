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

    bool spaceFullWhenCharHalf() const;
    void convertString(std::string &text) const;

private:
    static const char *fullWidthForAscii(unsigned char ch);
    const char *fullWidthIfEnabled(unsigned char ch) const;

private:
    bool available_ = false;
    bool spaceFullWhenCharHalf_ = false;
};

} // namespace freewb

#endif /* CHARWIDTH_H */
