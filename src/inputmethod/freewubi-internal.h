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

#include <dbus/dbus.h>

#include <fcitx/instance.h>

#include "freedict.h"
#include "freewubi-config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        IM_INTO_FREEWB,
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

    typedef enum
    {
        CT_NORMAL = 0,
        CT_AUTOPHRASE,
        CT_REMIND,
        CT_QUICK,
        CT_REPEATE
    } CANDTYPE;

    typedef struct
    {
        AUTOPHRASE *autoPhrase;
        QUCIK_TABLE *qucikPhrase;
        RECORD *record;
        RECORD *simpelRecord;
    } CANDWORD;

    typedef struct
    {
        CANDTYPE flag; // 指示该候选字/词是自动组的词还是正常的字/词
        CANDWORD candWord;
    } TABLECANDWORD;

    typedef struct _Fcitxfreewubi
    {
        FcitxfreewubiConfig config;
        FcitxInstance *owner;
        TableMetaData *table; /* 码表 */
        DBusConnection *conn;
        RECORD *pLastCommitRecord;
        char strTableRemindSource[PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1];
        boolean bIsTableDelPhrase;
        boolean bIsTableAdjustOrder;
        boolean bIsTableAddPhrase;
        boolean bIsTableAddPhraseByClip;
        boolean bIsTempEnglish;
        boolean bIsAutoEnglish; // 英文状态
        char iTableNewPhraseHZCount;
        boolean bHasQuickDelete; // 是否快删过了
        boolean bUseWidePunc;
        boolean bUseFullWidthChar;
        boolean bisDelNumber;
        FcitxHotkey hkTableAddPhraseByClip[HOT_KEY_COUNT];
        FcitxHotkey hkReverseCheckByClip[2]; // 反查码表和拼音及注释
        boolean bNeedMoveCur;
        //     boolean         bFirstMoveCur;//先上屏還是先移动光标

        boolean bNotFirstStart; // 是否是第一次启动极点五笔
        //    boolean bSwichImSeccess; //上一个状态是否创建成功kde服务
    } Fcitxfreewubi;

    boolean LoadFreeWubiGlobalInfo(Fcitxfreewubi *fwb);
    boolean reloadFreewb(Fcitxfreewubi *fwb);
    void sortCandwords(UT_array *arry1, UT_array *arry2, UT_array *result);
    boolean freeDbusInit(Fcitxfreewubi *fwb);
    DBusConnection *getFreeDbusConn(Fcitxfreewubi *fwb);
    int TableCreateAutoPhrase(TableMetaData *tableMetaData, int iCount);
    void freeAutoPhrase(TableMetaData *tableMetaData);
    CONFIG_BINDING_DECLARE(FcitxfreewubiConfig);

#ifdef __cplusplus
}
#endif
#endif
