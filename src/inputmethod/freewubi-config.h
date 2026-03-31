/**************************************************************************
 *   Copyright (C) YEAR~YEAR by Your Name                                  *
 *   your-email@address.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#ifndef _FCITX_FREEWUBI_CONFIG_H_
#define _FCITX_FREEWUBI_CONFIG_H_

#include <fcitx-config/fcitx-config.h>
#include <fcitx/ime.h>
#include <libintl.h>

#define _(x) dgettext("fcitx-freewubi", x)

typedef struct
{
    FcitxGenericConfig gconfig;
    //     char           *kbdlayout;
    int iCandidateWordNumber;
    FcitxHotkey hkAlternativePrevPage[HOT_KEY_COUNT];
    FcitxHotkey hkAlternativeNextPage[HOT_KEY_COUNT];
    FcitxHotkey hkReverseCheck[HOT_KEY_COUNT]; // 反查码表和拼音及注释

    FcitxHotkey unCommonKey[HOT_KEY_COUNT];    // 生癖字
    FcitxHotkey QuickInputKey[HOT_KEY_COUNT];  // 快速码表
    FcitxHotkey tempEnglishKey[HOT_KEY_COUNT]; // 临时英文
    char *WubiPath;                            // 五笔码表路径
    char *PinyinPath;                          // 拼音码表路径
    char *usrPath;                             // 用户词组路径
    FcitxHotkey hkTableDelPhrase[HOT_KEY_COUNT];
    //     FcitxHotkey     hkTableAdjustOrder[HOT_KEY_COUNT];
    FcitxHotkey hkTableAddPhrase[HOT_KEY_COUNT];
    FcitxHotkey hkSwitchFreeim[HOT_KEY_COUNT];
    FcitxHotkey hkQuickDelete[HOT_KEY_COUNT]; // 快删上屏项

    int bUserWordChanged; //
    int bBackUpTable;     // 词库恢复
    int iImType;
    boolean bCodeRemind;
    boolean bUseSmartPunc;
    boolean bAutoAdjustOrder;
    boolean bPhraseRemind; // 词组联想
    // int inputMode; //输入模式
    FcitxHotkey hkSetup[HOT_KEY_COUNT];
    FcitxHotkey hkSmartPunc[HOT_KEY_COUNT];
    FcitxHotkey hkSecondRecode[HOT_KEY_COUNT];
    FcitxHotkey hkThirdRecode[HOT_KEY_COUNT];
    FcitxHotkey hkShowHideCandiWin[HOT_KEY_COUNT];
    FcitxHotkey hkSwitchChttrans[HOT_KEY_COUNT];
    FcitxHotkey hkShowHideToolbar[HOT_KEY_COUNT];
    FcitxHotkey hkSwitchSkin[HOT_KEY_COUNT];
    FcitxHotkey hkSwitchVKb[HOT_KEY_COUNT];
    FcitxHotkey hkSwitchCharSet[HOT_KEY_COUNT]; // 切换字符集
    FcitxHotkey hkSwitchTable[HOT_KEY_COUNT];   // 切换字符集
    boolean bFullSpace;                         // 空格全角
    int iAutoPhraseOpt;                         // 自动造词选项   0-禁止自动造词   1-关闭极点时消失    2-保存到词库
    int bIsGBK;
    boolean bRemindExistWords;
    boolean bRecodeProof; // 重码上屏校对
    boolean bInputVoice;
    boolean bRecodeVoice;
    boolean bDisableHk;
    boolean bShiftCommit; // shift+字母上屏
    char *strUsrKeyBoard;
    char *strPuncKeyBoard;
    int bQuickTableChanged;
    int iKeyboardMode;
    char *strAutoEng;
    boolean bAutoHalf;
    boolean bEnterClear;
    int bIsTraditional;
} FcitxfreewubiConfig;

CONFIG_BINDING_DECLARE(FcitxfreewubiConfig);

void freeWbConfigBindSync(FcitxGenericConfig *config);

int FreewbHotkeyGetKeyList(const char *strKey);
char *FreewbHotKeyGetKeyChar(int sym);
#endif
