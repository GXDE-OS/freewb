#ifndef CANDIDATELIST_H
#define CANDIDATELIST_H

#include <string>
#include <vector>

#include "chttrans.h"

namespace freewb
{
class CandidateList
{
public:
    CandidateList();
    ~CandidateList();

    void prev();
    bool hasPrev() const;
    void next();
    bool hasNext() const;
    void setCursor(int cursor);
    int cursor() const;

    /** 当前页可见候选条数（与每页字数有关）。 */
    int size() const;
    /** 引擎本次返回的候选总条数。 */
    int totalCandidateCount() const;

    void clear();

    void setCandidates(std::vector<std::string> texts, std::vector<std::string> prompts);
    const std::string &selectCandidateText(int index) const;
    const std::string &firstVisibleCandidateOrPreedit() const;
    std::vector<std::string> candidateTexts() const;
    std::vector<std::string> candidatePrompts() const;

    void setPreeditText(const std::string &text);
    const std::string &preeditText() const;
    void popPreeditText();

private:
    void init();
    void syncVisiblePage();

private:
    int cursor_;
    std::vector<std::string> allTexts_;
    std::vector<std::string> allPrompts_;
    std::vector<std::string> currentPageTexts_;
    std::vector<std::string> currentPagePrompts_;
    int pageIndex_ = 0;
    int totalPages_ = 0;
    int wordCount_ = 5;
    std::string preeditText_;
    Chttrans chttrans_;
};
} // namespace freewb

#endif
