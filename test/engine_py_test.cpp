#include <iostream>

#include "log.h"
#include "py.h"

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

    for (std::size_t i = 0; i < results.texts.size(); ++i)
    {
        const std::string trail = (i < results.prompts.size()) ? results.prompts[i] : std::string{};
        std::cout << results.texts[i] + trail << std::endl;
    }
    std::cout << "freewb-test-engine-py: ok\n";
    return 0;
}
