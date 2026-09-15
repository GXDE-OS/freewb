#include "statemanager.h"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "candidatelist.h"
#include "committer.h"
#include "common.h"
#include "enginemanager.h"
#include "freewb.h"
#include "key.h"
#include "punc.h"

namespace freewb
{

/** 当前引擎是否在 state 的不感兴趣列表中。 */
#define CHECK_ENGINE(manager, state)                                                                                             \
    do                                                                                                                           \
    {                                                                                                                            \
        const Freewb *const _freewb = (manager).freewb_;                                                                         \
        if (_freewb != nullptr && _freewb->engineManager() != nullptr)                                                           \
        {                                                                                                                        \
            const char *const _engine = _freewb->engineManager()->currentEngineName();                                           \
            if (_engine != nullptr)                                                                                              \
            {                                                                                                                    \
                const std::vector<std::string> _uninterested = (state).uninterestedEngines();                                    \
                if (std::find(_uninterested.begin(), _uninterested.end(), _engine) != _uninterested.end())                       \
                {                                                                                                                \
                    return false;                                                                                                \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
    } while (0)

StateManager::StateManager(Freewb *freewb)
    : freewb_(freewb), current_(&idle_), add_(this, freewb), del_(this, freewb), tempEnglish_(this, freewb),
      tempPinyin_(this, freewb)
{
}

bool StateManager::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (current_ == &idle_)
    {
        return false;
    }

    if (keysym == FreewbKey_Escape && Key::hasNoModifier(state))
    {
        return false;
    }

    // 仅引导符时再按 lead：对齐旧版 ChooseByIndex(0)。
    // 旧版仅 lead 时 quick 候选即标点转换结果，故此处直接 punc 上屏并退出。
    CandidateList *const candidates = (freewb_ != nullptr) ? freewb_->candidateList() : nullptr;
    const char *const keyName = Key::keySymToName(keysym);
    if (Key::hasNoModifier(state) && candidates != nullptr && candidates->inputCode().empty() && keyName != nullptr &&
        candidates->lead() == keyName && (current_ == &tempEnglish_ || current_ == &tempPinyin_))
    {
        if (freewb_->committer() != nullptr && freewb_->punc() != nullptr)
        {
            const PuncPushResult result = freewb_->punc()->convert(keysym, state);
            if (!result.empty())
            {
                freewb_->committer()->commit(result.joined());
            }
        }
        reset();
        return true;
    }

    return current_->processKey(keysym, state);
}

bool StateManager::enterAddPhraseState(bool useClipboardText)
{
    CHECK_ENGINE(*this, add_);
    const bool entered = useClipboardText ? add_.beginFromClipboard() : add_.beginFromHistory();
    if (entered)
    {
        current_ = &add_;
    }
    return entered;
}

bool StateManager::enterDeletePhraseState(const std::string &wordText, const std::string &wordCode)
{
    CHECK_ENGINE(*this, del_);
    const bool entered = del_.begin(wordText, wordCode);
    if (entered)
    {
        current_ = &del_;
    }
    return entered;
}

bool StateManager::enterTempEnglishState(const std::string &commandPrefix)
{
    CHECK_ENGINE(*this, tempEnglish_);
    const bool entered = tempEnglish_.begin(commandPrefix);
    if (entered)
    {
        current_ = &tempEnglish_;
    }
    return entered;
}

bool StateManager::enterTempPinyinState(const std::string &lead)
{
    CHECK_ENGINE(*this, tempPinyin_);
    const bool entered = tempPinyin_.begin(lead);
    if (entered)
    {
        current_ = &tempPinyin_;
    }
    return entered;
}

void StateManager::reset()
{
    if (current_ == &idle_)
    {
        return;
    }

    current_->cancel();
    current_ = &idle_;
}

std::string filterNonHanziContent(const std::string &text)
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
        const std::uint32_t cp = (static_cast<std::uint32_t>(c0 & 0x0FU) << 12U) |
                                 (static_cast<std::uint32_t>(c1 & 0x3FU) << 6U) | static_cast<std::uint32_t>(c2 & 0x3FU);
        if ((cp >= 0x4E00U && cp <= 0x9FFFU) || (cp >= 0x3400U && cp <= 0x4DBFU))
        {
            out.append(ch);
        }
    }
    return out;
}

} // namespace freewb
