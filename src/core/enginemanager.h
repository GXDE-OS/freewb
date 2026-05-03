#ifndef ENGINEMANAGER_H
#define ENGINEMANAGER_H

#include <memory>
#include <utility>
#include <vector>

#include "engine.h"
#include "en.h"
#include "keysym.h"
#include "py.h"
#include "types.h"
#include "wbpy.h"
#include "wbzx.h"
#include "candidatelist.h"

namespace freewb
{

class EngineManager
{
public:
    explicit EngineManager(CandidateList *candidateList = nullptr);
    ~EngineManager();

    void nextEngine();
    const char *currentEngineName() const;
    void changeEngine(const std::string &engineName);

    bool processKey(FreewbKeySym keysym, FreewbKeyState state);
    void refreshEngineResult();
    void reset();

private:
    void initAllEngines();
    void loadDefaultEngines();
    IFreewbEngine *findEngineByName(const char *name) const;

private:
    std::unique_ptr<WbzxEngine> wbzxEngine_ = nullptr;
    std::unique_ptr<Wbpy> wbpyEngine_ = nullptr;
    std::unique_ptr<PyEngine> pyEngine_ = nullptr;
    std::unique_ptr<En> enEngine_ = nullptr;

    IFreewbEngine *currentEngine_ = nullptr;
    std::vector<std::pair<const char *, std::unique_ptr<IFreewbEngine>>> engines_;

    CandidateList *candidateList_ = nullptr;
};

} // namespace freewb

#endif