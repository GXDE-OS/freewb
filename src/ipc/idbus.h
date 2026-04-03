#ifndef IDBUS_H
#define IDBUS_H

#include <functional>

#include "types.h"

namespace freewb::ipc
{

using SpotRectPayload = ::freewb::SpotRectPayload;
using CandidatePayload = ::freewb::CandidatePayload;
using CandidatePreeditPayload = ::freewb::CandidatePreeditPayload;
using CandidateAuxPayload = ::freewb::CandidateAuxPayload;
using ToolbarPropertiesPayload = ::freewb::ToolbarPropertiesPayload;

struct DBusCallbacks
{
    std::function<void(int)> onSelectCandidate;
    std::function<void()> onPageUp;
    std::function<void()> onPageDown;
    std::function<void()> onReloadConfig;
};

class IDBus
{
public:
    virtual ~IDBus() = default;

    virtual bool bindDBusCallbacks(const DBusCallbacks &callbacks) = 0;

    // toolbar
    virtual void emitUpdateProperties(const ToolbarPropertiesPayload &payload) = 0;
    virtual void emitShowToolbar() = 0;
    virtual void emitHideToolbar() = 0;

    // candidate
    virtual void sendSetSpotRect(const SpotRectPayload &payload) = 0;
    virtual void sendSetCandidate(const CandidatePayload &payload) = 0;

    virtual void emitUpdateCandidate(const SpotRectPayload &spotRect, const CandidatePayload &candidate, const CandidatePreeditPayload &preedit, const CandidateAuxPayload &aux) = 0;
    virtual void emitUpdatePreeditText(const CandidatePreeditPayload &payload) = 0;
    virtual void emitUpdatePreeditCaret(int caret) = 0;
    virtual void emitUpdateAux(const CandidateAuxPayload &payload) = 0;
};

} // namespace freewb::ipc

#endif
