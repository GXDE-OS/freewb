#include "candidatelist.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

#include "log.h"
#include "settings.h"

namespace freewb
{
CandidateList::CandidateList() : cursor_(-1)
{
    init();
}

CandidateList::~CandidateList() = default;

void CandidateList::init()
{
    wordCount_ = settings::instance().get_candiWordCount();
    if (wordCount_ <= 0)
    {
        wordCount_ = 5;
    }

    FREEWB_DEBUG("will set word count={}", wordCount_);
}

void CandidateList::syncVisiblePage()
{
    currentPageTexts_.clear();
    currentPagePrompts_.clear();

    const int total = static_cast<int>(allTexts_.size());
    if (total == 0)
    {
        totalPages_ = 0;
        return;
    }

    totalPages_ = total % wordCount_ == 0 ? total / wordCount_ : total / wordCount_ + 1;
    if (totalPages_ <= 0)
    {
        totalPages_ = 1;
    }
    pageIndex_ = std::min(pageIndex_, totalPages_ - 1);

    const int start = pageIndex_ * wordCount_;
    const int end = std::min(start + wordCount_, total);
    const bool codeRemind = settings::instance().get_codeRemind();
    const bool simpTradOn = settings::instance().get_simpTradFlg();
    const int nVisible = end - start;
    const auto n = static_cast<std::size_t>(nVisible);

    currentPageTexts_.resize(n);
    currentPagePrompts_.resize(n);

    for (int i = start, j = 0; i < end; ++i, ++j)
    {
        const std::size_t src = static_cast<std::size_t>(i);
        const std::size_t dst = static_cast<std::size_t>(j);

        currentPageTexts_[dst] = allTexts_[src];
        chttrans_.simpToTrad(currentPageTexts_[dst]);

        const bool showTradHint = simpTradOn && currentPageTexts_[dst] != allTexts_[src];

        std::string prompt;
        if (showTradHint)
        {
            prompt.push_back('(');
            prompt.append(allTexts_[src]);
            prompt.push_back(')');
        }
        if (codeRemind)
        {
            prompt.append(allPrompts_[src]);
        }
        currentPagePrompts_[dst] = std::move(prompt);
    }
}

void CandidateList::prev()
{
    --pageIndex_;
    if (pageIndex_ < 0)
    {
        pageIndex_ = 0;
    }
    syncVisiblePage();

    FREEWB_DEBUG("will prev page, pageIndex={}", pageIndex_);
}

bool CandidateList::hasPrev() const
{
    if (pageIndex_ <= 0)
    {
        return false;
    }

    return true;
}

void CandidateList::next()
{
    ++pageIndex_;
    if (pageIndex_ > totalPages_)
    {
        pageIndex_ = totalPages_;
    }
    syncVisiblePage();

    FREEWB_DEBUG("will next page, pageIndex={}", pageIndex_);
}

bool CandidateList::hasNext() const
{
    FREEWB_DEBUG("pageIndex={}, totalPages={}", pageIndex_, totalPages_);
    if (pageIndex_ >= totalPages_ - 1)
    {
        return false;
    }

    return true;
}

void CandidateList::setCursor(int cursor)
{
    cursor_ = cursor;
}

int CandidateList::cursor() const
{
    return cursor_;
}

int CandidateList::size() const
{
    return static_cast<int>(currentPageTexts_.size());
}

int CandidateList::totalCandidateCount() const
{
    return static_cast<int>(allTexts_.size());
}

void CandidateList::clear()
{
    allTexts_.clear();
    allPrompts_.clear();
    currentPageTexts_.clear();
    currentPagePrompts_.clear();
    preeditText_.clear();
    cursor_ = -1;
    pageIndex_ = 0;
}

void CandidateList::setCandidates(std::vector<std::string> texts, std::vector<std::string> prompts)
{
    allTexts_ = std::move(texts);
    pageIndex_ = 0;
    allPrompts_ = std::move(prompts);
    allPrompts_.resize(allTexts_.size());
    syncVisiblePage();
}

const std::string &CandidateList::selectCandidateText(int index) const
{
    static const std::string kEmpty;
    if (index < 0 || index >= static_cast<int>(currentPageTexts_.size()))
    {
        return kEmpty;
    }
    return currentPageTexts_[static_cast<std::size_t>(index)];
}

const std::string &CandidateList::firstVisibleCandidateOrPreedit() const
{
    const std::string &word = selectCandidateText(0);
    if (!word.empty())
    {
        return word;
    }
    return preeditText();
}

std::vector<std::string> CandidateList::candidateTexts() const
{
    return currentPageTexts_;
}

std::vector<std::string> CandidateList::candidatePrompts() const
{
    return currentPagePrompts_;
}

void CandidateList::setPreeditText(const std::string &text)
{
    if (text.empty())
    {
        preeditText_.clear();
        return;
    }

    preeditText_ = text;
    cursor_ = static_cast<int>(preeditText_.size());
}

const std::string &CandidateList::preeditText() const
{
    return preeditText_;
}

void CandidateList::popPreeditText()
{
    if (preeditText_.empty())
    {
        return;
    }

    preeditText_.pop_back();
    if (preeditText_.empty())
    {
        clear();
    }

    cursor_ = static_cast<int>(preeditText_.size());
}
} // namespace freewb
