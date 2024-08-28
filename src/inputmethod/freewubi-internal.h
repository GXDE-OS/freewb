/***************************************************************************
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

#ifndef _FCITX_FREEWUBI_INTERNAL_H_
#define _FCITX_FREEWUBI_INTERNAL_H_

#include "freewubi.h"
#include <fcitx/instance.h>
#include <dbus/dbus.h>
#include "fcitx-config/fcitx-config.h"
#include "fcitx-config/hotkey.h"
#include "freedict.h"
#define _(x) dgettext("fcitx-freewubi", x)
#ifdef __cplusplus
extern "C"{
#endif

typedef enum{
    IM_INTO_FREEWB ,
    IM_TO_ENGLISH,
    IM_CLOSE_FREEWB
} IMState;
typedef enum
{
    SOUND_LETTER,
    SOUND_ENTER,
    SOUND_BACK,
    SOUND_RECODE,
    SOUND_SAPCE,
    SOUND_EMPTY,

    SOUND_NUM
} SoundType;
typedef enum {
    CT_NORMAL = 0,
    CT_AUTOPHRASE,
    CT_REMIND,
    CT_QUICK,
    CT_REPEATE
} CANDTYPE;
typedef struct {
    AUTOPHRASE     *autoPhrase;
    QUCIK_TABLE    *qucikPhrase;
    RECORD         *record;
    RECORD         *simpelRecord;
} CANDWORD;
typedef struct {
    CANDTYPE        flag;   //指示该候选字/词是自动组的词还是正常的字/词
    CANDWORD        candWord;
} TABLECANDWORD;
typedef struct {
    FcitxGenericConfig   gconfig;
//     char           *kbdlayout;
    int             iCandidateWordNumber;
    FcitxHotkey     hkAlternativePrevPage[HOT_KEY_COUNT];
    FcitxHotkey     hkAlternativeNextPage[HOT_KEY_COUNT];
    FcitxHotkey     hkReverseCheck[HOT_KEY_COUNT];    //反查码表和拼音及注释
   

    FcitxHotkey     unCommonKey[HOT_KEY_COUNT];    //生癖字
    FcitxHotkey     QuickInputKey[HOT_KEY_COUNT];    //快速码表
    FcitxHotkey     tempEnglishKey[HOT_KEY_COUNT];    //临时英文
    char           *WubiPath;//五笔码表路径
    char           *PinyinPath;//拼音码表路径
    char	   *usrPath;//用户词组路径
    FcitxHotkey     hkTableDelPhrase[HOT_KEY_COUNT];
//     FcitxHotkey     hkTableAdjustOrder[HOT_KEY_COUNT];   
    FcitxHotkey     hkTableAddPhrase[HOT_KEY_COUNT];
    FcitxHotkey     hkSwitchFreeim[HOT_KEY_COUNT];
    FcitxHotkey     hkQuickDelete[HOT_KEY_COUNT];//快删上屏项
    
    int        bUserWordChanged;//
    int        bBackUpTable;//词库恢复
    int        iImType;
    boolean    bCodeRemind;
    boolean    bUseSmartPunc;
    boolean    bAutoAdjustOrder;
    boolean         bPhraseRemind;//词组联想
    //int inputMode; //输入模式
    FcitxHotkey    hkSetup[HOT_KEY_COUNT];    
    FcitxHotkey    hkSmartPunc[HOT_KEY_COUNT];    
    FcitxHotkey    hkSecondRecode[HOT_KEY_COUNT];
    FcitxHotkey    hkThirdRecode[HOT_KEY_COUNT];
    FcitxHotkey    hkShowHideCandiWin[HOT_KEY_COUNT];
    FcitxHotkey    hkSwitchChttrans[HOT_KEY_COUNT];
    FcitxHotkey    hkShowHideToolbar[HOT_KEY_COUNT];
    FcitxHotkey    hkSwitchSkin[HOT_KEY_COUNT];
    FcitxHotkey    hkSwitchVKb[HOT_KEY_COUNT];
    FcitxHotkey    hkSwitchCharSet[HOT_KEY_COUNT]; //切换字符集
    FcitxHotkey    hkSwitchTable[HOT_KEY_COUNT]; //切换字符集
    boolean     bFullSpace;//空格全角
    int         iAutoPhraseOpt; //自动造词选项   0-禁止自动造词   1-关闭极点时消失    2-保存到词库
    int     bIsGBK;
    boolean bRemindExistWords;
    boolean bRecodeProof;//重码上屏校对
    boolean bInputVoice;
    boolean bRecodeVoice;
    boolean bDisableHk;
    boolean bShiftCommit;//shift+字母上屏
    char *strUsrKeyBoard;
    char *strPuncKeyBoard;
    int bQuickTableChanged;
    int iKeyboardMode;
    char *strAutoEng;
    boolean bAutoHalf;
    boolean bEnterClear;
    int bIsTraditional;
} FcitxfreewubiConfig;
typedef struct _Fcitxfreewubi{
    FcitxfreewubiConfig config;
    FcitxInstance *owner;
    TableMetaData *table ;/* 码表 */
    DBusConnection *conn;
    RECORD         *pLastCommitRecord;
    char            strTableRemindSource[PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1];
    boolean         bIsTableDelPhrase;  
    boolean         bIsTableAdjustOrder;
    boolean         bIsTableAddPhrase;
    boolean         bIsTableAddPhraseByClip;
    boolean         bIsTempEnglish;
    boolean         bIsAutoEnglish; //英文状态
    char            iTableNewPhraseHZCount;
    boolean         bHasQuickDelete;//是否快删过了
    boolean         bUseWidePunc;
    boolean         bUseFullWidthChar;
    boolean         bisDelNumber;
    FcitxHotkey     hkTableAddPhraseByClip[HOT_KEY_COUNT];
    FcitxHotkey     hkReverseCheckByClip[2];    //反查码表和拼音及注释
    boolean bNeedMoveCur;
//     boolean         bFirstMoveCur;//先上屏還是先移动光标

    boolean bNotFirstStart;  //是否是第一次启动极点五笔
//    boolean bSwichImSeccess; //上一个状态是否创建成功kde服务

} Fcitxfreewubi;

boolean LoadFreeWubiGlobalInfo(Fcitxfreewubi *fwb);
boolean reloadFreewb(Fcitxfreewubi *fwb);
void sortCandwords(UT_array *arry1, UT_array *arry2, UT_array *result);
boolean freeDbusInit(Fcitxfreewubi *fwb);
DBusConnection *getFreeDbusConn(Fcitxfreewubi *fwb);
int TableCreateAutoPhrase(TableMetaData* tableMetaData, int iCount);
void freeAutoPhrase(TableMetaData* tableMetaData);
CONFIG_BINDING_DECLARE(FcitxfreewubiConfig);

#ifdef __cplusplus
}
#endif
#endif
