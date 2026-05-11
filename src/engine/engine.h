#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>
#include <string>
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
    /** 超长顶字拆码时前编最大字节数；0 表示不启用。具体由各引擎（码表 codeLength、拼音长度策略等）实现。 */
    virtual int inputCodeLength() const = 0;
    virtual bool shouldProcessKey(const char *key) const = 0;

    /** @p preedit 是否与词典中某条编码键完全一致（整码）；供上屏/拆码策略查询。 */
    virtual bool isExactDictionaryKey(const std::string &preedit) const = 0;

    /**
     * 本键按下后（pre + key = full）：续码在词库中不可接且 pre 非空时为 preedit 顶字溢出，返回 true；
     * 返回 false 表示将 full 写入 preedit 并刷新候选。
     */
    virtual bool isPreeditOverflow(const char *key, const std::string &pre, const std::string &full) const = 0;
};

} // namespace freewb

#endif
