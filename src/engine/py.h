#ifndef PY_H
#define PY_H

#include <functional>
#include <string>
#include <vector>

#include "common.h"
#include "engine.h"
#include "ifreewb.h"

namespace freewb
{

/** 单字 UTF-8 → 五笔首选码；空表示不做反查。 */
using WubiPrimaryCodeLookupCallback = std::function<std::string(const std::string &hzUtf8)>;

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

    bool shouldProcessKey(const char *key) const override;
    bool isExactDictionaryKey(const std::string &preedit) const override;
    bool isPreeditOverflow(const char *key, const std::string &pre, const std::string &full) const override;

    /** 词典是否存在以 @p raw 为真前缀的更长编码键；供五笔拼音组合引擎查询。 */
    bool hasLongerCodeContinuation(const std::string &raw) const;

    /** 单字五笔首选码查询 */
    void setWubiPrimaryCodeLookupCallback(WubiPrimaryCodeLookupCallback callback);

private:
    void clearMbLoadState();
    void loadDictionary();
    void fillCandidatePayloadPrompts(const std::string &preedit, CandidatePayload &payload) const;

private:
    MbDictionaryTable mbTable_;
    WubiPrimaryCodeLookupCallback wubiPrimaryCodeLookupCallback_;
    std::string inputCodes_;
    bool available_ = true;
    CandidatePayload result_;
};

} // namespace freewb

#endif
