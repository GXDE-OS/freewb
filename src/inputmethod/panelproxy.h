#ifndef PANEL_PROXY_H
#define PANEL_PROXY_H

#include <fcitx/instance.h>

void FreeWubiPanelProxyInitializeInstance(FcitxInstance* instance);
void FreeWubiPanelProxyDestroyInstance();

void FreeWubiPanelProxyShowInputWindow();
void FreeWubiPanelProxyCloseInputWindow();
void FreeWubiPanelProxyOnTriggerOn();
void FreeWubiPanelProxyOnTriggerOff();

#endif