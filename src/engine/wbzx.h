#ifndef WBZX_H
#define WBZX_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common.h"
#include "engine.h"
#include "ifreewb.h"
#include "userdict.h"

namespace freewb
{

class WbzxEngine : public IFreewbEngine, public IFreewb
{
public:
    WbzxEngine();
    ~WbzxEngine();

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    void putKey(const char *strCode) override;
    const CandidatePayload &getResult() const override;
    void reset() override;

    bool shouldProcessKey(const char *key) const override;
    bool isExactDictionaryKey(const std::string &preedit) const override;
    bool isPreeditOverflow(const std::string &full) const override;
    /** 主五笔码表单字 UTF-8 → 首选码；无索引或查无则空串。供拼音 [xxxx] 反查等。 */
    std::string primaryWubiCodeForSingleHanziUtf8(const std::string &hz) const;

private:
    void clearMbLoadState();
    void loadDictionary();
    /** 从 mbTable_ 单字码表构建单字→首选码（仅首字节为字母的编码参与）。 */
    void initSingleHanziPrimaryCodeFromMbTable();
    void fillCandidatePayloadPrompts(const std::string &preedit, CandidatePayload &payload) const;

private:
    MbDictionaryTable mbTable_;
    UserDict userDict_;
    std::unordered_map<std::string, std::string> singleHanziPrimaryCode_;
    std::string inputCodes_;
    bool available_ = true;
    CandidatePayload result_;
};
} // namespace freewb

#endif
