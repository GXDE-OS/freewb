#ifndef COMMON_H
#define COMMON_H

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine.h"

namespace freewb
{

class MbDictionaryTable
{
public:
    /** UTF-8 字节串中的字符数（按首字节判定宽度；非法序列按单字节步进）。 */
    static std::size_t utf8CharCount(const std::string &s);

    void clear();
    bool loadFromStream(std::ifstream &in, const char *linePrefix = nullptr);
    /** 向 @p out 追加候选：与 texts 同步写入 fullCodes（完整编码键）。 */
    void appendCandidatesForPrefix(const std::string &prefix, CandidatePayload &out) const;
    const std::string &strInputCode() const;
    bool hasExactCode(const std::string &code) const;
    /** 是否与 appendCandidatesForPrefix 至少产出一条一致（非空 hz）；供引擎续码判断 */
    bool hasCandidateForPrefix(const std::string &prefix) const;
    /** 单字码表（code → 若干 hz）；供五笔引擎构建单字→首选码索引等。 */
    const std::unordered_map<std::string, std::vector<std::string>> &singleCharLexicon() const
    {
        return singleChardict_;
    }

private:
    static void collectCandidateItemsForPrefix(const std::string &prefix,
                                               const std::unordered_map<std::string, std::vector<std::string>> &dict,
                                               CandidatePayload &out);
    void readNulTerminatedField(std::ifstream &in, std::string &out);
    bool readU32(std::ifstream &in, uint32_t &out);
    bool readExact(std::ifstream &in, void *dst, std::streamsize len);

private:
    std::unordered_map<std::string, std::vector<std::string>> singleChardict_;
    std::unordered_map<std::string, std::vector<std::string>> multiChardict_;

    std::string tableName_;
    std::string tableInfo_;
    std::string tableCreateTime_;
    std::string strEndKeys_;
    std::string strSpecialKeys_;
    std::string strCodeType_;
    std::string strStraightUPKeys_;
    std::string strInputCode_;
    uint8_t cWildChar_ = 0;
    uint8_t bRule_ = 0;
    uint32_t iCodeLength_ = 4;
    std::vector<EngineRuleBlock> rules_;
    uint32_t recordCount_ = 0;

    static const std::size_t maxCandidatesPages_ = 500;
};

} // namespace freewb

#endif // COMMON_H
