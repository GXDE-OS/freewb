#ifndef FREEWB_H
#define FREEWB_H

#include "candidatelist.h"
#include "committer.h"
#include "enginemanager.h"
#include "idbus.h"
#include "keysym.h"
#include "log.h"
#include "punc.h"
#include "types.h"

namespace freewb
{
class Committer;
class Chttrans;
class Special;
class StateManager;

class Freewb
{
public:
    Freewb(ipc::IDBus *dbusProxy, CommitCallback commitCallback);
    ~Freewb();
    void activate();
    void deactivate();
    bool processKey(FreewbKeySym keysym, FreewbKeyState state);
    void updateCandidateAndPreeditToUI();
    void reset();
    void reloadConfig();

    ipc::IDBus *dbusProxy() const;
    EngineManager *engineManager() const;
    CandidateList *candidateList() const;
    Committer *committer() const;
    Chttrans *chttrans() const;
    Punc *punc() const;
    Special *special() const;

private:
    bool handleGlobalShortcutKey(FreewbKeySym keysym, FreewbKeyState state);
    bool handleSingleShortcutKey(FreewbKeySym keysym, FreewbKeyState state);
    void connectDBusCallback();

private:
    FreewbLog log_;
    ipc::IDBus *dbusProxy_ = nullptr;
    EngineManager *engineManager_ = nullptr;
    Chttrans *chttrans_ = nullptr;
    CandidateList *candidateList_ = nullptr;
    Committer *committer_ = nullptr;
    Punc *punc_ = nullptr;
    Special *special_ = nullptr;
    StateManager *stateManager_ = nullptr;
};
} // namespace freewb

#endif