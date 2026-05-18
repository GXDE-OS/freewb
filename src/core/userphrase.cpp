#include "userphrase.h"

#include <cstdint>

#include "common.h"
#include "sdbus_proxy.h"
#include "log.h"

namespace freewb
{

bool IdleState::processKey(FreewbKeySym /*keysym*/, FreewbKeyState /*state*/)
{
    return false;
}

void IdleState::cancel() {}

AddUserPhraseState::AddUserPhraseState(ipc::SDBusProxy *proxy, PhraseFromHistoryCallback phraseFromHistory,
                                       CalculateWubiPhraseCodeCallback &calculateWubiPhraseCode)
    : proxy_(proxy),
      calculateWubiPhraseCode_(calculateWubiPhraseCode),
      phraseFromHistoryCallback_(std::move(phraseFromHistory))
{
}

std::string AddUserPhraseState::utf8SuffixSkipChars(const std::string &text, int skipChars)
{
    const char *p = text.c_str();
    for (int i = 0; i < skipChars && *p != '\0'; ++i)
    {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c < 0x80U)
        {
            ++p;
        }
        else if ((c >> 5U) == 6U)
        {
            p += 2;
        }
        else if ((c >> 4U) == 14U)
        {
            p += 3;
        }
        else if ((c >> 3U) == 30U)
        {
            p += 4;
        }
        else
        {
            ++p;
        }
    }
    return std::string(p);
}

std::string AddUserPhraseState::filterNonHanziContent(const std::string &text)
{
    std::string out;
    const std::size_t charCount = MbDictionaryTable::utf8CharCount(text);
    out.reserve(text.size());
    for (std::size_t i = 0; i < charCount; ++i)
    {
        const std::string ch = MbDictionaryTable::utf8CharAt(text, i);
        if (ch.size() != 3U)
        {
            continue;
        }
        const unsigned char c0 = static_cast<unsigned char>(ch[0]);
        const unsigned char c1 = static_cast<unsigned char>(ch[1]);
        const unsigned char c2 = static_cast<unsigned char>(ch[2]);
        if ((c0 >> 4U) != 14U || (c1 >> 6U) != 2U || (c2 >> 6U) != 2U)
        {
            continue;
        }
        const std::uint32_t cp =
            (static_cast<std::uint32_t>(c0 & 0x0FU) << 12U) | (static_cast<std::uint32_t>(c1 & 0x3FU) << 6U) |
            static_cast<std::uint32_t>(c2 & 0x3FU);
        if ((cp >= 0x4E00U && cp <= 0x9FFFU) || (cp >= 0x3400U && cp <= 0x4DBFU))
        {
            out.append(ch);
        }
    }
    return out;
}

bool AddUserPhraseState::beginFromHistory()
{
    FREEWB_DEBUG("beginFromHistory");
    fromClipboard_ = false;
    if (proxy_ == nullptr || !phraseFromHistoryCallback_)
    {
        return false;
    }

    originalText_ = filterNonHanziContent(phraseFromHistoryCallback_(kPhraseMaxLength));
    sourceCharCount_ = static_cast<int>(MbDictionaryTable::utf8CharCount(originalText_));
    if (sourceCharCount_ <= 1)
    {
        originalText_.clear();
        return false;
    }

    phraseLen_ = kDefaultPhraseLen;
    refreshPhrase();
    if (wordText_.empty())
    {
        return false;
    }
    updatePhraseInfoToUI();
    return true;
}

void AddUserPhraseState::refreshPhrase()
{
    if (originalText_.empty() || sourceCharCount_ <= 0)
    {
        return;
    }

    const int skipChars = sourceCharCount_ - phraseLen_;
    wordText_ = utf8SuffixSkipChars(originalText_, skipChars);
    wordCode_.clear();
    wordCode_ = calculateWubiPhraseCode_(wordText_);
}

bool AddUserPhraseState::beginFromClipboard()   
{
    FREEWB_DEBUG("beginFromClipboard");
    fromClipboard_ = true;
    if (proxy_ == nullptr)
    {
        return false;
    }

    originalText_ = filterNonHanziContent(proxy_->callGetClipboardMethod());
    FREEWB_DEBUG("clip: {}", originalText_);
    sourceCharCount_ = static_cast<int>(MbDictionaryTable::utf8CharCount(originalText_));
    if (sourceCharCount_ <= 1 || sourceCharCount_ >= kPhraseMaxLength)
    {
        FREEWB_WARN("clip is too long or too short");
        originalText_.clear();
        return false;
    }

    phraseLen_ = sourceCharCount_;
    refreshPhrase();
    if (wordText_.empty())
    {
        originalText_.clear();
        return false;
    }
    updatePhraseInfoToUI();
    return true;
}

void AddUserPhraseState::cancel()
{
    if (proxy_ != nullptr)
    {
        proxy_->callAddUsrParseMethod(2, "", "");
    }
}

void AddUserPhraseState::updatePhraseInfoToUI()
{
    if (proxy_ == nullptr)
    {
        return;
    }

    // 如果词组编码为空且不是从剪贴板获取，则设置为自定义词组编码
    // 0: 显示待造词提示
    // 1: 确认造词
    // 2: 取消造词
    // 3: 自定义词组编码
    const int flg = (wordCode_.empty() && !fromClipboard_) ? 3 : 0;
    FREEWB_DEBUG("flg: {}, wordCode: {}, wordText: {}", flg, wordCode_, wordText_);
    proxy_->callAddUsrParseMethod(flg, wordCode_, wordText_);
}

bool AddUserPhraseState::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (proxy_ == nullptr)
    {
        return true;
    }

    switch (keysym)
    {
    case FreewbKey_Left:
        // 左移：增加词组长度
        if (state == FreewbKeyState_None)
        {
            const int maxChars = sourceCharCount_;
            if (phraseLen_ >= maxChars || phraseLen_ >= kPhraseMaxLength - 1)
            {
                return true;
            }
            ++phraseLen_;
            refreshPhrase();
            updatePhraseInfoToUI();
        }
        return true;
    case FreewbKey_Right:
        // 右移：减少词组长度
        if (state == FreewbKeyState_None)
        {
            if (phraseLen_ <= kDefaultPhraseLen)
            {
                return true;
            }
            --phraseLen_;
            refreshPhrase();
            updatePhraseInfoToUI();
        }
        return true;
    case FreewbKey_Return:
        // Ctrl + Return：更新编码
        if (state & FreewbKeyState_Ctrl)
        {
            proxy_->callAddUsrParseMethod(3, wordCode_, wordText_);
            return false;
        }
        // Return：确认
        if (state == FreewbKeyState_None)
        {
            proxy_->callAddUsrParseMethod(1, wordCode_, wordText_);
            return false;
        }
        return true;
    default:
        return true;
    }
}

DeleteUserPhraseState::DeleteUserPhraseState(ipc::SDBusProxy *proxy,
                                             CalculateWubiPhraseCodeCallback &calculateWubiPhraseCode)
    : proxy_(proxy), calculateWubiPhraseCode_(calculateWubiPhraseCode)
{
}

bool DeleteUserPhraseState::begin(const std::string &wordText)
{
    if (wordText.empty() || proxy_ == nullptr)
    {
        return false;
    }

    wordText_ = wordText;
    wordCode_.clear();
    wordCode_ = calculateWubiPhraseCode_(wordText_);
    proxy_->callDeleteUsrParseMethod(0, wordCode_, wordText_);
    return true;
}

void DeleteUserPhraseState::cancel()
{
    if (proxy_ != nullptr)
    {
        proxy_->callDeleteUsrParseMethod(2, "", "");
    }
}

bool DeleteUserPhraseState::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (proxy_ == nullptr)
    {
        return true;
    }

    if (keysym == FreewbKey_Return && state == FreewbKeyState_None)
    {
        proxy_->callDeleteUsrParseMethod(1, wordCode_, wordText_);
        return false;
    }
    return true;
}

UserPhrase::UserPhrase(ipc::SDBusProxy *proxy, CalculateWubiPhraseCodeCallback calculateWubiPhraseCode,
                       PhraseFromHistoryCallback phraseFromHistory)
    : calculateWubiPhraseCodeCallback_(std::move(calculateWubiPhraseCode)),
      current_(&idle_),
      add_(proxy, std::move(phraseFromHistory), calculateWubiPhraseCodeCallback_),
      del_(proxy, calculateWubiPhraseCodeCallback_)
{
}

bool UserPhrase::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (current_ == &idle_)
    {
        return false;
    }

    if (keysym == FreewbKey_Escape && state == FreewbKeyState_None)
    {
        return false;
    }

    if (!current_->processKey(keysym, state))
    {
        current_ = &idle_;
    }
    return true;
}

void UserPhrase::enterAddPhraseState(bool useClipboardText)
{
    const bool started = useClipboardText ? add_.beginFromClipboard() : add_.beginFromHistory();
    if (started)
    {
        current_ = &add_;
    }
}

void UserPhrase::enterDeletePhraseState(const std::string &wordText)
{
    if (del_.begin(wordText))
    {
        current_ = &del_;
    }
}

void UserPhrase::reset()
{
    if (current_ == &idle_)
    {
        return;
    }

    current_->cancel();
    current_ = &idle_;
}

} // namespace freewb
