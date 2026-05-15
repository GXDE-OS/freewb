#include "committer.h"

#include <utility>

#include "dbus.h"
#include "freewb.h"
#include "log.h"
#include "settings.h"

namespace freewb
{
Committer::Committer(CommitCallback commitCallback, Freewb *freewb) : commitCallback_(std::move(commitCallback)), freewb_(freewb)
{
    loadSettings();
}

Committer::~Committer() = default;

void Committer::loadSettings()
{
    secondRecodeKey_ = Key::keySymFromUniqueName(settings::instance().get_secondRecodeKey().c_str());
    thirdRecodeKey_ = Key::keySymFromUniqueName(settings::instance().get_thirdRecodeKey().c_str());

    prevPageKey_ = Key::keySymFromUniqueName(settings::instance().get_prevPageKey().c_str());
    nextPageKey_ = Key::keySymFromUniqueName(settings::instance().get_nextPageKey().c_str());
}

bool Committer::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (!commitCallback_)
    {
        return false;
    }
    CandidateList *candidates = freewb_->candidateList();
    if (candidates->size() == 0 && candidates->preeditText().empty())
    {
        return false;
    }

    // 上下翻页按键不支持上屏
    if ((keysym == prevPageKey_ && state == FreewbKeyState_None) || (keysym == nextPageKey_ && state == FreewbKeyState_None))
    {
        return false;
    }

    // 数字键支持上屏
    if (Key::isKey09(keysym, state))
    {
        return selectCandidate(static_cast<int>(keysym - FreewbKey_1));
    }

    // 二三重码上屏
    if ((keysym == secondRecodeKey_ && state == FreewbKeyState_None) ||
        (keysym == thirdRecodeKey_ && state == FreewbKeyState_None))
    {
        const int idx = keysym == secondRecodeKey_ ? 1 : 2;
        if (freewb_->candidateList()->size() <= idx)
        {
            return false;
        }
        commit(freewb_->candidateList()->selectCandidateText(idx));
        return true;
    }

    // 空格上屏
    if (keysym == FreewbKey_space && state == FreewbKeyState_None)
    {
        commit(candidates->firstVisibleCandidateOrPreedit());
        return true;
    }

    // 回车上屏
    if (keysym == FreewbKey_Return && state == FreewbKeyState_None)
    {
        commit(freewb_->candidateList()->preeditText());
        return true;
    }

    // 顶字上屏,上屏效果与普通上屏不同。如："你好,"
    if (Key::isSpecialCommitCharacter(keysym, state))
    {
        const char *keyString = Key::keySymToName(keysym);
        const std::pair<const char *, const char *> autoPair = freewb_->punc()->autoPair(keyString);
        if (autoPair.first != nullptr && autoPair.second != nullptr)
        {
            FREEWB_DEBUG("auto pair: {} {}", autoPair.first, autoPair.second);
            commit(autoPair.first + candidates->firstVisibleCandidateOrPreedit() + autoPair.second);
        }
        else
        {
            FREEWB_DEBUG("no auto pair: {}", keyString);
            commit(candidates->firstVisibleCandidateOrPreedit() + keyString);
        }
        return true;
    }
    return false;
}

const std::string &Committer::lastCommitString() const
{
    return lastCommitString_;
}

void Committer::commit(const std::string &text)
{
    FREEWB_DEBUG("committer will commit text : {}", text);
    lastCommitString_ = text;
    commitCallback_(text);
    freewb_->candidateList()->clear();
    freewb_->engineManager()->reset();
}

bool Committer::selectCandidate(int index)
{
    CandidateList *candidates = freewb_->candidateList();
    if (index < 0)
    {
        candidates->clear();
        freewb_->engineManager()->reset();
        return false;
    }
    else if (index >= freewb_->candidateList()->size())
    {
        commit(candidates->firstVisibleCandidateOrPreedit());
    }
    else
    {
        commit(freewb_->candidateList()->selectCandidateText(index));
    }
    return true;
}

} // namespace freewb