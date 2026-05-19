#ifndef SPECIAL_H
#define SPECIAL_H

#include <ctime>
#include <string>

#include "ifreewb.h"

namespace freewb
{

/** 用户词库模板（$y、$0h 等）展开。 */
class Special final : public IFreewb
{
public:
    Special() = default;
    ~Special() override = default;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    bool hasTimePlaceholder(const std::string &text) const;
    void format(std::string &text) const;

private:
    static bool isTimeSpec(char c);
    static bool scanTimePlaceholder(const std::string &text, std::size_t dollarPos, std::size_t &specPos);
    static std::string chineseYear(int year);
    static std::string chineseBelow100(int value, bool zeroWhenZero);
    static std::string expandTimeSpec(char spec, char next, const std::tm &tm, bool &consumedNext);

    bool available_ = true;
};

} // namespace freewb

#endif
