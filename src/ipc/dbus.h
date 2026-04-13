#ifndef IDBUS_H
#define IDBUS_H

#include "types.h"

namespace freewb::ipc
{

using SpotRectPayload = ::freewb::SpotRectPayload;
using CandidatePayload = ::freewb::CandidatePayload;
using CandidatePreeditPayload = ::freewb::CandidatePreeditPayload;
using CandidateAuxPayload = ::freewb::CandidateAuxPayload;
using ToolbarPropertiesPayload = ::freewb::ToolbarPropertiesPayload;

using DBusSignalCallback = void (*)(const char *member, int index);

class IDBus
{
public:
    virtual ~IDBus() = default;

    virtual bool bindDBusSignalCallback(DBusSignalCallback callback) = 0;

    // toolbar
    virtual void emitUpdateProperties(const ToolbarPropertiesPayload &payload) = 0;
    virtual void emitShowToolbar() = 0;
    virtual void emitHideToolbar() = 0;

    // candidate
    virtual void emitUpdateSpotRect(const SpotRectPayload &payload) = 0;
    virtual void emitUpdateCandidate(const CandidatePayload &payload) = 0;

    virtual void emitUpdatePreeditText(const CandidatePreeditPayload &payload) = 0;
    virtual void emitUpdatePreeditCaret(int caret) = 0;
    virtual void emitUpdateAux(const CandidateAuxPayload &payload) = 0;
};

} // namespace freewb::ipc

#endif
