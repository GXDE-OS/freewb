#include "punc.h"

#include <cstring>
#include <string>

#include "charwidth.h"
#include "freewb.h"
#include "idbus.h"
#include "key.h"
#include "settings.h"

namespace freewb
{

const PuncPairEntry Punc::kAutoPairList[] = {
    {"(", ")", "（", "）"}, {"[", "]", "【", "】"}, {"{", "}", "{", "}"},
    {"<", ">", "《", "》"}, {"\"", "\"", "“", "”"}, {"'", "'", "‘", "’"},
};

const PuncMapEntry Punc::kPuncMap[] = {
    {'.', {"。"}, 1},
    {',', {"，"}, 1},
    {'?', {"？"}, 1},
    {':', {"："}, 1},
    {';', {"；"}, 1},
    {'\\', {"、"}, 1},
    {'/', {"、"}, 1},
    {'<', {"《"}, 1},
    {'>', {"》"}, 1},
    {'(', {"（"}, 1},
    {')', {"）"}, 1},
    {'[', {"【", "「", "『"}, 3},
    {']', {"】", "」", "』"}, 3},
    {'`', {"·"}, 1},
    {'^', {"……"}, 1},
    {'_', {"——"}, 1},
    {'"', {"“"}, 1},
    {'\'', {"‘"}, 1},
};

Punc::Punc(Freewb *freewb) : freewb_(freewb)
{
    // chinesePuncEnabled_ 为运行态，仅构造时初始化
    chinesePuncEnabled_ = settings::instance().get_chinesePunc();
    loadSettings();
    notifyToolbarProperty();
}

void Punc::notifyToolbarProperty() const
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return;
    }
    freewb_->dbusProxy()->callPanelUpdateProperties({chinesePuncEnabled_ ? "punc:active" : "punc:inactive"});
}

void Punc::loadSettings()
{
    puncAutoPairEnabled_ = settings::instance().get_puncAutoPair();
    autoHalfMarkAfterNum_ = settings::instance().get_autoToHalfPuncAfterNumber();
}

const char *Punc::name() const
{
    return "core:punc";
}

bool Punc::available() const
{
    return chinesePuncEnabled_;
}

bool Punc::effectiveChinesePunc() const
{
    if (!chinesePuncEnabled_)
    {
        return false;
    }
    if (freewb_ == nullptr || freewb_->engineManager() == nullptr)
    {
        return true;
    }
    const char *const engineName = freewb_->engineManager()->currentEngineName();
    return engineName == nullptr || std::strcmp(engineName, "engine:en") != 0;
}

void Punc::changeAvailable()
{
    chinesePuncEnabled_ = !chinesePuncEnabled_;
}

void Punc::toggleAutoPair()
{
    puncAutoPairEnabled_ = !puncAutoPairEnabled_;
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

bool Punc::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    Committer *const committer = freewb_->committer();
    CandidateList *const candidates = freewb_->candidateList();
    if (committer == nullptr || candidates == nullptr)
    {
        return false;
    }

    const bool hasCandidate = candidates->size() != 0 || !candidates->preeditText().empty();

    // 全角模式下数字须由输入法转换后送出，交给应用只会得到半角；
    // 半角模式下字面结果相同，但可使数字进入上屏历史，供“数字后自动半角标点”判断
    if (!hasCandidate && Key::isKey09(keysym, state))
    {
        committer->commit(std::string(1, static_cast<char>(keysym)));
        return true;
    }

    if (!shouldProcessKey(keysym, state))
    {
        return false;
    }

    const PuncPushResult result = convert(keysym, state);
    if (result.empty())
    {
        return false;
    }

    // 顶字上屏：先送出当前候选，再补上标点。如："你好，"
    if (hasCandidate)
    {
        const std::string visible = candidates->firstVisibleCandidateOrPreedit();
        const std::string code = candidates->firstVisibleCandidateFullCode();
        committer->commit(result.before + visible + result.after, code);
        return true;
    }

    committer->commit(result.joined());
    return true;
}

PuncPushResult Punc::convert(FreewbKeySym keysym, FreewbKeyState state)
{
    const FreewbKeySym sym = Key::normalizedKeySymbol(keysym, state);
    if (sym == FreewbKey_None)
    {
        return {};
    }

    CharWidth *const charWidth = freewb_->charWidth();
    if (sym == FreewbKey_backslash && charWidth != nullptr && charWidth->available())
    {
        return {{}, "\\"};
    }

    const bool useChinesePunc = effectiveChinesePunc();

    char pairKey[2] = {};
    const PuncPairEntry *pair = lookupPair(sym, pairKey);
    if (pair != nullptr)
    {
        const bool sameKey = std::strcmp(pair->asciiLeft, pair->asciiRight) == 0;
        const bool opening = sameKey || std::strcmp(pairKey, pair->asciiLeft) == 0;

        bool skipPair = false;
        if (useChinesePunc)
        {
            const PuncMapEntry *mapEntry = lookupMap(static_cast<char>(sym));
            skipPair = mapEntry != nullptr && mapEntry->variantCount > 1 && !(puncAutoPairEnabled_ && opening);
        }

        if (!skipPair)
        {
            const char *left = useChinesePunc ? pair->chineseLeft : pair->asciiLeft;
            const char *right = useChinesePunc ? pair->chineseRight : pair->asciiRight;

            if (puncAutoPairEnabled_)
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
    if (useChinesePunc && autoHalfMarkAfterNum_ && isDigitChar(freewb_->committer()->committedText(1)) &&
        (sym == FreewbKey_comma || sym == FreewbKey_period || sym == FreewbKey_semicolon))
    {
        return {{}, std::string(1, ascii)};
    }

    if (sym == FreewbKey_space)
    {
        if (charWidth != nullptr && (charWidth->available() || charWidth->spaceFullWhenCharHalf()))
        {
            return {{}, " "};
        }
        return {};
    }

    if (useChinesePunc)
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

    CharWidth *const charWidth = freewb_->charWidth();
    if (sym == FreewbKey_backslash && charWidth != nullptr && charWidth->available())
    {
        return true;
    }
    if (sym == FreewbKey_space && charWidth != nullptr && (charWidth->available() || charWidth->spaceFullWhenCharHalf()))
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
