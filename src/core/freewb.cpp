#include "freewb.h"

namespace freewb
{
Freewb::Freewb(void *sd_event_handle) : log_("/tmp/freewb-engine.log")
{
    sdbusProxy_ = new ipc::SDBusProxy(sd_event_handle);
    engineManager_ = new EngineManager();
    candidateList_ = new CandidateList();
}

Freewb::~Freewb()
{
    if (sdbusProxy_ != nullptr)
    {
        delete sdbusProxy_;
        sdbusProxy_ = nullptr;
    }
    if (engineManager_ != nullptr)
    {
        delete engineManager_;
        engineManager_ = nullptr;
    }
    if (candidateList_ != nullptr)
    {
        delete candidateList_;
        candidateList_ = nullptr;
    }
}

void Freewb::activate()
{
    sdbusProxy_->emitShowToolbar();
}

void Freewb::deactivate()
{
    engineManager_->reset();
    candidateList_->clear();
    sdbusProxy_->emitHideToolbar();
    sdbusProxy_->emitUpdatePreeditText({.text = "", .caret = 0, .show = false});
    sdbusProxy_->emitUpdateCandidate({.labels = {}, .texts = {}, .attrs = {}, .hasPrev = false, .hasNext = false, .cursor = -1, .layout = Horizontal});
}

ipc::SDBusProxy *Freewb::sdbusProxy() const
{
    return sdbusProxy_;
}

void Freewb::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    engineManager_->processKey(keysym, state);
    std::pair<PreeditPayload, CandidatePayload> result = engineManager_->getResult();
    candidateList_->setCandidateTexts(result.second.texts);
    sdbusProxy_->emitUpdatePreeditText(result.first);
    sdbusProxy_->emitUpdateCandidate({.labels = {}, .texts = candidateList_->candidateTexts(), .attrs = {}, .hasPrev = candidateList_->hasPrev(), .hasNext = candidateList_->hasNext(), .cursor = candidateList_->cursor(), .layout = Horizontal});
}

void Freewb::reset()
{
    engineManager_->reset();
    candidateList_->clear();
}

} // namespace freewb