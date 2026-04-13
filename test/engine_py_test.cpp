#include "log.h"
#include "py.h"
#include "utils.h"

#include <iostream>

int main()
{
    FreewbLog log("/tmp/freewb-test-engine-py.log");
    freewb::PyEngine engine;
    engine.putKey("nihaoaaoa");
    const auto &results = engine.getResult();
    if (results.texts.empty())
    {
        std::cout << "no candidates\n";
        return 1;
    }

    for (const auto &text : results.texts)
    {
        std::cout << text << std::endl;
    }
    std::cout << "freewb-test-engine-py: ok\n";
    return 0;
}
