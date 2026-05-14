#include "wbzx.h"

#include <cctype>
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

void WbzxEngine::fillCandidatePayloadPrompts(const std::string &preedit, CandidatePayload &payload) const
{
    const std::size_t n = payload.texts.size();
    const std::size_t rawLen = preedit.size();
    payload.prompts.assign(n, std::string{});

    for (std::size_t i = 0; i < n; ++i)
    {
        const std::string &fullCode =
            (i < payload.fullCodes.size() && !payload.fullCodes[i].empty()) ? payload.fullCodes[i] : preedit;
        if (fullCode.size() > rawLen)
        {
            payload.prompts[i].assign(fullCode, rawLen, std::string::npos);
        }
    }
}

bool WbzxEngine::isPreeditOverflow(const std::string &full) const
{
    if (full.empty())
    {
        return false;
    }

    return !mbTable_.hasCandidateForPrefix(full) && !userDict_.hasEntryStartingWithPrefix(full);
}

void WbzxEngine::putKey(const char *strCode)
{
    result_.clearRows();

    if (strCode == nullptr)
    {
        return;
    }

    const std::string prefix(strCode);
    if (userDict_.contains(prefix))
    {
        const std::vector<std::string> &userTexts = userDict_.lookup(prefix);
        result_.texts.insert(result_.texts.end(), userTexts.begin(), userTexts.end());
        result_.fullCodes.insert(result_.fullCodes.end(), userTexts.size(), std::string{});
        fillCandidatePayloadPrompts(prefix, result_);
        return;
    }
    mbTable_.appendCandidatesForPrefix(prefix, result_);
    fillCandidatePayloadPrompts(prefix, result_);
}

const CandidatePayload &WbzxEngine::getResult() const
{
    return result_;
}

void WbzxEngine::clearMbLoadState()
{
    mbTable_.clear();
    singleHanziPrimaryCode_.clear();
    result_.clearRows();
}

void WbzxEngine::initSingleHanziPrimaryCodeFromMbTable()
{
    /* 仅首字节是字母的编码参与；剔除 /xxx 等特殊键。
     * 对已入选者仅当新编码更长时替换；等长时保留先记录的。 */
    singleHanziPrimaryCode_.clear();
    for (const auto &kv : mbTable_.singleCharLexicon())
    {
        const std::string &code = kv.first;
        if (code.empty() || !std::isalpha(static_cast<unsigned char>(code[0])))
        {
            continue;
        }
        for (const std::string &hz : kv.second)
        {
            if (MbDictionaryTable::utf8CharCount(hz) != 1U)
            {
                continue;
            }
            const auto it = singleHanziPrimaryCode_.find(hz);
            if (it == singleHanziPrimaryCode_.end())
            {
                singleHanziPrimaryCode_.emplace(hz, code);
            }
            else if (code.size() > it->second.size())
            {
                it->second = code;
            }
        }
    }
}

std::string WbzxEngine::primaryWubiCodeForSingleHanziUtf8(const std::string &hz) const
{
    if (hz.empty())
    {
        return {};
    }
    const auto it = singleHanziPrimaryCode_.find(hz);
    if (it == singleHanziPrimaryCode_.end())
    {
        return {};
    }
    return it->second;
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
        singleHanziPrimaryCode_.clear();
        result_.clearRows();
        return;
    }
    initSingleHanziPrimaryCodeFromMbTable();
}

void WbzxEngine::reset()
{
    inputCodes_.clear();
    result_.clearRows();
}

bool WbzxEngine::shouldProcessKey(const char *key) const
{
    return mbTable_.strInputCode().find(key) != std::string::npos;
}

bool WbzxEngine::isExactDictionaryKey(const std::string &preedit) const
{
    return mbTable_.hasExactCode(preedit) || userDict_.contains(preedit);
}

} // namespace freewb
