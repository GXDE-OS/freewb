#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "candidatelist.h"
#include "chttrans.h"
#include "log.h"
#include "settings.h"
namespace
{

void setFromCommits(freewb::CandidateList &cl, std::vector<std::string> texts)
{
    std::vector<std::string> attrs(texts.size());
    cl.setCandidates(std::move(texts), std::move(attrs));
}

void pushRow(std::vector<std::string> &texts, std::vector<std::string> &attrs, std::string text,
             std::string attr)
{
    texts.push_back(std::move(text));
    attrs.push_back(std::move(attr));
}

freewb::CandidateList makeList(freewb::Chttrans *chttrans)
{
    return freewb::CandidateList(chttrans);
}

void fail(const char *msg)
{
    std::cerr << "FAIL: " << msg << '\n';
    std::abort();
}

#define REQUIRE(cond, msg)                                                                                                       \
    do                                                                                                                           \
    {                                                                                                                            \
        if (!(cond))                                                                                                             \
        {                                                                                                                        \
            fail(msg);                                                                                                           \
        }                                                                                                                        \
    } while (0)

std::string expectedTodayYmdArabic()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d年%d月%d日", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
    return buf;
}

bool chttransConvertsSimpToTrad(freewb::Chttrans &chttrans)
{
    if (!chttrans.available())
    {
        return false;
    }
    std::string sample = "汉";
    chttrans.simpToTrad(sample);
    return sample == "漢";
}

bool promptHasTradHint(const std::string &prompt)
{
    return prompt.size() >= 2 && prompt.front() == '(' && prompt.find(')') != std::string::npos;
}

} // namespace

int main()
{
    FreewbLog log("/tmp/freewb-candidate-list-test.log");

    freewb::Chttrans chttrans;
    const bool openccOk = chttransConvertsSimpToTrad(chttrans);

    const int wc = settings::instance().get_candiWordCount();
    REQUIRE(wc > 0, "candiWordCount should be positive");

    // 默认构造 + clear（无 Special）
    {
        freewb::CandidateList cl = makeList(&chttrans);
        cl.clear();
        REQUIRE(cl.size() == 0, "size after clear");
        REQUIRE(cl.candidateTexts().empty(), "candidateTexts empty");
    }

    // setCandidates：单条、合法下标 select
    {
        freewb::CandidateList cl = makeList(&chttrans);
        setFromCommits(cl, {"hello"});
        REQUIRE(cl.size() == 1, "single item size");
        REQUIRE(cl.candidateTexts().size() == 1u, "candidateTexts size");
        REQUIRE(cl.selectCandidateText(0) == "hello", "selectCandidateText(0)");
    }

    // cursor + 首屏切片
    {
        freewb::CandidateList cl = makeList(&chttrans);
        setFromCommits(cl, {u8"\u7532", u8"\u4e59"});
        cl.setCursor(42);
        REQUIRE(cl.cursor() == 42, "cursor from setCursor");
        REQUIRE(static_cast<int>(cl.candidateTexts().size()) == std::min(wc, 2), "first page width");
        cl.setCursor(0);
        REQUIRE(cl.cursor() == 0, "setCursor");
    }

    // 多页：total = 2*wc + 3 => 三页，末页 3 条
    {
        const int total = 2 * wc + 3;
        std::vector<std::string> all;
        all.reserve(static_cast<size_t>(total));
        for (int i = 0; i < total; ++i)
        {
            all.push_back("c" + std::to_string(i));
        }

        freewb::CandidateList cl = makeList(&chttrans);
        setFromCommits(cl, std::move(all));
        REQUIRE(static_cast<int>(cl.candidateTexts().size()) == wc, "page 0 width");
        REQUIRE(cl.hasPrev() == false, "page 0 has no prev");
        REQUIRE(cl.hasNext() == true, "page 0 hasNext");

        cl.next();
        REQUIRE(cl.hasPrev() == true, "page 1 has prev");
        REQUIRE(cl.hasNext() == true, "page 1 hasNext");
        REQUIRE(static_cast<int>(cl.candidateTexts().size()) == wc, "page 1 width");
        REQUIRE(cl.candidateTexts()[0] == "c" + std::to_string(wc), "page 1 first item");

        cl.next();
        REQUIRE(static_cast<int>(cl.candidateTexts().size()) == 3, "last page three items");
        REQUIRE(cl.candidateTexts()[2] == "c" + std::to_string(total - 1), "last item");

        cl.prev();
        cl.prev();
        REQUIRE(static_cast<int>(cl.candidateTexts().size()) == wc, "back to page 0 width");
        REQUIRE(cl.candidateTexts()[0] == "c0", "back to first code");
        REQUIRE(cl.hasPrev() == false, "page 0 again has no prev");
    }

    // 词条 + attrs：正文列与提示列分离（默认 codeRemind 时提示含 attrs）
    {
        std::vector<std::string> texts;
        std::vector<std::string> attrs;
        pushRow(texts, attrs, "hello", "ab");
        pushRow(texts, attrs, "snow", "");
        pushRow(texts, attrs, "gate", "xy");
        freewb::CandidateList cl = makeList(&chttrans);
        cl.setCandidates(std::move(texts), std::move(attrs));
        REQUIRE(cl.selectCandidateText(0) == "hello", "commit text row");
        REQUIRE(cl.candidateTexts()[0] == "hello", "main column commit");
        REQUIRE(cl.candidateTexts()[1] == "snow", "main column");
        if (settings::instance().get_codeRemind())
        {
            REQUIRE(cl.candidatePrompts()[0] == "ab", "prompt column attrs");
            REQUIRE(cl.candidatePrompts()[1].empty(), "empty attr row");
            REQUIRE(cl.candidatePrompts()[2] == "xy", "prompt xy");
        }
    }

    // attr 为空
    {
        std::vector<std::string> texts;
        std::vector<std::string> attrs;
        pushRow(texts, attrs, "hello", "");
        freewb::CandidateList cl = makeList(&chttrans);
        cl.setCandidates(std::move(texts), std::move(attrs));
        REQUIRE(cl.candidateTexts()[0] == "hello", "main column");
        if (settings::instance().get_codeRemind())
        {
            REQUIRE(cl.candidatePrompts()[0].empty(), "no attr in prompt");
        }
    }

    // 结构化词条 + attrs
    {
        freewb::CandidateList cl = makeList(&chttrans);
        std::vector<std::string> texts;
        std::vector<std::string> attrs;
        pushRow(texts, attrs, "a", "1");
        pushRow(texts, attrs, "b", "");
        cl.setCandidates(std::move(texts), std::move(attrs));
        REQUIRE(cl.selectCandidateText(0) == "a", "commit");
        REQUIRE(cl.candidateTexts()[0] == "a", "main column");
        if (settings::instance().get_codeRemind())
        {
            REQUIRE(cl.candidatePrompts()[0] == "1", "prompt column");
        }
    }

    // setCandidates 将页码拉回第一页
    {
        freewb::CandidateList cl = makeList(&chttrans);
        std::vector<std::string> a;
        for (int i = 0; i < 2 * wc + 3; ++i)
        {
            a.push_back("x");
        }
        setFromCommits(cl, std::move(a));
        cl.next();
        cl.next();
        cl.setCandidates({"only"}, {""});
        REQUIRE(cl.size() == 1, "reset page via setCandidates");
        REQUIRE(cl.candidateTexts()[0] == "only", "new content");
    }

    // 普通词条 + OpenCC：应出现简繁提示括号
    if (openccOk)
    {
        freewb::CandidateList cl = makeList(&chttrans);
        setFromCommits(cl, {u8"\u6c49"});
        REQUIRE(promptHasTradHint(cl.candidatePrompts()[0]), "simp/trad hint for normal hanzi");
        REQUIRE(cl.candidatePrompts()[0].find(u8"\u6c49") != std::string::npos, "hint shows simplified form");
    }

    // clear
    {
        freewb::CandidateList cl = makeList(&chttrans);
        setFromCommits(cl, {"a", "b"});
        cl.clear();
        REQUIRE(cl.size() == 0, "clear");
    }

    std::cout << "candidate_list_test: all passed (candiWordCount=" << wc
              << ", opencc=" << (openccOk ? "yes" : "no") << ")\n";
    return 0;
}
