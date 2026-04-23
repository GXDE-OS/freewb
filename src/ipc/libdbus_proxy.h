#ifndef LIBDBUS_PROXY_H
#define LIBDBUS_PROXY_H

#include <dbus/dbus.h>

void FreeWubiServiceAddUsrParse(DBusConnection *conn, int flg, char *wordText, char *wordCode);
void FreeWubiServiceDeleteUsrParse(DBusConnection *conn, int flg, char *wordText, char *wordCode);
void FreeWubiServiceResetUerWordFlag(DBusConnection *conn);
void FreeWubiServiceResetQuickTableFlag(DBusConnection *conn);
void FreeWubiServiceResetTableFlag(DBusConnection *conn);
void FreeWubiServiceExitFreewbPanel(DBusConnection *conn);
void FreeWubiServiceDictQuery(DBusConnection *conn, char *wordText);
void FreeWubiServiceSwitchFreeIm(DBusConnection *conn, int imState);
void FreeWubiServiceSwitchToolbarState(DBusConnection *conn);
void FreeWubiServiceSwitchCandiwinState(DBusConnection *conn);
void FreeWubiServiceSwitchSkin(DBusConnection *conn);
void FreeWubiServiceSwitchVk(DBusConnection *conn, int flg);
void FreeWubiServiceSwitchSmartPunc(DBusConnection *conn, int flg);
void FreeWubiServiceSwitchCharSet(DBusConnection *conn);
void FreeWubiServiceSwitchRecodeProof(DBusConnection *conn, int flg);
void FreeWubiServiceSwitchUncommon(DBusConnection *conn, char *wordText, int flg); // 词组常用与非常用切换成功
void FreeWubiServiceSwitchChttrans(DBusConnection *conn);
void FreeWubiServiceOpenSysConf(DBusConnection *conn);
void FreeWubiServiceShowVersion(DBusConnection *conn);
void FreeWubiServiceOpenProfessionalConf(DBusConnection *conn);
void FreeWubiServiceModQuickTable(DBusConnection *conn);
void FreeWubiServiceModUserTable(DBusConnection *conn);
void FreeWubiServiceModWubiTable(DBusConnection *conn);
void FreeWubiServiceModPinyinTable(DBusConnection *conn);
void FreeWubiServiceOpenConfDir(DBusConnection *conn);
void FreeWubiServiceCloseVkBoard(DBusConnection *conn);
void FreeWubiServiceSwitchTable(DBusConnection *conn);
void FreeWubiServiceSetCharWidth(DBusConnection *conn, int charWidth, int PuncMode);
int FreeWubiServiceCreateFreewbPanel(DBusConnection *conn);
char *FreeWubiServiceGetClipboard(DBusConnection *conn);
void FreeWubiServiceSwitchCapState(DBusConnection *conn);

#endif // LIBDBUS_PROXY_H
