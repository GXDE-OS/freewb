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

void PyEngine::fillCandidatePayloadPrompts(const std::string &preedit, CandidatePayload &payload) const
{
    const std::size_t n = payload.texts.size();
    const std::size_t rawLen = preedit.size();
    payload.prompts.assign(n, std::string{});

    for (std::size_t i = 0; i < n; ++i)
    {
        const std::string &commitText = payload.texts[i];
        const std::string &rowCode =
            (i < payload.fullCodes.size() && !payload.fullCodes[i].empty()) ? payload.fullCodes[i] : preedit;
        std::string &out = payload.prompts[i];

        const std::size_t nch = MbDictionaryTable::utf8CharCount(commitText);
        if (nch == 1U && preedit == rowCode && wubiPrimaryCodeLookupCallback_)
        {
            const std::string w = wubiPrimaryCodeLookupCallback_(commitText);
            if (!w.empty())
            {
                out.push_back('[');
                out.append(w);
                out.push_back(']');
            }
        }

        if (rowCode.size() > rawLen)
        {
            out.append(rowCode, rawLen, std::string::npos);
        }
    }
}

bool PyEngine::isPreeditOverflow(const char *key, const std::string &pre, const std::string &full) const
{
    if (key == nullptr || pre.empty())
    {
        return false;
    }
    if (hasLongerCodeContinuation(full))
    {
        return false;
    }
    return true;
}

void PyEngine::setWubiPrimaryCodeLookupCallback(WubiPrimaryCodeLookupCallback callback)
{
    wubiPrimaryCodeLookupCallback_ = std::move(callback);
}

void PyEngine::putKey(const char *strCode)
{
    result_.clearRows();

    if (strCode == nullptr)
    {
        return;
    }

    const std::string prefix(strCode);
    mbTable_.appendCandidatesForPrefix(prefix, result_);
    fillCandidatePayloadPrompts(prefix, result_);
}

const CandidatePayload &PyEngine::getResult() const
{
    return result_;
}

void PyEngine::reset()
{
    inputCodes_.clear();
    result_.clearRows();
}

int PyEngine::inputCodeLength() const
{
    return 128;
}

bool PyEngine::shouldProcessKey(const char *key) const
{
    return mbTable_.strInputCode().find(key) != std::string::npos;
}

bool PyEngine::isExactDictionaryKey(const std::string &preedit) const
{
    return mbTable_.hasExactCode(preedit);
}

bool PyEngine::hasLongerCodeContinuation(const std::string &raw) const
{
    return !raw.empty() && mbTable_.hasCandidateForPrefix(raw);
}

void PyEngine::clearMbLoadState()
{
    mbTable_.clear();
    result_.clearRows();
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
        result_.clearRows();
    }
}

} // namespace freewb
