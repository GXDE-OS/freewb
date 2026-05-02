#ifndef PY_H
#define PY_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine.h"
#include "ifreewb.h"

namespace freewb
{

class PyEngine : public IFreewbEngine, public IFreewb
{
public:
    PyEngine();
    ~PyEngine();

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    void putKey(const char *strCode) override;
    const CandidatePayload &getResult() const override;
    void reset() override;

    int inputCodeLength() const override;
    bool shouldProcessKey(const char *key) const override;

private:
    void clearMbLoadState();
    void loadDictionary();
    static void collectCandidatesForPrefix(const std::string &prefix, const std::unordered_map<std::string, std::vector<std::string>> &dict, std::vector<std::string> &out);

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
    std::string inputCodes_;
    bool available_ = true;
    CandidatePayload result_;

    static constexpr uint32_t kMaxHzFieldBytes = 7U * 30U;
};

} // namespace freewb

#endif
