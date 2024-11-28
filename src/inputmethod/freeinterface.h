#ifndef _FREE_INTERFACE_H
#define _FREE_INTERFACE_H

#include "freewubi-internal.h"

char *dbusTest(Fcitxfreewubi *fwb, char *text);
void addUsrParse(Fcitxfreewubi *fwb, int flg, char *wordText, char *wordCode);
void addUsrParseDirect(Fcitxfreewubi *fwb, char *wordText, char *wordCode);
void deleteUsrParse(Fcitxfreewubi *fwb, int flg, char *wordText, char *wordCode);
void resetUerWordFlag(Fcitxfreewubi *fwb);
void resetQuickTableFlag(Fcitxfreewubi *fwb);
void resetTableFlag(Fcitxfreewubi *fwb);
void exitFreewbPanel(Fcitxfreewubi *fwb);
void dictQuery(Fcitxfreewubi *fwb, char *wordText);
void switchFreeIm(Fcitxfreewubi *fwb, int imState);
void switchImState(Fcitxfreewubi *fwb, int imState); // 切换输入法状态  0-进入极点五笔   1-切换至英文  2- 退出极点五笔
void switchToolbarState(Fcitxfreewubi *fwb);
void switchCandiwinState(Fcitxfreewubi *fwb);
void switchSkin(Fcitxfreewubi *fwb);
void switchVk(Fcitxfreewubi *fwb, int flg);
void switchSmartPunc(Fcitxfreewubi *fwb, int flg);
void switchCharSet(Fcitxfreewubi *fwb);
void switchRecodeProof(Fcitxfreewubi *fwb, int flg);
void switchUncommon(Fcitxfreewubi *fwb, char *wordText, int flg); // 词组常用与非常用切换成功
void switchChttrans(Fcitxfreewubi *fwb, int flg);
void openSysConf(Fcitxfreewubi *fwb);
void showVersion(Fcitxfreewubi *fwb);
void openProfessionalConf(Fcitxfreewubi *fwb);
void modQuickTable(Fcitxfreewubi *fwb);
void modUserTable(Fcitxfreewubi *fwb);
void modWubiTable(Fcitxfreewubi *fwb);
void modPinyinTable(Fcitxfreewubi *fwb);
void openConfDir(Fcitxfreewubi *fwb);
void closeVkBoard(Fcitxfreewubi *fwb);
void switchTable(Fcitxfreewubi *fwb);
void setCharWidth(Fcitxfreewubi *fwb, int charWidth, int PuncMode);
int createFreewbPanel(Fcitxfreewubi *fwb);
char *getClipboard(Fcitxfreewubi *fwb);
void switchCapState(Fcitxfreewubi *fwb);
#endif
