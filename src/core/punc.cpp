#include "punc.h"

#include <cstring>
#include <string>

#include "charwidth.h"
#include "freewb.h"
#include "key.h"
#include "settings.h"

namespace freewb
{

const PuncPairEntry Punc::kAutoPairList[] = {
    {"(", ")", "（", "）"}, {"[", "]", "【", "】"}, {"{", "}", "｛", "｝"},
    {"<", ">", "《", "》"}, {"\"", "\"", "“", "”"}, {"'", "'", "‘", "’"},
};

const PuncMapEntry Punc::kPuncMap[] = {
    {'.', {"。"}, 1},
    {',', {"，"}, 1},
    {'?', {"？"}, 1},
    {':', {"："}, 1},
    {';', {"；"}, 1},
    {'!', {"！"}, 1},
    {'\\', {"、"}, 1},
    {'<', {"《"}, 1},
    {'>', {"》"}, 1},
    {'(', {"（"}, 1},
    {')', {"）"}, 1},
    {'[', {"【", "「", "『"}, 3},
    {']', {"】", "」", "』"}, 3},
    {'{', {"｛"}, 1},
    {'}', {"｝"}, 1},
    {'~', {"～"}, 1},
    {'`', {"·"}, 1},
    {'^', {"……"}, 1},
    {'_', {"——"}, 1},
    {'"', {"“"}, 1},
    {'\'', {"‘"}, 1},
    {'#', {"＃"}, 1},
    {'$', {"￥"}, 1},
    {'%', {"％"}, 1},
};

Punc::Punc(Freewb *freewb) : freewb_(freewb)
{
    loadSettings();
}

void Punc::loadSettings()
{
    smartMarkEnabled_ = settings::instance().get_smartMark();
    autoHalfMarkAfterNum_ = settings::instance().get_autoToHalfMarkFlg();
    chinesePuncEnabled_ = settings::instance().get_chinesePuncFlg();
}

const char *Punc::name() const
{
    return "core:punc";
}

bool Punc::available() const
{
    return chinesePuncEnabled_;
}

void Punc::changeAvailable()
{
    chinesePuncEnabled_ = !chinesePuncEnabled_;
}

void Punc::toggleAutoPair()
{
    smartMarkEnabled_ = !smartMarkEnabled_;
}

bool Punc::isAsciiSymbolKey(FreewbKeySym sym)
{
    if (sym < FreewbKey_space || sym > static_cast<FreewbKeySym>(0x007e))
    {
        return false;
    }
    return !Key::isKey09(sym, FreewbKeyState_None) && !Key::isKeyAZ(sym, FreewbKeyState_None) &&
           !Key::isKeyaz(sym, FreewbKeyState_None) && sym != FreewbKey_space;
}

bool Punc::isDigitChar(const std::string &textChar)
{
    if (textChar.size() == 1 && textChar[0] >= '0' && textChar[0] <= '9')
    {
        return true;
    }

    if (textChar.size() == 3 && static_cast<unsigned char>(textChar[0]) == 0xef &&
        static_cast<unsigned char>(textChar[1]) == 0xbc)
    {
        const unsigned char third = static_cast<unsigned char>(textChar[2]);
        return third >= 0x90 && third <= 0x99;
    }

    return false;
}

const PuncMapEntry *Punc::lookupMap(char ascii)
{
    for (const PuncMapEntry &entry : kPuncMap)
    {
        if (entry.ascii == ascii)
        {
            return &entry;
        }
    }
    return nullptr;
}

const PuncPairEntry *Punc::lookupPair(FreewbKeySym sym, char pairKey[2])
{
    const char *name = Key::keySymToName(sym);
    const char key = (name != nullptr && name[0] != '\0' && name[1] == '\0') ? name[0] : static_cast<char>(sym);
    pairKey[0] = key;
    pairKey[1] = '\0';

    for (const PuncPairEntry &entry : kAutoPairList)
    {
        if ((entry.asciiLeft[0] == key && entry.asciiLeft[1] == '\0') ||
            (entry.asciiRight[0] == key && entry.asciiRight[1] == '\0'))
        {
            return &entry;
        }
    }
    return nullptr;
}

PuncPushResult Punc::convert(FreewbKeySym keysym, FreewbKeyState state)
{
    const FreewbKeySym sym = Key::normalizedKeySymbol(keysym, state);
    if (sym == FreewbKey_None)
    {
        return {};
    }

    CharWidth *const charWidth = freewb_->charWidth();
    if (charWidth != nullptr && charWidth->overridesChinesePunc(sym))
    {
        const std::string text = charWidth->convert(keysym, state);
        if (!text.empty())
        {
            return {{}, text};
        }
    }

    char pairKey[2] = {};
    const PuncPairEntry *pair = lookupPair(sym, pairKey);
    if (pair != nullptr)
    {
        const bool sameKey = std::strcmp(pair->asciiLeft, pair->asciiRight) == 0;
        const bool opening = sameKey || std::strcmp(pairKey, pair->asciiLeft) == 0;

        bool skipPair = false;
        if (chinesePuncEnabled_)
        {
            const PuncMapEntry *mapEntry = lookupMap(static_cast<char>(sym));
            skipPair = mapEntry != nullptr && mapEntry->variantCount > 1 && !(smartMarkEnabled_ && opening);
        }

        if (!skipPair)
        {
            const char *left = chinesePuncEnabled_ ? pair->chineseLeft : pair->asciiLeft;
            const char *right = chinesePuncEnabled_ ? pair->chineseRight : pair->asciiRight;

            if (smartMarkEnabled_)
            {
                lastPuncStack_.erase(pairKey[0]);
                return opening ? PuncPushResult{left, right} : PuncPushResult{{}, right};
            }

            if (sameKey)
            {
                if (lastPuncStack_.erase(pairKey[0]) > 0)
                {
                    return {{}, right};
                }
                lastPuncStack_[pairKey[0]] = pairKey[0];
                return {{}, left};
            }

            return std::strcmp(pairKey, pair->asciiLeft) == 0 ? PuncPushResult{{}, left} : PuncPushResult{{}, right};
        }
    }

    const char ascii = static_cast<char>(sym);
    if (chinesePuncEnabled_ && autoHalfMarkAfterNum_ && isDigitChar(freewb_->committer()->committedText(1)) &&
        (sym == FreewbKey_comma || sym == FreewbKey_period || sym == FreewbKey_semicolon))
    {
        return {{}, std::string(1, ascii)};
    }

    if (!isAsciiSymbolKey(sym))
    {
        if (charWidth == nullptr)
        {
            return {};
        }
        const std::string fallback = charWidth->convert(keysym, state);
        if (fallback.empty())
        {
            return {};
        }
        return {{}, fallback};
    }

    if (chinesePuncEnabled_)
    {
        const PuncMapEntry *mapEntry = lookupMap(ascii);
        if (mapEntry != nullptr && mapEntry->variantCount > 0 && mapEntry->variants[0] != nullptr)
        {
            return {{}, mapEntry->variants[0]};
        }
    }

    return {{}, std::string(1, ascii)};
}

bool Punc::shouldProcessKey(FreewbKeySym keysym, FreewbKeyState state) const
{
    const FreewbKeySym sym = Key::normalizedKeySymbol(keysym, state);
    if (sym == FreewbKey_None)
    {
        return false;
    }

    CharWidth *const charWidth = freewb_->charWidth();
    if (charWidth != nullptr && (charWidth->overridesChinesePunc(sym) || charWidth->isTopCommitKey(keysym, state)))
    {
        return true;
    }

    char pairKey[2] = {};
    if (lookupPair(sym, pairKey) != nullptr)
    {
        return true;
    }

    return isAsciiSymbolKey(sym);
}

void Punc::reset()
{
    lastPuncStack_.clear();
}

} // namespace freewb
