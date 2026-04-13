#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>
#include <vector>

#include "types.h"

namespace freewb
{
struct EngineRuleCell
{
    uint8_t iFlag = 0;
    uint8_t iWhich = 0;
    uint8_t iIndex = 0;
};

struct EngineRuleBlock
{
    uint8_t iFlag = 0;
    uint8_t iWords = 0;
    std::vector<EngineRuleCell> cells;
};

class IFreewbEngine
{
public:
    virtual ~IFreewbEngine() = default;
    virtual void putKey(const char *key) = 0;
    virtual const CandidatePayload &getResult() const = 0;
    virtual void reset() = 0;
    virtual int inputCodeLength() const = 0;
};
} // namespace freewb

#endif