#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "candidatelist.h"
#include "log.h"
#include "settings.h"
#include "types.h"

namespace
{

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

} // namespace

int main()
{
    FreewbLog log("/tmp/freewb-candidate-list-test.log");

    const int wc = settings::instance().get_candiWordCount();
    REQUIRE(wc > 0, "candiWordCount should be positive");

    // 默认构造 + clear
    {
        freewb::CandidateList cl;
        cl.clear();
        REQUIRE(cl.size() == 0, "size after clear");
        REQUIRE(cl.candidateTexts().empty(), "candidateTexts empty");
    }

    // setCandidateTexts：单条、合法下标 select
    {
        freewb::CandidateList cl;
        cl.setCandidateTexts({"hello"});
        REQUIRE(cl.size() == 1, "single item size");
        REQUIRE(cl.candidateTexts().size() == 1u, "candidateTexts size");
        REQUIRE(cl.selectCandidateText(0) == "hello", "selectCandidateText(0)");
    }

    // CandidatePayload 移动构造：cursor、首屏切片
    {
        freewb::CandidatePayload p;
        p.labels = {"1"};
        p.texts = {u8"\u7532", u8"\u4e59"};
        p.attrs = {""};
        p.hasPrev = true;
        p.hasNext = false;
        p.cursor = 42;
        freewb::CandidateList cl(std::move(p));
        REQUIRE(cl.cursor() == 42, "cursor from payload");
        REQUIRE(static_cast<int>(cl.candidateTexts().size()) == std::min(wc, 2), "payload ctor first page width");
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

        freewb::CandidateList cl(std::move(all), 0);
        REQUIRE(static_cast<int>(cl.candidateTexts().size()) == wc, "page 0 width");
        REQUIRE(cl.hasPrev() == true, "page 0 has prev");
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
    }

    // setCandidateTexts 将页码拉回第一页
    {
        freewb::CandidateList cl;
        std::vector<std::string> a;
        for (int i = 0; i < 2 * wc + 3; ++i)
        {
            a.push_back("x");
        }
        cl.setCandidateTexts(std::move(a));
        cl.next();
        cl.next();
        cl.setCandidateTexts({"only"});
        REQUIRE(cl.size() == 1, "reset page via setCandidateTexts");
        REQUIRE(cl.candidateTexts()[0] == "only", "new content");
    }

    // clear
    {
        freewb::CandidateList cl;
        cl.setCandidateTexts({"a", "b"});
        cl.clear();
        REQUIRE(cl.size() == 0, "clear");
    }

    std::cout << "candidate_list_test: all passed (candiWordCount=" << wc << ")\n";
    return 0;
}
