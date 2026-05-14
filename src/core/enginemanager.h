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

    bool processKey(FreewbKeySym keysym, FreewbKeyState state);
    void refreshEngineResult();
    void reset();

    bool isCurrentPreeditExactDictionaryKey(const std::string &preedit) const;

private:
    void initAllEngines();
    void loadDefaultEngines();
    IFreewbEngine *findEngineByName(const char *name) const;

    void commitPreeditOverflow(const std::string &prefix);
    void tryExactDictionarySingleCandidateCommit();

private:
    std::unique_ptr<WbzxEngine> wbzxEngine_ = nullptr;
    std::unique_ptr<Wbpy> wbpyEngine_ = nullptr;
    std::unique_ptr<PyEngine> pyEngine_ = nullptr;
    std::unique_ptr<En> enEngine_ = nullptr;

    IFreewbEngine *currentEngine_ = nullptr;
    std::vector<std::pair<const char *, std::unique_ptr<IFreewbEngine>>> engines_;

    CandidateList *candidateList_ = nullptr;
    Committer *committer_ = nullptr;
};

} // namespace freewb

#endif