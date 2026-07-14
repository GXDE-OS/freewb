#ifndef ENGINEMANAGER_H
#define ENGINEMANAGER_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "candidatelist.h"
#include "committer.h"
#include "en.h"
#include "engine.h"
#include "keysym.h"
#include "py.h"
#include "types.h"
#include "wbpy.h"
#include "wbzx.h"

namespace freewb
{

class EngineManager
{
public:
    explicit EngineManager(CandidateList *candidateList = nullptr, Committer *committer = nullptr);
    ~EngineManager();

    void nextEngine();
    const char *currentEngineName() const;
    void changeEngine(const std::string &engineName);

    // 英文引擎不在常用引擎列表中，通过此接口切换
    void toggleEnglishEngine();

    bool processKey(FreewbKeySym keysym, FreewbKeyState state);
    void refreshEngineResult();
    void reset();
    void reloadDictionaries(int mask);

    void toggleCharset();
    /** 当前字符集：0=GB, 1=GBK；无五笔引擎时默认 0。 */
    int charSet() const;

    bool isCurrentPreeditExactDictionaryKey(const std::string &preedit) const;
    /** 按五笔码表造词规则计算词组编码；失败返回空串。 */
    std::string calculateWubiPhraseCode(const std::string &phrase) const;

    bool addUserWord(const std::string &code, const std::string &text);
    bool deleteUserWord(const std::string &code, const std::string &text);

    /** 上屏后处理自动词组；选中会话词组上屏时写入 autophrase.mb。 */
    void addAutoPhrase(const std::string &committedText, const std::string &code);

private:
    void initAllEngines();
    void loadDefaultEngines();
    IFreewbEngine *findEngineByName(const char *name) const;

    void restoreLastEngine();

    void commitPreeditOverflow(const std::string &prefix);
    void tryExactDictionarySingleCandidateCommit();

private:
    std::unique_ptr<WbzxEngine> wbzxEngine_ = nullptr;
    std::unique_ptr<Wbpy> wbpyEngine_ = nullptr;
    std::unique_ptr<PyEngine> pyEngine_ = nullptr;
    std::unique_ptr<En> enEngine_ = nullptr;

    IFreewbEngine *currentEngine_ = nullptr;
    IFreewbEngine *lastEngine_ = nullptr;
    std::vector<std::pair<const char *, std::unique_ptr<IFreewbEngine>>> engines_;

    CandidateList *candidateList_ = nullptr;
    Committer *committer_ = nullptr;
};

} // namespace freewb

#endif
