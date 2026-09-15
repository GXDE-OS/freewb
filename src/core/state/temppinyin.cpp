#include "statemanager.h"

#include "candidatelist.h"
#include "enginemanager.h"
#include "freewb.h"
#include "key.h"
#include "log.h"

namespace freewb
{

TempPinyinState::TempPinyinState(StateManager *manager, Freewb *freewb) : manager_(manager), freewb_(freewb)
{
}

const char *TempPinyinState::name() const
{
    return "state:tempPinyin";
}

bool TempPinyinState::available() const
{
    return available_;
}

void TempPinyinState::changeAvailable()
{
    available_ = !available_;
}

std::vector<std::string> TempPinyinState::uninterestedEngines() const
{
    return {"engine:py", "engine:wbpy", "engine:en"};
}

bool TempPinyinState::begin(const std::string &lead)
{
    if (lead.empty() || freewb_ == nullptr)
    {
        return false;
    }
    EngineManager *const engines = freewb_->engineManager();
    CandidateList *const candidates = freewb_->candidateList();
    if (engines == nullptr || candidates == nullptr)
    {
        return false;
    }
    // 已有编码或候选时不进入临时拼音
    if (!candidates->preeditText().empty() || candidates->totalCandidateCount() > 0)
    {
        return false;
    }

    const char *const prevName = engines->currentEngineName();
    engines->changeEngine("engine:wbpy");
    const char *const nowName = engines->currentEngineName();
    if (nowName == nullptr || std::string(nowName) != "engine:wbpy")
    {
        return false;
    }
    restoreEngineName_ = prevName != nullptr ? prevName : std::string();

    candidates->setLead(lead);
    FREEWB_DEBUG("[{}] begin lead={} display={}", name(), lead, candidates->preeditText());
    return true;
}

void TempPinyinState::restoreEngine()
{
    if (!restoreEngineName_.empty())
    {
        freewb_->engineManager()->changeEngine(restoreEngineName_);
    }
    restoreEngineName_.clear();
    freewb_->candidateList()->setLead({});
}

void TempPinyinState::cancel()
{
    restoreEngine();
    freewb_->candidateList()->clear();
}

void TempPinyinState::refreshCandidates()
{
    freewb_->engineManager()->refreshEngineResult();
}

bool TempPinyinState::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    CandidateList *const candidates = freewb_->candidateList();

    if (Key::isModifierKeySym(keysym) || !Key::hasNoModifier(state))
    {
        return false;
    }

    if (keysym == FreewbKey_BackSpace)
    {
        if (candidates->inputCode().empty())
        {
            manager_->reset();
        }
        else
        {
            candidates->popInputCode();
            refreshCandidates();
        }
        return true;
    }

    const char *const key = Key::keySymToName(keysym);
    if (key != nullptr && key[0] >= 'a' && key[0] <= 'z' && key[1] == '\0')
    {
        candidates->setInputCode(candidates->inputCode() + key);
        refreshCandidates();
        return true;
    }

    // 已有编码后再按引导键：追加进缓冲（不是退出去出标点）
    if (key != nullptr && candidates->lead() == key && !candidates->inputCode().empty())
    {
        candidates->setInputCode(candidates->inputCode() + key);
        refreshCandidates();
        return true;
    }

    // 空格/回车/数字选词等：还原引擎回 idle，交给 Committer
    restoreEngine();
    manager_->enterIdleState();
    return false;
}

} // namespace freewb
