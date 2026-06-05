#ifndef PUNC_H
#define PUNC_H

#include <string>
#include <unordered_map>

#include "ifreewb.h"
#include "keysym.h"

namespace freewb
{

class Freewb;

struct PuncPairEntry
{
    const char *asciiLeft;
    const char *asciiRight;
    const char *chineseLeft;
    const char *chineseRight;
};

struct PuncMapEntry
{
    char ascii;
    const char *variants[3];
    int variantCount;
};

struct PuncPushResult
{
    std::string before;
    std::string after;

    bool empty() const
    {
        return before.empty() && after.empty();
    }

    std::string joined() const
    {
        return before + after;
    }
};

class Punc final : public IFreewb
{
public:
    explicit Punc(Freewb *freewb);
    ~Punc() override = default;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    void loadSettings();
    void toggleSmartMark();

    PuncPushResult convert(FreewbKeySym keysym, FreewbKeyState state);
    bool shouldProcessKey(FreewbKeySym keysym, FreewbKeyState state) const;
    void reset();

private:
    static bool isAsciiSymbolKey(FreewbKeySym sym);
    static bool isDigitChar(const std::string &textChar);
    static const PuncPairEntry *lookupPair(FreewbKeySym sym, char pairKey[2]);
    static const PuncMapEntry *lookupMap(char ascii);

    static const PuncPairEntry kAutoPairList[];
    static const PuncMapEntry kPuncMap[];

    Freewb *freewb_;
    bool chinesePuncEnabled_ = true;
    bool smartMarkEnabled_ = true;
    bool autoHalfMarkAfterNum_ = false;
    std::unordered_map<char, char> lastPuncStack_;
};

} // namespace freewb

#endif /* PUNC_H */
