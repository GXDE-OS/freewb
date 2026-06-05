#include "committer.h"

#include <algorithm>
#include <utility>

#include "charwidth.h"
#include "common.h"
#include "freewb.h"
#include "key.h"
#include "log.h"
#include "punc.h"
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
    {
        Punc *punc = freewb_->punc();
        if (!punc->shouldProcessKey(keysym, state))
        {
            return false;
        }

        const std::string visible = candidates->firstVisibleCandidateOrPreedit();
        const PuncPushResult symbolPush = punc->convert(keysym, state);
        commit(symbolPush.before + visible + symbolPush.after);
        return true;
    }
}

const std::string &Committer::lastCommitString() const
{
    return lastCommitString_;
}

void Committer::appendCommittedText(const std::string &text)
{
    const std::size_t charCount = MbDictionaryTable::utf8CharCount(text);
    for (std::size_t i = 0; i < charCount; ++i)
    {
        const std::string textChar = MbDictionaryTable::utf8CharAt(text, i);
        if (textChar.empty())
        {
            continue;
        }
        if (committedTexts_.size() >= kMaxCommittedTextRecords)
        {
            committedTexts_.erase(committedTexts_.begin());
        }
        committedTexts_.push_back(textChar);
    }
}

std::string Committer::committedText(std::size_t charCount) const
{
    if (committedTexts_.empty() || charCount == 0)
    {
        return {};
    }
    const std::size_t n = std::min(charCount, committedTexts_.size());
    const std::size_t start = committedTexts_.size() - n;
    std::string text;
    for (std::size_t i = start; i < committedTexts_.size(); ++i)
    {
        text += committedTexts_[i];
    }
    return text;
}

void Committer::commit(const std::string &text)
{
    std::string output = text;
    freewb_->charWidth()->convertString(output);

    FREEWB_DEBUG("committer will commit text : {}", output);
    lastCommitString_ = output;
    appendCommittedText(output);
    commitCallback_(output);

    freewb_->candidateList()->clear();
    freewb_->engineManager()->reset();
}

void Committer::handleCommittedBackspace()
{
    if (!committedTexts_.empty())
    {
        committedTexts_.pop_back();
    }
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