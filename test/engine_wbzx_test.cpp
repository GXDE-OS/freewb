/**
 * 词库加载测试：将 HOME 指向临时目录，写入与 freedict LoadDict 格式一致的最小 .mb，
 * 构造 WbzxEngine 并校验解析结果。
 *
 * 刻意使用 C++11 + POSIX，不依赖 std::filesystem（C++17）。
 */

#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "log.h"
#include "wbzx.h"

namespace
{

void testWbzxEngine()
{
    FreewbLog log("/tmp/freewb-test-engine-wbzx.log");
    freewb::WbzxEngine engine;

    engine.putKey("g");
    const auto &results1 = engine.getResult();
    std::cout << "results1 : " << results1.texts[0] << std::endl;
    std::cout << "results1 : " << results1.texts[1] << std::endl;
    std::cout << "results1 : " << results1.texts[2] << std::endl;
    std::cout << "results1 : " << results1.texts[3] << std::endl;

    engine.putKey("e");
    engine.putKey("x");
    const auto &results3 = engine.getResult();
    for (const auto &text : results3.texts)
    {
        std::cout << text << std::endl;
    }
    engine.putKey("k");
    const auto &results4 = engine.getResult();
    for (const auto &text : results4.texts)
    {
        std::cout << text << std::endl;
    }
}

} // namespace

int main()
{
    testWbzxEngine();

    return 0;
}
