#include "committer.h"

#include <utility>

#include "settings.h"
#include "log.h"

namespace freewb
{
Committer::Committer(CommitCallback commitCallback, CandidateList *candidateList, EngineManager *engineManager, Punc *punc) : commitCallback_(std::move(commitCallback)), candidateList_(candidateList), engineManager_(engineManager), punc_(punc)
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
    if (!commitCallback_ || (candidateList_->size() == 0))
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
        int index = keysym - FreewbKey_1;
        if (index < 0)
        {
            candidateList_->clear();
            engineManager_->reset();
            return false;
        }
        else if (index >= candidateList_->size())
        {
            commit(candidateList_->selectCandidateText(0));
        }
        else
        {
            commit(candidateList_->selectCandidateText(index));
        }
        return true;
    }

    // 二三重码上屏
    if ((keysym == secondRecodeKey_ && state == FreewbKeyState_None) || (keysym == thirdRecodeKey_ && state == FreewbKeyState_None))
    {
        commit(candidateList_->selectCandidateText(keysym == secondRecodeKey_ ? 1 : 2));
        return true;
    }

    // 空格上屏
    if (keysym == FreewbKey_space && state == FreewbKeyState_None)
    {
        commit(candidateList_->selectCandidateText(0));
        return true;
    }

    // 回车上屏
    if (keysym == FreewbKey_Return && state == FreewbKeyState_None)
    {
        commit(candidateList_->preeditText());
        return true;
    }

    //顶字上屏,上屏效果与普通上屏不同。如："你好,"
    if (Key::isSpecialCommitCharacter(keysym, state))
    {
        const char *keyString = Key::keySymToName(keysym);
        const std::pair<const char *, const char *> autoPair = punc_->autoPair(keyString);
        if (autoPair.first != nullptr && autoPair.second != nullptr)
        {
            FREEWB_DEBUG("auto pair: {} {}", autoPair.first, autoPair.second);
            commit(autoPair.first + candidateList_->selectCandidateText(0) + autoPair.second);
        }
        else
        {
            FREEWB_DEBUG("no auto pair: {}", keyString);
            commit(candidateList_->selectCandidateText(0) + keyString);
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
    candidateList_->clear();
    engineManager_->reset();
}
} // namespace freewb