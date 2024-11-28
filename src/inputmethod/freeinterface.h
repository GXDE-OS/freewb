#ifndef _FREE_INTERFACE_H
#define _FREE_INTERFACE_H

#include <dbus/dbus.h>

#include "freedict.h"

void addUsrParse(DBusConnection* conn, TableMetaData* table, int flg, char *wordText, char *wordCode);
void addUsrParseDirect(DBusConnection* conn, char *wordText, char *wordCode);
void deleteUsrParse(DBusConnection* conn, TableMetaData* table, int flg, char *wordText, char *wordCode);
void resetUerWordFlag(DBusConnection* conn);
void resetQuickTableFlag(DBusConnection* conn);
void resetTableFlag(DBusConnection* conn);
void exitFreewbPanel(DBusConnection* conn);
void dictQuery(DBusConnection* conn, char *wordText);
void switchFreeIm(DBusConnection* conn, int imState);
void switchImState(DBusConnection* conn, int imState); // 切换输入法状态  0-进入极点五笔   1-切换至英文  2- 退出极点五笔
void switchToolbarState(DBusConnection* conn);
void switchCandiwinState(DBusConnection* conn);
void switchSkin(DBusConnection* conn);
void switchVk(DBusConnection* conn, int flg);
void switchSmartPunc(DBusConnection* conn, int flg);
void switchCharSet(DBusConnection* conn);
void switchRecodeProof(DBusConnection* conn, int flg);
void switchUncommon(DBusConnection* conn, char *wordText, int flg); // 词组常用与非常用切换成功
void switchChttrans(DBusConnection* conn, int flg);
void openSysConf(DBusConnection* conn);
void showVersion(DBusConnection* conn);
void openProfessionalConf(DBusConnection* conn);
void modQuickTable(DBusConnection* conn);
void modUserTable(DBusConnection* conn);
void modWubiTable(DBusConnection* conn);
void modPinyinTable(DBusConnection* conn);
void openConfDir(DBusConnection* conn);
void closeVkBoard(DBusConnection* conn);
void switchTable(DBusConnection* conn);
void setCharWidth(DBusConnection* conn, int charWidth, int PuncMode);
int createFreewbPanel(DBusConnection* conn);
char *getClipboard(DBusConnection* conn);
void switchCapState(DBusConnection* conn);

#endif
