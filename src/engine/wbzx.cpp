#include "wbzx.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <vector>

#include "log.h"
#include "utils.h"

namespace freewb
{
void WbzxEngine::collectCandidatesForPrefix(const std::string &prefix, const std::unordered_map<std::string, std::vector<std::string>> &dict, std::vector<std::string> &out)
{
    std::vector<std::string> keys;
    keys.reserve(dict.size());
    for (const auto &kv : dict)
    {
        const std::string &key = kv.first;
        if (key.size() < prefix.size())
        {
            continue;
        }
        if (key.compare(0, prefix.size(), prefix) == 0)
        {
            keys.push_back(key);
        }
    }
    std::sort(keys.begin(), keys.end());
    for (const std::string &k : keys)
    {
        const auto it = dict.find(k);
        if (it == dict.end())
        {
            continue;
        }
        for (const auto &hz : it->second)
        {
            if (!hz.empty())
            {
                out.push_back(hz);
            }
        }
    }
}

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

    // 1. 用户词库：仅做精确编码匹配，命中后排在最前。
    if (userDict_.contains(prefix))
    {
        const std::vector<std::string> &userTexts = userDict_.lookup(prefix);
        result_.texts.insert(result_.texts.end(), userTexts.begin(), userTexts.end());
    }

    // 2. 五笔字型字库：按前缀收集。
    std::vector<std::string> engineTexts;
    collectCandidatesForPrefix(prefix, singleChardict_, engineTexts);
    collectCandidatesForPrefix(prefix, multiChardict_, engineTexts);
    result_.texts.insert(result_.texts.end(), engineTexts.begin(), engineTexts.end());
}

const CandidatePayload &WbzxEngine::getResult() const
{
    return result_;
}

void WbzxEngine::clearMbLoadState()
{
    result_ = CandidatePayload{};
    singleChardict_.clear();
    multiChardict_.clear();
    tableName_.clear();
    tableInfo_.clear();
    tableCreateTime_.clear();
    strEndKeys_.clear();
    strSpecialKeys_.clear();
    strCodeType_.clear();
    strStraightUPKeys_.clear();
    strInputCode_.clear();
    cWildChar_ = 0;
    bRule_ = 0;
    iCodeLength_ = 4;
    rules_.clear();
    recordCount_ = 0;
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

    readNulTerminatedField(in, tableName_);
    readNulTerminatedField(in, tableInfo_);
    readNulTerminatedField(in, tableCreateTime_);
    readNulTerminatedField(in, strEndKeys_);
    readNulTerminatedField(in, strSpecialKeys_);
    readNulTerminatedField(in, strCodeType_);
    readNulTerminatedField(in, strStraightUPKeys_);

    uint32_t inputCodeLen = 0;
    if (!readU32(in, inputCodeLen))
    {
        clearMbLoadState();
        return;
    }

    {
        std::vector<char> buf(static_cast<size_t>(inputCodeLen) + 1U);
        in.read(buf.data(), static_cast<std::streamsize>(inputCodeLen) + 1);
        if (!in || static_cast<uint32_t>(in.gcount()) != inputCodeLen + 1U)
        {
            clearMbLoadState();
            return;
        }
        strInputCode_.assign(buf.data());
    }
    if (!readExact(in, &cWildChar_, 1))
    {
        clearMbLoadState();
        return;
    }
    iCodeLength_ = 4;
    if (!readExact(in, &bRule_, 1))
    {
        clearMbLoadState();
        return;
    }
    if (bRule_)
    {
        rules_.resize(static_cast<size_t>(iCodeLength_ - 1U));
        for (uint32_t ri = 0; ri < iCodeLength_ - 1U; ++ri)
        {
            if (!readExact(in, &rules_[ri].iFlag, 1) || !readExact(in, &rules_[ri].iWords, 1))
            {
                clearMbLoadState();
                return;
            }
            rules_[ri].cells.resize(static_cast<size_t>(iCodeLength_));
            for (uint32_t k = 0; k < iCodeLength_; ++k)
            {
                EngineRuleCell &cell = rules_[ri].cells[k];
                if (!readExact(in, &cell.iFlag, 1) || !readExact(in, &cell.iWhich, 1) || !readExact(in, &cell.iIndex, 1))
                {
                    clearMbLoadState();
                    return;
                }
            }
        }
    }

    if (!readU32(in, recordCount_))
    {
        clearMbLoadState();
        return;
    }
    for (uint32_t i = 0; i < recordCount_; ++i)
    {
        uint32_t codeFieldLen = 0;
        if (!readU32(in, codeFieldLen))
        {
            clearMbLoadState();
            return;
        }
        std::vector<char> codeBuf(static_cast<size_t>(codeFieldLen));
        in.read(codeBuf.data(), static_cast<std::streamsize>(codeFieldLen));
        if (!in || static_cast<uint32_t>(in.gcount()) != codeFieldLen)
        {
            clearMbLoadState();
            return;
        }
        if (codeFieldLen == 0 || codeBuf[static_cast<size_t>(codeFieldLen) - 1U] != '\0')
        {
            clearMbLoadState();
            return;
        }
        const std::string strCode(codeBuf.data());
        uint32_t hzFieldLen = 0;
        if (!readU32(in, hzFieldLen))
        {
            clearMbLoadState();
            return;
        }
        if (hzFieldLen > kMaxHzFieldBytes)
        {
            clearMbLoadState();
            return;
        }
        std::vector<char> hzBuf(static_cast<size_t>(hzFieldLen));
        in.read(hzBuf.data(), static_cast<std::streamsize>(hzFieldLen));
        if (!in || static_cast<uint32_t>(in.gcount()) != hzFieldLen)
        {
            clearMbLoadState();
            return;
        }
        if (hzFieldLen == 0 || hzBuf[static_cast<size_t>(hzFieldLen) - 1U] != '\0')
        {
            clearMbLoadState();
            return;
        }

        int8_t typ = 0;
        if (!readExact(in, &typ, 1))
        {
            clearMbLoadState();
            return;
        }
        (void)typ;
        std::string hz(hzBuf.data());
        const size_t nChar = utf8CharCount(hz);
        if (nChar == 1U)
        {
            singleChardict_[strCode].push_back(std::move(hz));
        }
        else if (nChar >= 2U)
        {
            multiChardict_[strCode].push_back(std::move(hz));
        }
    }

    FREEWB_DEBUG("tableName={}\ntableInfo={}\ntableCreateTime={}\nstrEndKeys={}\nstrSpecialKeys={}\nstrCodeType={}\nstrStraightUPKeys={}\ninputCodeLen={}\ncWildChar={}\nbRule={}\niCodeLength={}\nrecordCount={}", tableName_, tableInfo_, tableCreateTime_, strEndKeys_, strSpecialKeys_, strCodeType_,
                 strStraightUPKeys_, inputCodeLen, cWildChar_, bRule_, iCodeLength_, recordCount_);
}

void WbzxEngine::reset()
{
    inputCodes_.clear();
    result_ = CandidatePayload{};
}

int WbzxEngine::inputCodeLength() const
{
    return iCodeLength_;
}

} // namespace freewb
