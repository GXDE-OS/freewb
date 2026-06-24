#ifndef WBZX_H
#define WBZX_H

#include <string>
#include <unordered_map>
#include <vector>

#include "autophrase.h"
#include "common.h"
#include "engine.h"
#include "gb2312filter.h"
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
    /** 按码表造词规则为 UTF-8 词组计算五笔编码；失败或非五笔引擎返回空串。 */
    std::string calculateWubiPhraseCode(const std::string &phrase) const;

    void toggleCharset();

    void reloadMainDictionary();
    void reloadUserDictionary();
    void reloadAutoPhraseDictionary();

    bool addUserWord(const std::string &code, const std::string &text);
    bool deleteUserWord(const std::string &code, const std::string &text);

    /** 上屏后处理自动词组。 */
    void addAutoPhrase(const std::string &committedText, const std::string &code);

    /** 主五笔码表；供自动造词读取元数据与已有词条。 */
    const MbDictionaryTable &mainDictionaryTable() const;

private:
    void clearMbLoadState();
    void loadDictionary();
    /** 从 mbTable_ 单字码表构建单字→首选码（仅首字节为字母的编码参与）。 */
    void initSingleHanziPrimaryCodeFromMbTable();
    void fillCandidatePayloadPrompts(const std::string &preedit, CandidatePayload &payload) const;
    /** 按 [DeletedWord] 与 charset 过滤 @p payload 中自 @p userCandidateCount 起的主码表候选段。 */
    void filterMainDictCandidates(CandidatePayload &payload, std::size_t userCandidateCount) const;
    /**
     * 主码表在 @p prefix 下是否存在对用户可见的候选（charset 一致，且不在 [DeletedWord]）。
     * 用于 isPreeditOverflow —— 判断再输入一字后 preedit 是否还能在主码表续码。
     */
    bool hasVisibleMainDictCandidate(const std::string &prefix) const;

    MbDictionaryTable mbTable_;
    UserDict userDict_;
    Gb2312Filter gb2312Filter_;
    std::unordered_map<std::string, std::string> singleHanziPrimaryCode_;
    std::string inputCodes_;
    bool available_ = true;
    CandidatePayload result_;
    AutoPhrase autoPhrase_;
};
} // namespace freewb

#endif
