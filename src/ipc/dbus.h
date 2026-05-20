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
    virtual void callPanelUpdateProperties(const ::freewb::ToolbarPropertiesPayload &payload) = 0;
    virtual void callPanelShowToolbar() = 0;
    virtual void callPanelHideToolbar() = 0;

    // candidate
    virtual void callPanelUpdateSpotRect(const ::freewb::SpotRectPayload &payload) = 0;
    virtual void callPanelUpdateCandidate(const ::freewb::CandidatePayload &payload) = 0;

    virtual void callPanelUpdatePreeditText(const ::freewb::PreeditPayload &payload) = 0;
    virtual void callPanelUpdatePreeditCaret(int caret) = 0;
    virtual void callPanelUpdateAux(const ::freewb::CandidateAuxPayload &payload) = 0;
};

} // namespace freewb::ipc

#endif
