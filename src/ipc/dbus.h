#ifndef IDBUS_H
#define IDBUS_H

#include "types.h"

namespace freewb::ipc
{

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

    virtual void emitUpdatePreeditText(const PreeditPayload &payload) = 0;
    virtual void emitUpdatePreeditCaret(int caret) = 0;
    virtual void emitUpdateAux(const CandidateAuxPayload &payload) = 0;
};

} // namespace freewb::ipc

#endif
