#ifndef ENGINEMANAGER_H
#define ENGINEMANAGER_H

#include <memory>
#include <utility>
#include <vector>

#include "engine.h"
#include "keysym.h"
#include "py.h"
#include "types.h"
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
    const std::string &currentEngineName() const;
    void changeEngine(const std::string &engineName);

    bool processKey(FreewbKeySym keysym, FreewbKeyState state);
    void reset();

private:
    void initEngines();

private:
    std::unique_ptr<WbzxEngine> wbzxEngine_;
    std::unique_ptr<PyEngine> pyEngine_;
    IFreewbEngine *currentEngine_ = nullptr;
    std::vector<std::pair<const char *, std::unique_ptr<IFreewbEngine>>> engines_;

    CandidateList *candidateList_ = nullptr;
};

} // namespace freewb

#endif