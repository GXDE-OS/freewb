#include "py.h"

#include <fstream>
#include <vector>

#include "log.h"
#include "utils.h"

namespace freewb
{

PyEngine::PyEngine()
{
    clearMbLoadState();
    loadDictionary();
}

PyEngine::~PyEngine() = default;

const char *PyEngine::name() const
{
    return "engine:py";
}

bool PyEngine::available() const
{
    return available_;
}

void PyEngine::changeAvailable()
{
    available_ = !available_;
}

void PyEngine::putKey(const char *strCode)
{
    const std::string &prefix = strCode;

    std::vector<std::string> texts;
    mbTable_.appendCandidatesForPrefix(prefix, texts);

    result_.labels.clear();
    result_.attrs.clear();
    result_.texts = std::move(texts);
}

const CandidatePayload &PyEngine::getResult() const
{
    return result_;
}

void PyEngine::reset()
{
    inputCodes_.clear();
    result_ = CandidatePayload{};
}

int PyEngine::inputCodeLength() const
{
    return mbTable_.codeLength();
}

bool PyEngine::shouldProcessKey(const char *key) const
{
    return mbTable_.strInputCode().find(key) != std::string::npos;
}

void PyEngine::clearMbLoadState()
{
    mbTable_.clear();
    result_ = CandidatePayload{};
}

void PyEngine::loadDictionary()
{
    const std::string path = userFreewbPath() + "/data/mb/default/attach.mb";
    if (path.empty())
    {
        return;
    }

    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        return;
    }

    if (!mbTable_.loadFromStream(in, "py attach "))
    {
        mbTable_.clear();
        result_ = CandidatePayload{};
    }
}

} // namespace freewb
