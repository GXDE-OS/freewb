#include "committer.h"

#include <utility>

#include "settings.h"
#include "log.h"
#include "dbus.h"

namespace freewb
{
Committer::Committer(CommitCallback commitCallback, Freewb *freewb) : commitCallback_(std::move(commitCallback)), freewb_(freewb)
{
    loadSettings();
    connectDBusCallback();
}

Committer::~Committer() = default;

void Committer::loadSettings()
{
    secondRecodeKey_ = Key::keySymFromUniqueName(settings::instance().get_secondRecodeKey().c_str());
    thirdRecodeKey_ = Key::keySymFromUniqueName(settings::instance().get_thirdRecodeKey().c_str());

    prevPageKey_ = Key::keySymFromUniqueName(settings::instance().get_prevPageKey().c_str());
    nextPageKey_ = Key::keySymFromUniqueName(settings::instance().get_nextPageKey().c_str());
}

void Committer::connectDBusCallback()
{
    freewb_->sdbusProxy()->bindDBusSignalCallback([this](const char *member, int index) {
        if (std::strcmp(member, "SelectCandidate") != 0)
        {
            return;
        }
        this->commit(freewb_->candidateList()->selectCandidateText(index));
    });
}

bool Committer::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (!commitCallback_ || (freewb_->candidateList()->size() == 0))
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
            freewb_->candidateList()->clear();
            freewb_->engineManager()->reset();
            return false;
        }
        else if (index >= freewb_->candidateList()->size())
        {
            commit(freewb_->candidateList()->selectCandidateText(0));
        }
        else
        {
            commit(freewb_->candidateList()->selectCandidateText(index));
        }
        return true;
    }

    // 二三重码上屏
    if ((keysym == secondRecodeKey_ && state == FreewbKeyState_None) || (keysym == thirdRecodeKey_ && state == FreewbKeyState_None))
    {
        commit(freewb_->candidateList()->selectCandidateText(keysym == secondRecodeKey_ ? 1 : 2));
        return true;
    }

    // 空格上屏
    if (keysym == FreewbKey_space && state == FreewbKeyState_None)
    {
        commit(freewb_->candidateList()->selectCandidateText(0));
        return true;
    }

    // 回车上屏
    if (keysym == FreewbKey_Return && state == FreewbKeyState_None)
    {
        commit(freewb_->candidateList()->preeditText());
        return true;
    }

    //顶字上屏,上屏效果与普通上屏不同。如："你好,"
    if (Key::isSpecialCommitCharacter(keysym, state))
    {
        const char *keyString = Key::keySymToName(keysym);
        const std::pair<const char *, const char *> autoPair = freewb_->punc()->autoPair(keyString);
        if (autoPair.first != nullptr && autoPair.second != nullptr)
        {
            FREEWB_DEBUG("auto pair: {} {}", autoPair.first, autoPair.second);
            commit(autoPair.first + freewb_->candidateList()->selectCandidateText(0) + autoPair.second);
        }
        else
        {
            FREEWB_DEBUG("no auto pair: {}", keyString);
            commit(freewb_->candidateList()->selectCandidateText(0) + keyString);
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
} // namespace freewb