#ifndef FREEWB_H
#define FREEWB_H

#include "candidatelist.h"
#include "charwidth.h"
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
    bool processKeyPress(FreewbKeySym keysym, FreewbKeyState state);
    bool processKeyRelease(FreewbKeySym keysym, FreewbKeyState state);
    void updateCandidateAndPreeditToUI();
    void reset();
    void reloadConfig();

    ipc::IDBus *dbusProxy() const;
    EngineManager *engineManager() const;
    CandidateList *candidateList() const;
    Committer *committer() const;
    Chttrans *chttrans() const;
    Punc *punc() const;
    CharWidth *charWidth() const;
    Special *special() const;

private:
    // 中英切换键需“单独按下再抬起”才生效，按键按下阶段在此记录是否仍满足条件
    void updateCnEnSwitchPending(FreewbKeySym keysym, FreewbKeyState state);
    bool handleSingleKey(FreewbKeySym keysym, FreewbKeyState state);
    bool handleComboKey(FreewbKeySym keysym, FreewbKeyState state);
    void playTypingSound(FreewbKeySym keysym) const;
    void connectDBusCallback();

private:
    FreewbLog log_;
    ipc::IDBus *dbusProxy_ = nullptr;
    EngineManager *engineManager_ = nullptr;
    Chttrans *chttrans_ = nullptr;
    CandidateList *candidateList_ = nullptr;
    Committer *committer_ = nullptr;
    Punc *punc_ = nullptr;
    CharWidth *charWidth_ = nullptr;
    Special *special_ = nullptr;
    StateManager *stateManager_ = nullptr;
    bool cnEnSwitchKeyPending_ = false;
};
} // namespace freewb

#endif