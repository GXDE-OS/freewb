#include "charwidth.h"

#include "key.h"
#include "settings.h"

namespace freewb
{

static const char *const kCornerTrans[] = {
    "　", "！", "＂", "＃", "￥", "％", "＆", "＇", "（", "）", "＊", "＋", "，", "－", "．", "／", "０", "１", "２",
    "３", "４", "５", "６", "７", "８", "９", "：", "；", "＜", "＝", "＞", "？", "＠", "Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ",
    "Ｆ", "Ｇ", "Ｈ", "Ｉ", "Ｊ", "Ｋ", "Ｌ", "Ｍ", "Ｎ", "Ｏ", "Ｐ", "Ｑ", "Ｒ", "Ｓ", "Ｔ", "Ｕ", "Ｖ", "Ｗ", "Ｘ",
    "Ｙ", "Ｚ", "［", "＼", "］", "＾", "＿", "｀", "ａ", "ｂ", "ｃ", "ｄ", "ｅ", "ｆ", "ｇ", "ｈ", "ｉ", "ｊ", "ｋ",
    "ｌ", "ｍ", "ｎ", "ｏ", "ｐ", "ｑ", "ｒ", "ｓ", "ｔ", "ｕ", "ｖ", "ｗ", "ｘ", "ｙ", "ｚ", "｛", "｜", "｝", "～",
};

static constexpr int kCornerTransCount = static_cast<int>(sizeof(kCornerTrans) / sizeof(kCornerTrans[0]));

CharWidth::CharWidth()
{
    loadSettings();
}

void CharWidth::loadSettings()
{
    spaceFullWhenCharHalf_ = settings::instance().get_spaceFullWhenCharHalf();
    available_ = settings::instance().get_fullWidthFlg();
}

const char *CharWidth::name() const
{
    return "core:charwidth";
}

bool CharWidth::available() const
{
    return available_;
}

void CharWidth::changeAvailable()
{
    available_ = !available_;
}

bool CharWidth::isPrintableAscii(FreewbKeySym sym)
{
    return sym >= FreewbKey_space && sym <= static_cast<FreewbKeySym>(0x007e);
}

const char *CharWidth::fullWidthForAscii(unsigned char ch)
{
    if (ch < 0x20 || ch > 0x7e)
    {
        return nullptr;
    }
    const int index = static_cast<int>(ch) - 0x20;
    if (index < 0 || index >= kCornerTransCount)
    {
        return nullptr;
    }
    return kCornerTrans[index];
}

const char *CharWidth::fullWidthIfEnabled(unsigned char ch) const
{
    if (available_)
    {
        return fullWidthForAscii(ch);
    }
    if (spaceFullWhenCharHalf_ && ch == static_cast<unsigned char>(FreewbKey_space))
    {
        return fullWidthForAscii(ch);
    }
    return nullptr;
}

bool CharWidth::shouldConvert(FreewbKeySym sym) const
{
    if (!isPrintableAscii(sym))
    {
        return false;
    }
    return fullWidthIfEnabled(static_cast<unsigned char>(sym)) != nullptr;
}

std::string CharWidth::convert(FreewbKeySym keysym, FreewbKeyState state) const
{
    if (state != FreewbKeyState_None || !shouldConvert(keysym))
    {
        return {};
    }

    if (const char *converted = fullWidthIfEnabled(static_cast<unsigned char>(keysym)); converted != nullptr)
    {
        return converted;
    }
    return {};
}

bool CharWidth::overridesChinesePunc(FreewbKeySym sym) const
{
    return available_ && sym == FreewbKey_backslash;
}

bool CharWidth::isTopCommitKey(FreewbKeySym keysym, FreewbKeyState state) const
{
    const FreewbKeySym sym = Key::normalizedKeySymbol(keysym, state);
    if (sym == FreewbKey_None)
    {
        return false;
    }
    return shouldConvert(sym) && !Key::isKeyAZ(sym, FreewbKeyState_None) && !Key::isKeyaz(sym, FreewbKeyState_None) &&
           !Key::isKey09(sym, FreewbKeyState_None);
}

void CharWidth::convertString(std::string &text) const
{
    if (text.empty() || (!available_ && !spaceFullWhenCharHalf_))
    {
        return;
    }

    std::string result;
    result.reserve(text.size() * 2U);

    const char *cursor = text.c_str();
    while (*cursor != '\0')
    {
        const unsigned char ch = static_cast<unsigned char>(*cursor);
        if (ch < 0x80)
        {
            if (const char *converted = fullWidthIfEnabled(ch); converted != nullptr)
            {
                result.append(converted);
            }
            else
            {
                result.push_back(static_cast<char>(ch));
            }
            ++cursor;
            continue;
        }

        const std::size_t charLen = (ch & 0xe0) == 0xc0 ? 2U : (ch & 0xf0) == 0xe0 ? 3U : (ch & 0xf8) == 0xf0 ? 4U : 1U;
        result.append(cursor, charLen);
        cursor += static_cast<std::ptrdiff_t>(charLen);
    }

    text = std::move(result);
}

} // namespace freewb
