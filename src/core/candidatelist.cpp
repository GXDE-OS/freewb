#include "candidatelist.h"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "log.h"
#include "settings.h"

namespace freewb
{
CandidateList::CandidateList(CandidatePayload &&candidatePayload)
    : cursor_(candidatePayload.cursor), allTexts_(std::move(candidatePayload.texts))
{
    init();
    syncVisiblePage();
}

CandidateList::CandidateList(std::vector<std::string> texts, int cursor) : cursor_(cursor), allTexts_(std::move(texts))
{
    init();
    syncVisiblePage();
}

CandidateList::CandidateList()
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
    const int total = static_cast<int>(allTexts_.size());
    currentPageTexts_.clear();
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
    currentPageTexts_.insert(currentPageTexts_.end(), allTexts_.begin() + start, allTexts_.begin() + end);
    for (auto &text : currentPageTexts_)
    {
        chttrans_.simpToTrad(text);
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

void CandidateList::clear()
{
    allTexts_.clear();
    currentPageTexts_.clear();
    preeditText_.clear();
    cursor_ = -1;
    pageIndex_ = 0;
}

void CandidateList::setCandidateTexts(std::vector<std::string> text)
{
    allTexts_ = std::move(text);
    pageIndex_ = 0;
    syncVisiblePage();
}

const std::string &CandidateList::selectCandidateText(int index) const
{
    if (index < 0 || index >= wordCount_)
    {
        return "";
    }
    return currentPageTexts_[index];
}

std::vector<std::string> CandidateList::candidateTexts() const
{
    return currentPageTexts_;
}

void CandidateList::setPreeditText(const std::string &text)
{
    if (text.empty())
    {
        preeditText_.clear();
        return;
    }

    preeditText_ = text;
    cursor_ = preeditText_.size();
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

    cursor_ = preeditText_.size();
}
} // namespace freewb
