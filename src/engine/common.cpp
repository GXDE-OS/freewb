#include "common.h"

#include <algorithm>
#include <vector>

#include "log.h"
#include "utils.h"

namespace freewb
{
const uint32_t kMaxHzFieldBytes = 7U * 30U;

void MbDictionaryTable::collectCandidatesForPrefix(const std::string &prefix,
                                                   const std::unordered_map<std::string, std::vector<std::string>> &dict,
                                                   std::vector<std::string> &out)
{
    if (out.size() >= maxCandidatesPages_)
    {
        return;
    }
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
        if (out.size() >= maxCandidatesPages_)
        {
            return;
        }
        const auto it = dict.find(k);
        if (it == dict.end())
        {
            continue;
        }
        for (const auto &hz : it->second)
        {
            if (hz.empty())
            {
                continue;
            }
            out.push_back(hz);
            if (out.size() >= maxCandidatesPages_)
            {
                return;
            }
        }
    }
}

void MbDictionaryTable::clear()
{
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

bool MbDictionaryTable::loadFromStream(std::ifstream &in, const char *linePrefix)
{
    const char *p = linePrefix != nullptr ? linePrefix : "";

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
        clear();
        return false;
    }

    {
        std::vector<char> buf(static_cast<size_t>(inputCodeLen) + 1U);
        in.read(buf.data(), static_cast<std::streamsize>(inputCodeLen) + 1);
        if (!in || static_cast<uint32_t>(in.gcount()) != inputCodeLen + 1U)
        {
            clear();
            return false;
        }
        strInputCode_.assign(buf.data());
    }
    if (!readExact(in, &cWildChar_, 1))
    {
        clear();
        return false;
    }
    iCodeLength_ = 4;
    if (!readExact(in, &bRule_, 1))
    {
        clear();
        return false;
    }
    if (bRule_)
    {
        rules_.resize(static_cast<size_t>(iCodeLength_ - 1U));
        for (uint32_t ri = 0; ri < iCodeLength_ - 1U; ++ri)
        {
            if (!readExact(in, &rules_[ri].iFlag, 1) || !readExact(in, &rules_[ri].iWords, 1))
            {
                clear();
                return false;
            }
            rules_[ri].cells.resize(static_cast<size_t>(iCodeLength_));
            for (uint32_t k = 0; k < iCodeLength_; ++k)
            {
                EngineRuleCell &cell = rules_[ri].cells[k];
                if (!readExact(in, &cell.iFlag, 1) || !readExact(in, &cell.iWhich, 1) || !readExact(in, &cell.iIndex, 1))
                {
                    clear();
                    return false;
                }
            }
        }
    }

    if (!readU32(in, recordCount_))
    {
        clear();
        return false;
    }
    for (uint32_t i = 0; i < recordCount_; ++i)
    {
        uint32_t codeFieldLen = 0;
        if (!readU32(in, codeFieldLen))
        {
            clear();
            return false;
        }
        std::vector<char> codeBuf(static_cast<size_t>(codeFieldLen));
        in.read(codeBuf.data(), static_cast<std::streamsize>(codeFieldLen));
        if (!in || static_cast<uint32_t>(in.gcount()) != codeFieldLen)
        {
            clear();
            return false;
        }
        if (codeFieldLen == 0 || codeBuf[static_cast<size_t>(codeFieldLen) - 1U] != '\0')
        {
            clear();
            return false;
        }
        const std::string strCode(codeBuf.data());
        uint32_t hzFieldLen = 0;
        if (!readU32(in, hzFieldLen))
        {
            clear();
            return false;
        }
        if (hzFieldLen > kMaxHzFieldBytes)
        {
            clear();
            return false;
        }
        std::vector<char> hzBuf(static_cast<size_t>(hzFieldLen));
        in.read(hzBuf.data(), static_cast<std::streamsize>(hzFieldLen));
        if (!in || static_cast<uint32_t>(in.gcount()) != hzFieldLen)
        {
            clear();
            return false;
        }
        if (hzFieldLen == 0 || hzBuf[static_cast<size_t>(hzFieldLen) - 1U] != '\0')
        {
            clear();
            return false;
        }

        int8_t typ = 0;
        if (!readExact(in, &typ, 1))
        {
            clear();
            return false;
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

    FREEWB_DEBUG("{}tableName={}\ntableInfo={}\ntableCreateTime={}\nstrEndKeys={}\nstrSpecialKeys={}\nstrCodeType={}"
                 "\nstrStraightUPKeys={}\ninputCodeLen={}\ncWildChar={}\nbRule={}\niCodeLength={}\nrecordCount={}",
                 p, tableName_, tableInfo_, tableCreateTime_, strEndKeys_, strSpecialKeys_, strCodeType_, strStraightUPKeys_,
                 inputCodeLen, cWildChar_, bRule_, iCodeLength_, recordCount_);
    return true;
}

void MbDictionaryTable::appendCandidatesForPrefix(const std::string &prefix, std::vector<std::string> &out) const
{
    collectCandidatesForPrefix(prefix, singleChardict_, out);
    collectCandidatesForPrefix(prefix, multiChardict_, out);
}

const std::string &MbDictionaryTable::strInputCode() const
{
    return strInputCode_;
}

int MbDictionaryTable::codeLength() const
{
    return static_cast<int>(iCodeLength_);
}

void MbDictionaryTable::readNulTerminatedField(std::ifstream &in, std::string &out)
{
    uint32_t n = 0;
    if (!readU32(in, n))
    {
        return;
    }
    std::vector<char> buf(static_cast<size_t>(n) + 1U);
    in.read(buf.data(), static_cast<std::streamsize>(n) + 1);
    if (!in || static_cast<uint32_t>(in.gcount()) != n + 1U)
    {
        return;
    }
    out.assign(buf.data());
}

bool MbDictionaryTable::readU32(std::ifstream &in, uint32_t &out)
{
    unsigned char b[4];
    in.read(reinterpret_cast<char *>(b), 4);
    if (!in || in.gcount() != 4)
    {
        return false;
    }
    out = static_cast<uint32_t>(b[0]) | (static_cast<uint32_t>(b[1]) << 8) | (static_cast<uint32_t>(b[2]) << 16) |
          (static_cast<uint32_t>(b[3]) << 24);
    return true;
}

bool MbDictionaryTable::readExact(std::ifstream &in, void *dst, std::streamsize len)
{
    in.read(static_cast<char *>(dst), len);
    return in && in.gcount() == len;
}

size_t MbDictionaryTable::utf8CharCount(const std::string &s)
{
    size_t n = 0;
    const char *p = s.c_str();
    while (*p)
    {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c < 0x80U)
        {
            ++p;
        }
        else if ((c >> 5) == 6U)
        {
            p += 2;
        }
        else if ((c >> 4) == 14U)
        {
            p += 3;
        }
        else if ((c >> 3) == 30U)
        {
            p += 4;
        }
        else
        {
            ++p;
            continue;
        }
        ++n;
    }
    return n;
}

} // namespace freewb
