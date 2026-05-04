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
    void clear();
    bool loadFromStream(std::ifstream &in, const char *linePrefix = nullptr);
    void appendCandidatesForPrefix(const std::string &prefix, std::vector<std::string> &out) const;
    const std::string &strInputCode() const;
    int codeLength() const;

private:
    static void collectCandidatesForPrefix(const std::string &prefix,
                                           const std::unordered_map<std::string, std::vector<std::string>> &dict,
                                           std::vector<std::string> &out);
    void readNulTerminatedField(std::ifstream &in, std::string &out);
    bool readU32(std::ifstream &in, uint32_t &out);
    bool readExact(std::ifstream &in, void *dst, std::streamsize len);

    size_t utf8CharCount(const std::string &s);

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
