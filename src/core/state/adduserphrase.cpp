#include "statemanager.h"

#include "committer.h"
#include "common.h"
#include "enginemanager.h"
#include "freewb.h"
#include "idbus.h"
#include "key.h"
#include "log.h"

namespace freewb
{

AddUserPhraseState::AddUserPhraseState(StateManager *manager, Freewb *freewb) : manager_(manager), freewb_(freewb)
{
}

const char *AddUserPhraseState::name() const
{
    return "state:addPhrase";
}

bool AddUserPhraseState::available() const
{
    return available_;
}

void AddUserPhraseState::changeAvailable()
{
    available_ = !available_;
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

bool AddUserPhraseState::beginFromHistory()
{
    FREEWB_DEBUG("[{}] beginFromHistory", name());
    fromClipboard_ = false;
    if (freewb_ == nullptr || freewb_->committer() == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return false;
    }

    originalText_ = filterNonHanziContent(freewb_->committer()->committedText(kPhraseMaxLength));
    sourceCharCount_ = static_cast<int>(MbDictionaryTable::utf8CharCount(originalText_));
    if (sourceCharCount_ <= 1)
    {
        FREEWB_WARN("[{}] beginFromHistory: insufficient hanzi in commit history (count={})", name(), sourceCharCount_);
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
    if (originalText_.empty() || sourceCharCount_ <= 0 || freewb_ == nullptr || freewb_->engineManager() == nullptr)
    {
        return;
    }

    const int skipChars = sourceCharCount_ - phraseLen_;
    wordText_ = utf8SuffixSkipChars(originalText_, skipChars);
    wordCode_.clear();
    wordCode_ = freewb_->engineManager()->calculateWubiPhraseCode(wordText_);
}

bool AddUserPhraseState::beginFromClipboard()
{
    FREEWB_DEBUG("[{}] beginFromClipboard", name());
    fromClipboard_ = true;
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return false;
    }

    originalText_ = filterNonHanziContent(freewb_->dbusProxy()->callGetClipboardMethod());
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
    if (freewb_ != nullptr && freewb_->dbusProxy() != nullptr)
    {
        freewb_->dbusProxy()->callAddUsrParseMethod(2, "", "");
    }
}

void AddUserPhraseState::updatePhraseInfoToUI()
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
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
    freewb_->dbusProxy()->callAddUsrParseMethod(flg, wordCode_, wordText_);
}

bool AddUserPhraseState::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return true;
    }

    switch (keysym)
    {
    case FreewbKey_Left:
        // 左移：增加词组长度
        if (Key::hasNoModifier(state))
        {
            const int maxChars = sourceCharCount_;
            if (phraseLen_ < maxChars && phraseLen_ < kPhraseMaxLength - 1)
            {
                ++phraseLen_;
                refreshPhrase();
                updatePhraseInfoToUI();
            }
        }
        break;
    case FreewbKey_Right:
        // 右移：减少词组长度
        if (Key::hasNoModifier(state) && phraseLen_ > kDefaultPhraseLen)
        {
            --phraseLen_;
            refreshPhrase();
            updatePhraseInfoToUI();
        }
        break;
    case FreewbKey_Return:
        if (Key::modifiers(state) & FreewbKeyState_Ctrl)
        {
            freewb_->dbusProxy()->callAddUsrParseMethod(3, wordCode_, wordText_);
            break;
        }
        if (Key::hasNoModifier(state))
        {
            if (freewb_->engineManager() != nullptr)
            {
                (void)freewb_->engineManager()->addUserWord(wordCode_, wordText_);
            }
            freewb_->dbusProxy()->callAddUsrParseMethod(2, "", "");
            if (manager_ != nullptr)
            {
                manager_->enterIdleState();
            }
        }
        break;
    default:
        break;
    }
    return true;
}

std::vector<std::string> AddUserPhraseState::uninterestedEngines() const
{
    return {"engine:py", "engine:en"};
}

} // namespace freewb
