#ifndef FREEWB_H
#define FREEWB_H

#include "candidatelist.h"
#include "committer.h"
#include "enginemanager.h"
#include "keysym.h"
#include "log.h"
#include "punc.h"
#include "sdbus_proxy.h"
#include "types.h"

namespace freewb
{
class Committer;

class Freewb
{
public:
    Freewb(void *sd_event_handle, CommitCallback commitCallback);
    ~Freewb();
    void activate();
    void deactivate();
    bool processKey(FreewbKeySym keysym, FreewbKeyState state);
    void updateCandidateAndPreeditToUI();
    void reset();

    ipc::SDBusProxy *sdbusProxy() const;
    EngineManager *engineManager() const;
    CandidateList *candidateList() const;
    Punc *punc() const;

private:
    bool handleGlobalShortcutKey(FreewbKeySym keysym, FreewbKeyState state);
    bool handleSingleShortcutKey(FreewbKeySym keysym, FreewbKeyState state);

private:
    FreewbLog log_;
    ipc::SDBusProxy *sdbusProxy_ = nullptr;
    EngineManager *engineManager_ = nullptr;
    CandidateList *candidateList_ = nullptr;
    Committer *committer_ = nullptr;
    Punc *punc_ = nullptr;
};
} // namespace freewb

#endif