#include "committer.h"

#include <algorithm>
#include <utility>

#include "charwidth.h"
#include "common.h"
#include "freewb.h"
#include "key.h"
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

    // 以下上屏按键均要求不带修饰键
    if (!Key::hasNoModifier(state))
    {
        return false;
    }

    // 数字键支持上屏：1-9 对应第 1-9 个候选，0 对应第 10 个
    if (Key::isKey09(keysym, state))
    {
        const int index = (keysym == FreewbKey_0) ? 9 : static_cast<int>(keysym - FreewbKey_1);
        return selectCandidate(index);
    }

    // 二三重码上屏
    if (keysym == secondRecodeKey_ || keysym == thirdRecodeKey_)
    {
        const int idx = keysym == secondRecodeKey_ ? 1 : 2;
        if (freewb_->candidateList()->size() <= idx)
        {
            return false;
        }
        commit(freewb_->candidateList()->selectCandidateText(idx), freewb_->candidateList()->selectCandidateFullCode(idx));
        return true;
    }

    // 空格上屏
    if (keysym == FreewbKey_space)
    {
        if (candidates->size() > 0)
        {
            commit(candidates->selectCandidateText(0), candidates->selectCandidateFullCode(0));
        }
        else
        {
            commit(candidates->preeditText());
        }
        return true;
    }

    // 回车上屏
    if (keysym == FreewbKey_Return)
    {
        commit(freewb_->candidateList()->preeditText());
        return true;
    }

    return false;
}

const std::string &Committer::lastCommitString() const
{
    return lastCommitString_;
}

const std::string &Committer::lastCommitCode() const
{
    return lastCommitCode_;
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

void Committer::commit(const std::string &text, const std::string &code)
{
    std::string output = text;
    freewb_->charWidth()->convertString(output);

    FREEWB_DEBUG("committer will commit text : {}, code : {}", output, code);
    lastCommitString_ = output;
    lastCommitCode_ = code;
    appendCommittedText(output);

    const std::string preedit = freewb_->candidateList()->preeditText();
    const std::string inputCode = code.empty() ? preedit : code;
    commitCallback_(output);
    freewb_->engineManager()->addAutoPhrase(output, inputCode);

    freewb_->candidateList()->clear();
    freewb_->engineManager()->reset();
}

void Committer::handleCommittedBackspace()
{
    if (committedTexts_.empty())
    {
        return;
    }
    committedTexts_.pop_back();
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
        if (candidates->size() > 0)
        {
            commit(candidates->selectCandidateText(0), candidates->selectCandidateFullCode(0));
        }
        else
        {
            commit(candidates->firstVisibleCandidateOrPreedit());
        }
    }
    else
    {
        commit(freewb_->candidateList()->selectCandidateText(index), freewb_->candidateList()->selectCandidateFullCode(index));
    }
    return true;
}

} // namespace freewb
