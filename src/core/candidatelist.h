#ifndef CANDIDATELIST_H
#define CANDIDATELIST_H

#include <string>
#include <vector>

namespace freewb
{
class Freewb;

class CandidateList
{
public:
    explicit CandidateList(Freewb *freewb);
    ~CandidateList();

    void loadSettings();
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

    void setCandidates(std::vector<std::string> texts, std::vector<std::string> prompts, std::vector<std::string> fullCodes = {});
    const std::string &selectCandidateText(int index) const;
    const std::string &selectCandidateFullCode(int index) const;
    /** 返回当前页中完整编码与 @p code 一致的首个候选文本（已含简繁等转换）；没有匹配时返回空串。 */
    const std::string &firstCandidateTextForFullCode(const std::string &code) const;
    /**
     * 当前候选中是否可将 @p code 视为已输完的全码：存在精确匹配，且没有以 @p code 为前缀的更长编码。
     * 用于避免拼音 qing→qingw 被误顶屏。
     */
    bool isTerminalExactCode(const std::string &code) const;
    const std::string &firstVisibleCandidateFullCode() const;
    const std::string &firstVisibleCandidateOrPreedit() const;
    std::vector<std::string> candidateTexts() const;
    std::vector<std::string> candidatePrompts() const;

    void setPreeditText(const std::string &text);
    const std::string &preeditText() const;
    void popPreeditText();

private:
    void syncVisiblePage();

private:
    int cursor_;
    std::vector<std::string> allTexts_;
    std::vector<std::string> allPrompts_;
    std::vector<std::string> allFullCodes_;
    std::vector<std::string> currentPageTexts_;
    std::vector<std::string> currentPagePrompts_;
    int pageIndex_ = 0;
    int totalPages_ = 0;
    int wordCount_ = 5;
    std::string preeditText_;
    Freewb *freewb_ = nullptr;
};
} // namespace freewb

#endif
