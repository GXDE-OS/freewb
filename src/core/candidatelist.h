#ifndef CANDIDATELIST_H
#define CANDIDATELIST_H

#include "chttrans.h"
#include "types.h"

namespace freewb
{
class CandidateList
{
public:
    CandidateList(CandidatePayload &&candidatePayload);
    CandidateList(std::vector<std::string> texts, int cursor = 0);
    CandidateList();
    ~CandidateList();

    void prev();
    bool hasPrev() const;
    void next();
    bool hasNext() const;
    void setCursor(int cursor);
    int cursor() const;

    int size() const;

    void clear();

    void setCandidateTexts(std::vector<std::string> text);
    const std::string &selectCandidateText(int index) const;
    std::vector<std::string> candidateTexts() const;

    void setPreeditText(const std::string &text);
    const std::string &preeditText() const;
    void popPreeditText();

private:
    void init();
    void syncVisiblePage();

private:
    int cursor_;
    std::vector<std::string> allTexts_;
    std::vector<std::string> currentPageTexts_;
    int pageIndex_ = 0;
    int totalPages_ = 0;
    int wordCount_ = 5;
    std::string preeditText_;
    Chttrans chttrans_;
};
} // namespace freewb

#endif
