#include "wbzx.h"

#include <fstream>
#include <vector>

#include "log.h"
#include "utils.h"

namespace freewb
{

WbzxEngine::WbzxEngine()
{
    clearMbLoadState();
    loadDictionary();
}

WbzxEngine::~WbzxEngine() = default;

const char *WbzxEngine::name() const
{
    return "engine:wbzx";
}

bool WbzxEngine::available() const
{
    return true;
}

void WbzxEngine::changeAvailable()
{
    available_ = !available_;
}

void WbzxEngine::putKey(const char *strCode)
{
    result_.labels.clear();
    result_.attrs.clear();
    result_.texts.clear();

    if (strCode == nullptr)
    {
        return;
    }

    const std::string prefix(strCode);

    if (userDict_.contains(prefix))
    {
        const std::vector<std::string> &userTexts = userDict_.lookup(prefix);
        result_.texts.insert(result_.texts.end(), userTexts.begin(), userTexts.end());
    }

    std::vector<std::string> engineTexts;
    mbTable_.appendCandidatesForPrefix(prefix, engineTexts);
    result_.texts.insert(result_.texts.end(), engineTexts.begin(), engineTexts.end());
}

const CandidatePayload &WbzxEngine::getResult() const
{
    return result_;
}

void WbzxEngine::clearMbLoadState()
{
    mbTable_.clear();
    result_ = CandidatePayload{};
}

void WbzxEngine::loadDictionary()
{
    const std::string path = userFreewbPath() + "/data/mb/default/freeime.mb";
    if (path.empty())
    {
        return;
    }

    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        return;
    }

    if (!mbTable_.loadFromStream(in, nullptr))
    {
        mbTable_.clear();
        result_ = CandidatePayload{};
    }
}

void WbzxEngine::reset()
{
    inputCodes_.clear();
    result_ = CandidatePayload{};
}

int WbzxEngine::inputCodeLength() const
{
    return mbTable_.codeLength();
}

bool WbzxEngine::shouldProcessKey(const char *key) const
{
    return mbTable_.strInputCode().find(key) != std::string::npos;
}

} // namespace freewb
