

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <fcitx/ime.h>
#include <fcitx/candidate.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/utils.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/instance.h>
#include <fcitx/context.h>
#include <fcitx/module.h>
#include <fcitx/hook.h>
#include <dbus/dbus.h>
#include <fcitx/module/dbus/fcitx-dbus.h>
#include <libintl.h>
#include <fcitx/keys.h>
#include "config.h"
#include "freewubi-internal.h"
#include "freedict.h"
#include "freespecial.h"
#include "freeinterface.h"
#include "virtual_keyboard.h"
#include "ini.h"
#include "freewubi-config.h"

#include "panelproxy.h"
#include "inputstate.h"


#define SOUND_FILE_PATH "/usr/share/freewb/sound"

static const char *soundData[SOUND_NUM] =
    {
        "letter.wav", // SOUND_LETTER
        "enter.wav",  // SOUND_ENTER
        "back.wav",   // SOUND_BACK
        "recode.wav", // SOUND_RECODE
        "space.wav",  // SOUND_SAPCE
        "empty.wav"   // SOUND_EMPTY
};
#define STR_ADD_PHRASE "aa."
#define STR_DELETE_PHRASE "dd."
#define STR_SWITCH_UNCOMMON "ss."
#define STR_RECODE_PROOF "tt."
#define STR_CHTTRANS "jj."
#define STR_REVERSE_CHECK "ff."
#define STR_HIDE_TOOLBAR "hh."
#define STR_SWITCH_KEYBORD "kk."
#define STR_SWITCH_CHAR_SET "mm."
#define STR_OPEN_CONFIG "oo."
#define STR_VERSION "vv."
#define STR_HIDE_CANDWIN "cc."
#define STR_PROFEESINAL "pp."
#define STR_QUICK_TABLE "qq."
#define STR_USER_TABLE "uu."
#define STR_WUBI_TABLE "uw."
#define STR_PINYIN_TABLE "up."
#define STR_CONF_DIR "dos."
static void TableMetaDataFree(TableMetaData *table);
static void *FcitxFreeWubiCreate(FcitxInstance *instance);
static void FcitxFreeWubiDestroy(void *arg);
static INPUT_RETURN_VALUE FreeWubiGetCandWord(void *arg, FcitxCandidateWord *candWord);
static INPUT_RETURN_VALUE FreeWubiGetCandWords(void *arg);
static boolean TableCheckNoMatch(TableMetaData *table, const char *code);
static int TableFindPhraseByCodeNum(const TableDict *tableDict, const char *strCode, boolean gbk);
static void sortCandwords(UT_array *arry1, UT_array *arry2, UT_array *result);
static int TableCreateAutoPhrase(Fcitxfreewubi *fwb, int iCount);
static void freeAutoPhrase(TableMetaData *tableMetaData);

static FcitxHotkey FreewbCTRL_ENTER[2];
static FcitxHotkey FreewbCAPS_LOCK[2];
static void InternalInit(Fcitxfreewubi *fwb);
static int digitalNumberTrans(FcitxKeySym sym);
static void playSound(SoundType sType);

static boolean reloadFreewb(Fcitxfreewubi *fwb);
static void Fcitx4IMOnChanged(void *arg);
static boolean tray_menu_handler_empty(void *arg);
static void freewb_settings_handler(void *arg);
// #define DEBUG

void FreeWubiInstanceCommitString(FcitxInstance* instance, FcitxInputContext* ic, const char* str) {
    FcitxInstanceCommitString(instance, ic, str);
}

void freeGetOption(Fcitxfreewubi *fwb)
{

    char *path = getFreewbPath();
    char *inifile;
    fcitx_utils_alloc_cat_str(inifile, path, "/config/", "config.ini");
    FcitxLog(INFO, "func : %s line : %d ini filename: %s ", __FUNCTION__, __LINE__, inifile);
    INI *ini = fileToIni(inifile);

    fwb->config.bUseSmartPunc = GetIniKeyBool(ini, "Common", "smartMark");
    fwb->config.bFullSpace = GetIniKeyBool(ini, "Common", "spaceFullWhenCharHalf");
    fwb->config.bRecodeProof = GetIniKeyBool(ini, "Advanced", "recodeCalib");
    fwb->config.bInputVoice = GetIniKeyBool(ini, "Advanced", "typeEffect");
    fwb->config.iAutoPhraseOpt = GetIniKeyInt(ini, "Advanced", "autoWordGroupOpt", 1);

    fwb->config.bUserWordChanged = GetIniKeyBool(ini, "Misc", "userWordFlg");
    fwb->config.iImType = GetIniKeyInt(ini, "Misc", "inputMode", 1);
    fwb->config.bBackUpTable = GetIniKeyBool(ini, "Misc", "imeTableChanged");
    fwb->config.bIsGBK = GetIniKeyBool(ini, "Misc", "currentCharset");
    fwb->config.bQuickTableChanged = GetIniKeyBool(ini, "Misc", "quickFlg");
    fwb->config.iKeyboardMode = GetIniKeyInt(ini, "Misc", "vkMode", 1);
    fwb->config.bIsTraditional = GetIniKeyBool(ini, "Misc", "simpTradFlg");

    fcitx_utils_string_swap(&fwb->config.WubiPath, GetIniKeyString(ini, "Misc", "wubiTable", "freeime.mb"));
    fcitx_utils_string_swap(&fwb->config.PinyinPath, GetIniKeyString(ini, "Misc", "pinyinTable", "attach.mb"));
    fcitx_utils_string_swap(&fwb->config.usrPath, GetIniKeyString(ini, "Misc", "UsrFile", "user_word.txt"));

    fcitx_utils_string_swap(&fwb->config.strUsrKeyBoard, GetIniKeyString(ini, "Misc", "CoustomChar", ""));
    fcitx_utils_string_swap(&fwb->config.strPuncKeyBoard, GetIniKeyString(ini, "Misc", "CoustomMark", ""));

    fwb->config.bAutoHalf = GetIniKeyBool(ini, "Others", "autoToHalfMarkFlg");
    fcitx_utils_string_swap(&fwb->config.strAutoEng, GetIniKeyString(ini, "Others", "autoToEnStr", "www http mail."));

    fwb->config.bDisableHk = GetIniKeyBool(ini, "ShortcutKey", "disableAllShortcutKey");

    char key[256], *ptr;
    strcpy(key, GetIniKeyString(ini, "ShortcutKey", "showHideCandiWin", "KEY_NONE"));
    ptr = key;
    ptr += 5;
    fwb->config.hkShowHideCandiWin[0].sym = FreewbHotkeyGetKeyList(ptr);
    fwb->config.hkShowHideCandiWin[0].state = FcitxKeyState_Ctrl;

    strcpy(key, GetIniKeyString(ini, "ShortcutKey", "showHideToolbar", "KEY_NONE"));
    ptr = key;
    ptr += 5;
    fwb->config.hkShowHideToolbar[0].sym = FreewbHotkeyGetKeyList(ptr);
    fwb->config.hkShowHideToolbar[0].state = FcitxKeyState_Ctrl;

    strcpy(key, GetIniKeyString(ini, "ShortcutKey", "markAutoPair", "KEY_NONE"));
    ptr = key;
    ptr += 5;
    fwb->config.hkSmartPunc[0].sym = FreewbHotkeyGetKeyList(ptr);
    fwb->config.hkSmartPunc[0].state = FcitxKeyState_Ctrl;

    strcpy(key, GetIniKeyString(ini, "ShortcutKey", "switchLexicon", "KEY_NONE"));
    ptr = key;
    ptr += 5;
    fwb->config.hkSwitchTable[0].sym = FreewbHotkeyGetKeyList(ptr);
    fwb->config.hkSwitchTable[0].state = FcitxKeyState_Ctrl;

    free(ini);
    ini = NULL;
    free(path);
    free(inifile);
}

void run_freewb_panel()
{
    char panelBin[512] = {0};

    strcpy(panelBin, "/usr/bin/freewb.sh");

    system(panelBin);
}

static boolean FreeWubiInit(void *arg)
{
    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
    freeGetOption(fwb);
    if (!fwb->table)
    {
        fwb->table = fcitx_utils_new(TableMetaData);
        fwb->table->freeWubiConfig = &(fwb->config);
        LoadTableDict(fwb->table);
        fwb->table->tableType = FREE_WUBI;
    }

    char FileName[256];
    // sprintf(FileName,"%s/.config/fcitx/conf/fcitx-freewubi.config",getenv("HOME"));
    sprintf(FileName, "%s/.local/freewb/config/config.ini", getenv("HOME"));
    // puts(FileName);
    FcitxLog(INFO, "func : %s line : %d ini filename: %s ", __FUNCTION__, __LINE__, FileName);

    INI *ini = fileToIni(FileName);
    int i = GetIniKeyInt(ini, "Misc", "inputMode", 1);
    if (i == -1)
        i = 1;

    fwb->config.iImType = i;
    fwb->table->tableType = fwb->config.iImType;

    i = GetIniKeyInt(ini, "Misc", "currentCharset", 0);
    if (i == -1)
        i = 0;
    fwb->config.bIsGBK = i;

    i = GetIniKeyInt(ini, "Misc", "simpTradFlg", 0);
    if (i == -1)
        i = 0;
    fwb->config.bIsTraditional = i;

    free(ini);
    ini = NULL;

    FreeWubiServiceSwitchFreeIm(FcitxDBusGetConnection(fwb->owner), fwb->table->tableType);
    fwb->bIsAutoEnglish = false;
    return true;
}

static void FreeWubiResetStatus(void *arg)
{
    // #ifdef DEBUG
    //     FcitxLog(INFO,_("FreeWubiResetStatus"));
    // #endif

    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
    FcitxInputState *input = FcitxInstanceGetInputState(fwb->owner);
    fwb->bIsTableAddPhrase = false;
    fwb->bIsTableDelPhrase = false;
    fwb->bIsTableAdjustOrder = false;
    fwb->bIsTempEnglish = false;
    fwb->bIsTableAddPhraseByClip = false;
    //     fwb->bIsAutoEnglish = false;
    FcitxInputStateSetIsDoInputOnly(input, false);

    FreeWubiResetInputState(FreeWubiGetInputState());
    FreeWubiPanelProxyCloseInputWindow();
}

//查找是否存在当前编码的候选项
int TableFindPhraseByCodeNumAndStr(const TableDict *tableDict, const char *strCode, const char *str)
{
    RECORD *recTemp;
    int i = 0, find = 0;

    while (tableDict->recordIndex[i].record && strCode[0] != tableDict->recordIndex[i].cCode)
    {
        i++;
        if (i >= strlen(tableDict->strInputCode))
            return find;
    }

    recTemp = tableDict->recordIndex[i].record;
    while (recTemp != tableDict->recordHead)
    {
        if (recTemp->strCode[0] != strCode[0])
            break;

        if (strcmp(recTemp->strCode, strCode) == 0 && strcmp(recTemp->strHZ,str)==0)
        {

            return 1;
        }
        recTemp = recTemp->next;
    }
    return find;
}


static INPUT_RETURN_VALUE FreeWubiDoInput(void *arg, FcitxKeySym sym, unsigned int state)
{
    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
    if (!fwb->bNotFirstStart)
    { // 如果是第一次启动
// fwb->bSwichImSeccess =
// FreeWubiServiceSwitchImState(FcitxDBusGetConnection(fwb->owner), IM_INTO_FREEWB);
#ifdef DEBUG
        FcitxLog(INFO, "第一次启动");
#endif
        fwb->bNotFirstStart = true;
    }
//printf("sym=%d state=%d\n",sym,state);

    TableMetaData *table = fwb->table;
    INPUT_RETURN_VALUE retVal;
    FcitxInstance *instance = fwb->owner;
    FcitxInputState *input = FreeWubiGetInputState();
    FcitxProfile *profile = FcitxInstanceGetProfile(instance);
    char *strCodeInput = FcitxInputStateGetRawInputBuffer(input);
    //     FcitxGlobalConfig *fcitxConfig  = FcitxInstanceGetGlobalConfig(instance);
    FcitxCandidateWordList *candList = FreeWubiInputStateGetCandidateList(input);
    char *output_str = FcitxInputStateGetOutputString(input);
    int puncNumber = -1;
    FcitxCandidateWordSetChooseAndModifier(candList, DIGIT_STR_CHOOSE, FcitxKeyState_None);
    FcitxInstanceSetContext(fwb->owner, CONTEXT_ALTERNATIVE_PREVPAGE_KEY, fwb->config.hkAlternativePrevPage);
    FcitxInstanceSetContext(fwb->owner, CONTEXT_ALTERNATIVE_NEXTPAGE_KEY, fwb->config.hkAlternativeNextPage);

    // printf("key:%d=%d\n",sym ,fwb->config.hkAlternativeNextPage[0].sym);
    //printf("state:%d\n",state);
    // printf("hotkey:%d\n",FcitxHotkeyIsHotKey(sym, state, fwb->config.hkAlternativeNextPage));
    if (sym == FcitxKey_Escape)
    {
        FreeWubiPanelProxyCloseInputWindow();
        return IRV_CLEAN;
    }

    if (strlen(strCodeInput) == 0 && sym == FcitxKey_Escape && state == FcitxKeyState_None && !fwb->bIsTableAddPhrase && !fwb->bIsTableDelPhrase && !fwb->bIsTempEnglish)
    {
        return IRV_DO_NOTHING;
    }

    // 2024-5-6
    if (fwb->bIsTableAddPhrase && state == FcitxKeyState_None && sym != FcitxKey_Left && sym != FcitxKey_Right && sym != FcitxKey_Escape && sym != FcitxKey_Return)
    {
        return IRV_DO_NOTHING;
    }

    if (fwb->bisDelNumber)
    {
        if (state == FcitxKeyState_None && sym >= '0' && sym <= '9')
        {
            int index = sym - '1';
            if (sym == '0')
            {
                index = 9;
            }
            if (index < 0 || index > FcitxCandidateWordGetCurrentWindowSize(candList) - 1)
                return IRV_DO_NOTHING;
            else
            {
                TABLECANDWORD *tableCandWord = FcitxCandidateWordGetByIndex(candList, index)->priv;
                if (tableCandWord->flag == CT_NORMAL)
                {
                    RECORD *recTemp = tableCandWord->candWord.record;
                    fwb->bIsTableDelPhrase = false;
                    FcitxInputStateSetIsDoInputOnly(input, false);
                    if (recTemp)
                        FreeWubiServiceDeleteUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 1, recTemp->strHZ, recTemp->strCode);
                    return FreeWubiGetCandWords(fwb);
                }
            }
        }
        else
            fwb->bisDelNumber = false;
    }

    if (fwb->config.iKeyboardMode != -1)
    {
        if (FcitxHotkeyIsHotKey(sym, state, FCITX_ESCAPE))
            FreeWubiServiceCloseVkBoard(FcitxDBusGetConnection(fwb->owner));
        if (FcitxHotkeyIsHotKeySimple(sym, state))
        {
            if (fwb->config.iKeyboardMode)
            {
                char *matchStr = matchVkBoard(sym, fwb->config.iKeyboardMode);
                if (matchStr)
                {
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), matchStr);
                    FreeWubiResetInputState(FreeWubiGetInputState());
                    return IRV_CLEAN;
                }
            }
        }
    }

    if (fwb->bIsAutoEnglish)
    {
        if (FcitxHotkeyIsHotKey(sym, state, FCITX_ENTER))
        {

            profile->bUseWidePunc = fwb->bUseWidePunc;
            profile->bUseFullWidthChar = fwb->bUseFullWidthChar;
            // FreeWubiServiceSetCharWidth(FcitxDBusGetConnection(fwb->owner),!profile->bUseFullWidthChar,!profile->bUseWidePunc);
            fwb->bIsAutoEnglish = false;
            // FreeWubiServiceSwitchFreeIm(FcitxDBusGetConnection(fwb->owner),table->tableType);
            printf("table=%d im=%d\n", table->tableType, fwb->config.iImType);
            FreeWubiServiceSwitchFreeIm(FcitxDBusGetConnection(fwb->owner), 1);

            return IRV_TO_PROCESS;
        }
        else if (FcitxHotkeyIsHotKeySimple(sym, state))
        {
            profile->bUseWidePunc = false;
            profile->bUseFullWidthChar = false;
            size_t raw_size = FcitxInputStateGetRawInputBufferSize(input);
            strCodeInput[raw_size] = (char)sym;
            raw_size++;
            strCodeInput[raw_size] = '\0';
            FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), strCodeInput);
            FreeWubiResetInputState(FreeWubiGetInputState());
            return IRV_CLEAN;
        }
        else
            return IRV_TO_PROCESS;
    }

    if (state == FcitxKeyState_None)
    {
        size_t raw_size = FcitxInputStateGetRawInputBufferSize(input);
        strCodeInput[raw_size] = (char)sym;
        raw_size++;
        strCodeInput[raw_size] = '\0';
        FcitxInputStateSetRawInputBufferSize(input, raw_size);
        if (isAutoEngStr(table, strCodeInput))
        {
            fwb->bUseWidePunc = profile->bUseWidePunc;
            fwb->bUseFullWidthChar = profile->bUseFullWidthChar;
            profile->bUseWidePunc = false;
            profile->bUseFullWidthChar = false;
            // FreeWubiServiceSetCharWidth(FcitxDBusGetConnection(fwb->owner),!profile->bUseFullWidthChar,!profile->bUseWidePunc);
            FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), strCodeInput);
            FreeWubiResetInputState(FreeWubiGetInputState());
            fwb->bIsAutoEnglish = true;
            FreeWubiServiceSwitchFreeIm(FcitxDBusGetConnection(fwb->owner), 3);
            return IRV_CLEAN;
        }
        else
        {
            strCodeInput[--raw_size] = '\0';
            FcitxInputStateSetRawInputBufferSize(input, raw_size);
        }
    }
    if (FcitxHotkeyIsHotKeyUAZ(sym, state) && !fwb->bIsAutoEnglish && !fwb->bIsTableAddPhrase && !fwb->bIsTableDelPhrase && !fwb->bIsTempEnglish)
    {
puts("8888888");
        if (FcitxCandidateWordPageCount(candList))
        {
            FcitxCandidateWord *candWord = FcitxCandidateWordGetCurrentWindow(candList);
            if (candWord->owner == fwb)
            {
                INPUT_RETURN_VALUE ret = FreeWubiGetCandWord(fwb, candWord);
                if (ret & IRV_FLAG_PENDING_COMMIT_STRING)
                {
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                    FreeWubiResetInputState(FreeWubiGetInputState());
                }
            }
        }
        if (fwb->config.bShiftCommit)
        {
            char comStr[2] = {sym, 0};
            FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), comStr);
            return IRV_CLEAN;
        }
    }

    //if ((state==FcitxKeyState_Ctrl && sym==32) || (state != FcitxKeyState_None && fwb->config.bDisableHk))
    if ((state != FcitxKeyState_None && fwb->config.bDisableHk))
    {
        return IRV_FLAG_FORWARD_KEY;
    }
    if (FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSwitchFreeim))
    {
        table->tableType = (table->tableType + 1) % 3;
        FreeWubiServiceSwitchFreeIm(FcitxDBusGetConnection(fwb->owner), table->tableType);
        return IRV_DO_NOTHING;
    }

    retVal = IRV_DO_NOTHING;
    if ((sym == fwb->config.QuickInputKey[0].sym && state == FcitxKeyState_None && !FcitxInputStateGetRawInputBufferSize(input)))
    {
        size_t raw_size = FcitxInputStateGetRawInputBufferSize(input);
        strCodeInput[raw_size] = (char)sym;
        raw_size++;
        strCodeInput[raw_size] = '\0';
        FcitxInputStateSetRawInputBufferSize(input, raw_size);
        FreeWubiGetCandWords(arg);
        FreeWubiPanelProxyShowInputWindow();

        return IRV_DO_NOTHING;
    }

    if (fwb->config.QuickInputKey[0].sym && strCodeInput[0] == fwb->config.QuickInputKey[0].sym)
    {
        if ((FcitxHotkeyIsHotKey(sym, state, FCITX_SPACE) || (sym == fwb->config.QuickInputKey[0].sym && state == FcitxKeyState_None)) && FcitxCandidateWordPageCount(candList) != 0)
        {
            if (fwb->config.bInputVoice)
                playSound(SOUND_SAPCE);
            return FcitxCandidateWordChooseByIndex(candList, 0);
        }
        if (FcitxHotkeyIsHotKeySimple(sym, state))
        {
            const QUCIK_TABLE* table = findQuickPharse(fwb->table, sym);
            if (table != NULL) {
                FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), table->value);
            }

            return IRV_CLEAN;
        }
    }

    if (state == FcitxKeyState_None)
        puncNumber = isPuncKey(sym);
    if (!fwb->bIsTempEnglish && -1 != puncNumber && fwb->config.bUseSmartPunc &&
        !(FcitxCandidateWordPageCount(candList) && FcitxHotkeyIsHotKey(sym, state, fwb->config.hkThirdRecode)))
    {
        if (FcitxCandidateWordPageCount(candList))
        {
            retVal = FcitxCandidateWordChooseByIndex(candList, 0);
            if (retVal == IRV_TO_PROCESS)
                retVal = IRV_CLEAN;
        }
        if (IRV_DO_NOTHING == retVal)
            output_str[0] = '\0';
        matchPunc(output_str, puncNumber, profile->bUseWidePunc);
        FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
        FreeWubiResetInputState(FreeWubiGetInputState());
        fwb->bNeedMoveCur = true;
        //         FcitxInstanceForwardKey(instance, FcitxInstanceGetCurrentIC(instance), FCITX_PRESS_KEY, FcitxKey_Left, FcitxKeyState_None);
        return IRV_CLEAN;
    } // 成对标点

    if (state == FcitxKeyState_None && !fwb->bIsTempEnglish && !fwb->bIsAutoEnglish &&
        (IsInputKey(table, sym) || FcitxHotkeyIsHotKeyUAZ(sym, state) || (table->tableType != FREE_PINYIN && (IsUncommonKey(table, sym, state) || (strCodeInput[0] == fwb->config.unCommonKey[0].sym && '/' == sym)))))
    {
        if (fwb->config.bInputVoice)
            playSound(SOUND_LETTER);
        if (FcitxInputStateGetIsInRemind(input))
            FcitxCandidateWordReset(candList);
        FcitxInputStateSetIsInRemind(input, false);
        /* it's not in special state */
        if (!fwb->bIsTableAddPhrase && !fwb->bIsTableDelPhrase &&
            !fwb->bIsTableAdjustOrder)
        {
            if (IsUncommonKey(table, sym, state) && FcitxInputStateGetRawInputBufferSize(input) == 1 && strCodeInput[0] == fwb->config.unCommonKey[0].sym)
            {
                return FcitxCandidateWordChooseByIndex(candList, 0);
            }
            size_t raw_size = FcitxInputStateGetRawInputBufferSize(input);
            strCodeInput[raw_size] = (char)sym;
            raw_size++;
            strCodeInput[raw_size] = '\0';
            FcitxInputStateSetRawInputBufferSize(input, raw_size);
            boolean bhasMatch = !TableCheckNoMatch(table, FcitxInputStateGetRawInputBuffer(input));
            if (!strncmp(strCodeInput, "zz", 2))
                FcitxCandidateWordSetPageSize(candList, 10);
            else
                FcitxCandidateWordSetPageSize(candList, fwb->config.iCandidateWordNumber);
            // modify 2024-2-7
            int candCount = 1;
            // 是否超过五笔字符长度
            // 是否超过拼音或生僻字字符长度
// #define OLD_INFORM_METHOD 1
#ifdef OLD_INFORM_METHOD
            if ((table->tableType != FREE_PINYIN && raw_size <= table->WubiDict->iCodeLength) || bhasMatch)
            {
                retVal = FreeWubiGetCandWords(fwb);
                if (fwb->config.bRecodeVoice)
                {
                    if (table->tableType != FREE_PINYIN && raw_size == table->WubiDict->iCodeLength)
                    {
                        if (FcitxCandidateWordGetByIndex(candList, 1))
                        {
                            TABLECANDWORD *tableCandWord2 = FcitxCandidateWordGetByIndex(candList, 1)->priv;
                            if (tableCandWord2->flag == CT_NORMAL && strcmp(strCodeInput, tableCandWord2->candWord.record->strCode) == 0 && tableCandWord2->candWord.record->owner == table->WubiDict)
                            {
                                playSound(SOUND_RECODE);
                            }
                        }
                        if (!bhasMatch)
                            playSound(SOUND_EMPTY);
                    }
                }

                retVal = IRV_DISPLAY_CANDWORDS;
            }
            else
            {
                strCodeInput[--raw_size] = '\0';
                retVal = IRV_DISPLAY_CANDWORDS;
                char strReCodeProof[50] = {0};
                if (FcitxCandidateWordPageCount(candList))
                {
                    FcitxCandidateWord *candWord = FcitxCandidateWordGetCurrentWindow(candList);
                    if (candWord->owner == fwb)
                    {
                        TABLECANDWORD *tableCandWord = candWord->priv;
                        if (tableCandWord->candWord.record && (!strcmp(tableCandWord->candWord.record->strCode, strCodeInput) ||
                                                               (strCodeInput[0] == fwb->config.unCommonKey[0].sym && !strcmp(tableCandWord->candWord.record->strCode, strCodeInput + 1))))
                        {
                            if (fwb->config.bRecodeProof && FcitxCandidateWordGetByIndex(candList, 1))
                            {
                                TABLECANDWORD *tableCandWord2 = FcitxCandidateWordGetByIndex(candList, 1)->priv;
                                if (tableCandWord2->flag == CT_NORMAL && strcmp(strCodeInput, tableCandWord2->candWord.record->strCode) == 0)
                                {
                                    strcat(strReCodeProof, "【");
                                    strcat(strReCodeProof, FcitxCandidateWordGetByIndex(candList, 1)->strWord);
                                    strcat(strReCodeProof, "】");
                                }
                            }
                            INPUT_RETURN_VALUE ret = FreeWubiGetCandWord(fwb, candWord);
                            if (ret & IRV_FLAG_PENDING_COMMIT_STRING)
                            {
                                FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                                strCodeInput[0] = sym;
                                strCodeInput[1] = '\0';
                                FcitxInputStateSetRawInputBufferSize(input, 1);
                            }
                        }
                        else
                        {
                            strCodeInput[raw_size] = (char)sym;
                            raw_size++;
                            strCodeInput[raw_size] = '\0';
                            FcitxInputStateSetRawInputBufferSize(input, raw_size);
                            if (fwb->config.bRecodeVoice)
                                playSound(SOUND_EMPTY);
                        }
                    }
                }
                else
                {
                    if (fwb->config.bRecodeProof)
                    {
                        strcat(strReCodeProof, "【");
                        strcat(strReCodeProof, strCodeInput);
                        strcat(strReCodeProof, "】");
                    }
                    FcitxInputStateSetRawInputBufferSize(input, 1);
                    strCodeInput[0] = sym;
                    strCodeInput[1] = '\0';
                    FcitxInputStateSetIsInRemind(input, false);
                }
                if (fwb->config.bRecodeProof)
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), strReCodeProof);
            }
#else
            int maxCodeLen = table->WubiDict->iCodeLength;
            maxCodeLen += (strCodeInput[0] == fwb->config.unCommonKey[0].sym) ? 1 : 0;
            if ((table->tableType != FREE_PINYIN && raw_size <= maxCodeLen) || bhasMatch)
            {
                retVal = FreeWubiGetCandWords(fwb);
                if (fwb->config.bRecodeVoice && raw_size == table->WubiDict->iCodeLength)
                {
                    // printf("%s=%d[%d] \n",strCodeInput,TableFindPhraseByCodeNum(table->WubiDict,strCodeInput,fwb->config.bIsGBK),FcitxCandidateWordPageCount(candList));
                    candCount = TableFindPhraseByCodeNum(table->WubiDict, strCodeInput, fwb->config.bIsGBK);
                }

                if (retVal == IRV_CLEAN) {
                    return IRV_CLEAN;
                }

                // FreeWubiPanelProxyShowInputWindow();

                retVal = IRV_DO_NOTHING;
            }
            else
            {
                strCodeInput[--raw_size] = '\0';
                retVal = IRV_DO_NOTHING;
                char strReCodeProof[50] = {0};
                candCount = 99;
                if (FcitxCandidateWordPageCount(candList))
                {
                    FcitxCandidateWord *candWord = FcitxCandidateWordGetCurrentWindow(candList);
                    if (candWord->owner == fwb)
                    {
                        TABLECANDWORD *tableCandWord = candWord->priv;
                        if (tableCandWord->candWord.record && (!strcmp(tableCandWord->candWord.record->strCode, strCodeInput) || (strCodeInput[0] == fwb->config.unCommonKey[0].sym) && !strcmp(tableCandWord->candWord.record->strCode, strCodeInput)))
                        {
                            if (fwb->config.bRecodeProof && FcitxCandidateWordGetByIndex(candList, 1))
                            {
                                TABLECANDWORD *tableCandWord2 = FcitxCandidateWordGetByIndex(candList, 1)->priv;
                                // printf("%s=%s \n",strCodeInput,tableCandWord2->candWord.record->strHZ);

                                if (tableCandWord2->flag == CT_NORMAL && strcmp(strCodeInput, tableCandWord2->candWord.record->strCode) == 0)
                                {
                                    strcat(strReCodeProof, "【");
                                    strcat(strReCodeProof, FcitxCandidateWordGetByIndex(candList, 1)->strWord);
                                    strcat(strReCodeProof, "】");
                                }
                            }
                            INPUT_RETURN_VALUE ret = FreeWubiGetCandWord(fwb, candWord);
                            if (ret & IRV_FLAG_PENDING_COMMIT_STRING)
                            {
                                FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                                strCodeInput[0] = sym;
                                strCodeInput[1] = '\0';
                                FcitxInputStateSetRawInputBufferSize(input, 1);
                                FreeWubiGetCandWords(fwb);
                            }
                        }
                        else
                        {
                            strCodeInput[raw_size] = (char)sym;
                            raw_size++;
                            strCodeInput[raw_size] = '\0';
                            FcitxInputStateSetRawInputBufferSize(input, raw_size);
                        }
                    }
                }
                else
                {
                    FcitxInputStateSetRawInputBufferSize(input, 1);
                    strCodeInput[0] = sym;
                    strCodeInput[1] = '\0';
                    FcitxInputStateSetIsInRemind(input, false);
                    FreeWubiGetCandWords(fwb);
                }
                if (fwb->config.bRecodeProof)
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), strReCodeProof);


                // FreeWubiPanelProxyShowInputWindow();
            }

            if (fwb->config.bRecodeVoice)
            {
                // printf("9999  raw-size=%d max=%d\n",raw_size,maxCodeLen);
                if (candCount > 1 && candCount != 99)
                {
                    // printf("repeat=%d\n",candCount);
                    playSound(SOUND_RECODE);
                }
                else if (candCount == 0 && raw_size == maxCodeLen)
                {
                    playSound(SOUND_EMPTY);
                }
            }
#endif
            // end_2024-2-7
        } // 编码输入
    }
    else
    {
        if (FcitxHotkeyIsHotKey(sym, state, FCITX_ENTER) && fwb->config.bInputVoice)
            playSound(SOUND_ENTER);
        if (sym == fwb->config.tempEnglishKey[0].sym && state == FcitxKeyState_None && !FcitxInputStateGetRawInputBufferSize(input) && !fwb->bIsTempEnglish)
        {
            fwb->bIsTempEnglish = true;
            size_t raw_size = FcitxInputStateGetRawInputBufferSize(input);
            strCodeInput[raw_size] = (char)sym;
            raw_size++;
            strCodeInput[raw_size] = '\0';
            FcitxInputStateSetRawInputBufferSize(input, raw_size);
            FreeWubiGetCandWords(arg);
            FreeWubiPanelProxyShowInputWindow();

            return IRV_DO_NOTHING;
        }
        else if (fwb->bIsTempEnglish)
        {
            if (FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE))
            {
                if (fwb->config.bInputVoice)
                    playSound(SOUND_BACK);
                if (!FcitxInputStateGetRawInputBufferSize(input))
                {
                    FcitxInputStateSetIsInRemind(input, false);
                    return IRV_DONOT_PROCESS_CLEAN;
                }
                FcitxInputStateSetRawInputBufferSize(input, FcitxInputStateGetRawInputBufferSize(input) - 1);
                strCodeInput[FcitxInputStateGetRawInputBufferSize(input)] = '\0';
                if (FcitxInputStateGetRawInputBufferSize(input)) {
                    FreeWubiGetCandWords(arg);
                    FreeWubiPanelProxyShowInputWindow();
                    return IRV_DO_NOTHING;
                }
                else
                    return IRV_CLEAN;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_SPACE) && FcitxCandidateWordPageCount(candList) != 0)
            {
                if (fwb->config.bInputVoice)
                    playSound(SOUND_SAPCE);
                fwb->bIsTempEnglish = false;
                FcitxCandidateWordChooseByIndex(candList, 0);
                FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                return IRV_CLEAN;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSecondRecode) && FcitxCandidateWordGetByIndex(candList, 1))
            {
                fwb->bIsTempEnglish = false;
                FcitxCandidateWordChooseByIndex(candList, 1);
                FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                return IRV_CLEAN;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, fwb->config.hkThirdRecode) && FcitxCandidateWordGetByIndex(candList, 2))
            {
                fwb->bIsTempEnglish = false;
                FcitxCandidateWordChooseByIndex(candList, 2);
                FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                return IRV_CLEAN;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_ENTER))
            {
                if (fwb->config.bInputVoice)
                    playSound(SOUND_ENTER);

                FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), strCodeInput + 1);
                fwb->bIsTempEnglish = false;
                return IRV_CLEAN;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_ESCAPE))
            {
                fwb->bIsTempEnglish = false;
                return IRV_CLEAN;
            }
            else if (sym == fwb->config.tempEnglishKey[0].sym && state == FcitxKeyState_None && FcitxInputStateGetRawInputBufferSize(input) == 1)
            {
                if (fwb->config.bInputVoice)
                    playSound(SOUND_SAPCE);
                retVal = FcitxCandidateWordChooseByIndex(candList, 0);
                fwb->bIsTempEnglish = false;
            }
            else if (FcitxHotkeyIsHotKeySimple(sym, state))
            {
                size_t raw_size = FcitxInputStateGetRawInputBufferSize(input);
                strCodeInput[raw_size] = (char)sym;
                raw_size++;
                strCodeInput[raw_size] = '\0';
                FcitxInputStateSetRawInputBufferSize(input, raw_size);
                FreeWubiGetCandWords(arg);
                FreeWubiPanelProxyShowInputWindow();
                retVal = IRV_DO_NOTHING;
            }
        } // tmp english
        if (fwb->bIsTableAddPhrase)
        {
            if (FcitxHotkeyIsHotKey(sym, state, FCITX_LEFT))
            {
                if (fwb->bIsTableAddPhraseByClip)
                {
                    char *str = FreeWubiServiceGetClipboard(FcitxDBusGetConnection(fwb->owner));
                    int strLen = fcitx_utf8_strlen(str);
                    if (fwb->iTableNewPhraseHZCount < strLen && fwb->iTableNewPhraseHZCount < PHRASE_MAX_LENGTH)
                    {
                        fwb->iTableNewPhraseHZCount++;
                        int i;
                        char strHZ[PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1];
                        memset(strHZ, 0, PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1);
                        char strCode[5];
                        memset(strCode, 0, 5);
                        for (i = fwb->iTableNewPhraseHZCount; i >= 0 && i < strLen; i++)
                        {
                            str = str + fcitx_utf8_char_len(str);
                        }
                        strcpy(strHZ, str);
                        if (TableCalPhraseCode(table->WubiDict, strHZ, strCode))
                            FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 0, strHZ, strCode);
                    }
                    else
                        playSound(SOUND_RECODE);
                }
                else if (!fwb->bIsTableAddPhraseByClip && fwb->iTableNewPhraseHZCount < table->WubiDict->iHZLastInputCount && fwb->iTableNewPhraseHZCount < PHRASE_MAX_LENGTH)
                {
                    fwb->iTableNewPhraseHZCount++;
                    int i;
                    char strHZ[PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1];
                    memset(strHZ, 0, PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1);
                    char strCode[5];
                    memset(strCode, 0, 5);
                    for (i = table->WubiDict->iHZLastInputCount - fwb->iTableNewPhraseHZCount; i >= 0 && i < table->WubiDict->iHZLastInputCount; i++)
                    {
                        strcat(strHZ, table->WubiDict->hzLastInput[i % PHRASE_MAX_LENGTH].strHZ);
                    }
                    if (TableCalPhraseCode(table->WubiDict, strHZ, strCode))
                        FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 0, strHZ, strCode);
                }
                else
                    playSound(SOUND_RECODE);
                return IRV_DO_NOTHING;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_RIGHT))
            {
                if (fwb->iTableNewPhraseHZCount > 2)
                {
                    fwb->iTableNewPhraseHZCount--;
                    int i;
                    char strHZ[PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1];
                    memset(strHZ, 0, PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1);
                    char strCode[5];
                    memset(strCode, 0, 5);
                    if (fwb->bIsTableAddPhraseByClip)
                    {
                        char *str = FreeWubiServiceGetClipboard(FcitxDBusGetConnection(fwb->owner));
                        int strLen = fcitx_utf8_strlen(str);
                        for (i = fwb->iTableNewPhraseHZCount; i >= 0 && i < strLen; i++)
                        {
                            str = str + fcitx_utf8_char_len(str);
                        }
                        strcpy(strHZ, str);
                    }
                    else
                    {
                        for (i = table->WubiDict->iHZLastInputCount - fwb->iTableNewPhraseHZCount; i >= 0 && i < table->WubiDict->iHZLastInputCount; i++)
                        {
                            strcat(strHZ, table->WubiDict->hzLastInput[i % PHRASE_MAX_LENGTH].strHZ);
                        }
                    }
                    if (TableCalPhraseCode(table->WubiDict, strHZ, strCode))
                        FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 0, strHZ, strCode);
                }
                else
                    playSound(SOUND_RECODE);
                return IRV_DO_NOTHING;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_ENTER))
            {
                fwb->bIsTableAddPhrase = false;
                fwb->bIsTableAddPhraseByClip = false;
                FcitxInputStateSetIsDoInputOnly(input, false);
                int i;
                char strHZ[PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1];
                memset(strHZ, 0, PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1);
                char strCode[5];
                memset(strCode, 0, 5);
                if (fwb->bIsTableAddPhraseByClip)
                {
                    char *str = FreeWubiServiceGetClipboard(FcitxDBusGetConnection(fwb->owner));
                    int strLen = fcitx_utf8_strlen(str);
                    for (i = fwb->iTableNewPhraseHZCount; i >= 0 && i < strLen; i++)
                    {
                        str = str + fcitx_utf8_char_len(str);
                    }
                    strcpy(strHZ, str);
                }
                else
                {
                    for (i = table->WubiDict->iHZLastInputCount - fwb->iTableNewPhraseHZCount; i >= 0 && i < table->WubiDict->iHZLastInputCount; i++)
                    {
                        strcat(strHZ, table->WubiDict->hzLastInput[i % PHRASE_MAX_LENGTH].strHZ);
                    }
                }
                if (TableCalPhraseCode(table->WubiDict, strHZ, strCode))
                {
                    if( TableFindPhraseByCodeNumAndStr(table->WubiDict,strCode,strHZ))
                    {//exists
                        printf("[%s] [%s] exists",strCode,strHZ);
                    }
                    else
                        FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 1, strHZ, strCode);
                }
                return IRV_CLEAN;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_ESCAPE))
            {
                fwb->bIsTableAddPhrase = false;
                fwb->bIsTableAddPhraseByClip = false;
                FcitxInputStateSetIsDoInputOnly(input, false);
                FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 2, "", "");
                return IRV_CLEAN;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FreewbCTRL_ENTER))
            {
                fwb->bIsTableAddPhrase = false;
                fwb->bIsTableAddPhraseByClip = false;
                FcitxInputStateSetIsDoInputOnly(input, false);
                int i;
                char strHZ[PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1];
                memset(strHZ, 0, PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1);
                char strCode[5];
                memset(strCode, 0, 5);
                if (fwb->bIsTableAddPhraseByClip)
                {
                    char *str = FreeWubiServiceGetClipboard(FcitxDBusGetConnection(fwb->owner));
                    int strLen = fcitx_utf8_strlen(str);
                    for (i = fwb->iTableNewPhraseHZCount; i >= 0 && i < strLen; i++)
                    {
                        str = str + fcitx_utf8_char_len(str);
                    }
                    strcpy(strHZ, str);
                }
                else
                {
                    for (i = table->WubiDict->iHZLastInputCount - fwb->iTableNewPhraseHZCount; i >= 0 && i < table->WubiDict->iHZLastInputCount; i++)
                    {
                        strcat(strHZ, table->WubiDict->hzLastInput[i % PHRASE_MAX_LENGTH].strHZ);
                    }
                }
                if (TableCalPhraseCode(table->WubiDict, strHZ, strCode))
                    FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 3, strHZ, strCode);
                return IRV_CLEAN;
            }
            else
            {
                return IRV_DO_NOTHING;
            }
        } // add word
        else if (fwb->bIsTableDelPhrase)
        {
            if (FcitxHotkeyIsHotKey(sym, state, FCITX_ENTER))
            {
                fwb->bIsTableDelPhrase = false;
                FcitxInputStateSetIsDoInputOnly(input, false);
                if (fwb->pLastCommitRecord)
                    FreeWubiServiceDeleteUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 1, fwb->pLastCommitRecord->strHZ, fwb->pLastCommitRecord->strCode);
                return IRV_CLEAN;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_ESCAPE))
            {
                fwb->bIsTableDelPhrase = false;
                FcitxInputStateSetIsDoInputOnly(input, false);
                FreeWubiServiceDeleteUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 2, "", "");
                return IRV_CLEAN;
            }
            else
            {
                return IRV_DO_NOTHING;
            }
        } // delete word

        // add word
        if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkTableAddPhrase) && !fwb->bIsTempEnglish) || (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_ADD_PHRASE)))
        {
            fwb->bIsTempEnglish = false;
            if (!fwb->bIsTableAddPhrase && table->tableType != FREE_PINYIN)
            {
                if (table->WubiDict->iHZLastInputCount <= 1 || !table->WubiDict->bRule) // 词组最少为两个汉字
                    return IRV_CLEAN;
                fwb->bIsTableAddPhrase = true;
                fwb->bIsTableAddPhraseByClip = false;
                fwb->iTableNewPhraseHZCount = 2;

                FcitxInputStateSetIsDoInputOnly(input, true);
                FcitxInputStateSetShowCursor(input, false);

                FreeWubiInputStateResetRawInputBuffer(input);
                FcitxInputStateSetIsInRemind(input, false);

                FreeWubiInputStateCleanInputWindow(input);

                int i;
                char strHZ[PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1];
                memset(strHZ, 0, PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1);
                char strCode[5];
                memset(strCode, 0, 5);
                for (i = table->WubiDict->iHZLastInputCount - fwb->iTableNewPhraseHZCount; i >= 0 && i < table->WubiDict->iHZLastInputCount; i++)
                {
                    strcat(strHZ, table->WubiDict->hzLastInput[i % PHRASE_MAX_LENGTH].strHZ);
                }

                // puts(strHZ);
                // puts(strCode);
                if (TableCalPhraseCode(table->WubiDict, strHZ, strCode))
                {
                    FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 0, strHZ, strCode);
                }
                else
                {
                    FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 3, strHZ, strCode);
                }
                retVal = IRV_DO_NOTHING;
            }
            else
            {
                retVal = IRV_TO_PROCESS;
            }
            return retVal;
        }

        if ((FcitxHotkeyIsHotKey(sym, state, fwb->hkTableAddPhraseByClip) && !fwb->bIsTempEnglish))
        {
            if (!fwb->bIsTableAddPhrase)
            {
                char *str = FreeWubiServiceGetClipboard(FcitxDBusGetConnection(fwb->owner));
                fwb->iTableNewPhraseHZCount = fcitx_utf8_strlen(str);
                if (fwb->iTableNewPhraseHZCount <= 1 || fwb->iTableNewPhraseHZCount >= PHRASE_MAX_LENGTH || !table->WubiDict->bRule) // 词组最少为两个汉字
                    return IRV_CLEAN;
                fwb->bIsTableAddPhrase = true;
                fwb->bIsTableAddPhraseByClip = true;
                FcitxInputStateSetIsDoInputOnly(input, true);
                FcitxInputStateSetShowCursor(input, false);

                FreeWubiInputStateResetRawInputBuffer(input);
                FcitxInputStateSetIsInRemind(input, false);

                FreeWubiInputStateCleanInputWindow(input);
                //                 int i;
                //                 char strHZ[PHRASE_MAX_LENGTH*UTF8_MAX_LENGTH+1];
                //                 memset(strHZ,0,PHRASE_MAX_LENGTH*UTF8_MAX_LENGTH+1);
                char strCode[5];
                memset(strCode, 0, 5);

                if (TableCalPhraseCode(table->WubiDict, str, strCode))
                    FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 0, str, strCode);
                retVal = IRV_DO_NOTHING;
            }
            else
                retVal = IRV_TO_PROCESS;
            return retVal;
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkTableDelPhrase) && !fwb->bIsTempEnglish) || (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_DELETE_PHRASE)))
        {
            fwb->bIsTempEnglish = false;
            if (!fwb->bIsTableDelPhrase && table->tableType != FREE_PINYIN)
            {
                if (table->WubiDict->iHZLastInputCount < 1) // 词组最少为一个汉字
                    return IRV_CLEAN;
                fwb->bIsTableDelPhrase = true;

                FcitxInputStateSetIsDoInputOnly(input, true);
                FcitxInputStateSetShowCursor(input, false);

                FreeWubiInputStateResetRawInputBuffer(input);
                FcitxInputStateSetIsInRemind(input, false);

                FreeWubiInputStateCleanInputWindow(input);
                if (fwb->pLastCommitRecord)
                    FreeWubiServiceDeleteUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 0, fwb->pLastCommitRecord->strHZ, fwb->pLastCommitRecord->strCode);
                retVal = IRV_DO_NOTHING;
            }
            else
                retVal = IRV_TO_PROCESS;

            return retVal;
        } // 以下为快捷命令

        // 添加对全半角、中英文标点切换的支持
        else if (FcitxHotkeyIsHotKey(sym, state, FCITX_SHIFT_SPACE))
        {
            FreeWubiServiceSetCharWidth(FcitxDBusGetConnection(fwb->owner), 1, 0);
            retVal = IRV_DO_NOTHING;
        }
        else if (sym == '.' && state == FcitxKeyState_Ctrl)
        {
            FreeWubiServiceSetCharWidth(FcitxDBusGetConnection(fwb->owner), 0, 1);
            retVal = IRV_DO_NOTHING;
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkReverseCheck) && !fwb->bIsTempEnglish) || (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_REVERSE_CHECK)))
        {
            fwb->bIsTempEnglish = false;
            if (fwb->pLastCommitRecord)
            {
                FreeWubiServiceDictQuery(FcitxDBusGetConnection(fwb->owner), fwb->pLastCommitRecord->strHZ);
            }
            return IRV_CLEAN;
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->hkReverseCheckByClip) && !fwb->bIsTempEnglish))
        {
            char *str = FreeWubiServiceGetClipboard(FcitxDBusGetConnection(fwb->owner));
            FreeWubiServiceDictQuery(FcitxDBusGetConnection(fwb->owner), str);
            return IRV_CLEAN;
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSetup) && !fwb->bIsTempEnglish))
        {
            fwb->bIsTempEnglish = false;
            FreeWubiServiceOpenSysConf(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSwitchChttrans) && !fwb->bIsTempEnglish))
        {
            fwb->config.bIsTraditional = !fwb->config.bIsTraditional;
            FreeWubiServiceSwitchChttrans(FcitxDBusGetConnection(fwb->owner), fwb->config.bIsTraditional);
            return IRV_CLEAN;
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkShowHideCandiWin) && !fwb->bIsTempEnglish) || (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_HIDE_CANDWIN)))
        {
            FreeWubiServiceSwitchCandiwinState(FcitxDBusGetConnection(fwb->owner));
            fwb->bIsTempEnglish = false;
            return IRV_CLEAN;
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkShowHideToolbar) && !fwb->bIsTempEnglish) || (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_HIDE_TOOLBAR)))
        {
            FreeWubiServiceSwitchToolbarState(FcitxDBusGetConnection(fwb->owner));
            fwb->bIsTempEnglish = false;
            return IRV_CLEAN;
        }
        else if (FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSwitchSkin))
        {
            FreeWubiServiceSwitchSkin(FcitxDBusGetConnection(fwb->owner));
            fwb->bIsTempEnglish = false;
            return IRV_CLEAN;
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSwitchVKb) && !fwb->bIsTempEnglish) || (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_SWITCH_KEYBORD)))
        {
            FreeWubiServiceSwitchVk(FcitxDBusGetConnection(fwb->owner), 0);
            fwb->bIsTempEnglish = false;
            return IRV_CLEAN;
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSwitchCharSet) && !fwb->bIsTempEnglish) || (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_SWITCH_CHAR_SET)))
        {
            FreeWubiServiceSwitchCharSet(FcitxDBusGetConnection(fwb->owner));
            fwb->bIsTempEnglish = false;
            fwb->config.bIsGBK = !fwb->config.bIsGBK;
            return IRV_CLEAN;
        } // 以上为快捷命令
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSmartPunc) && !fwb->bIsTempEnglish))
        {
            fwb->config.bUseSmartPunc = !fwb->config.bUseSmartPunc;
            FreeWubiServiceSwitchSmartPunc(FcitxDBusGetConnection(fwb->owner), fwb->config.bUseSmartPunc);
            return IRV_CLEAN;
        }
        else if (FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE))
        {
            if (!fwb->bIsTableAddPhrase && !fwb->bIsTableDelPhrase &&
                !fwb->bIsTableAdjustOrder)
            {
                if (!FcitxInputStateGetRawInputBufferSize(input))
                {
                    if (table->tableType != FREE_PINYIN && table->WubiDict->iHZLastInputCount > 0)
                        table->WubiDict->iHZLastInputCount--;
                    fwb->bHasQuickDelete = true;
                }
                retVal = IRV_TO_PROCESS;
            }
            if (fwb->config.bInputVoice)
                playSound(SOUND_BACK);
        }
        else if ((FcitxHotkeyIsHotKey(sym, state, fwb->config.hkQuickDelete) && !FcitxInputStateGetRawInputBufferSize(input)))
        {
            if (fwb->bHasQuickDelete)
                return IRV_DO_NOTHING;
            const char *strlast = FcitxInputStateGetLastCommitString(input);
            int a = fcitx_utf8_strlen(strlast);
            while (a--)
            {
                FcitxInstanceForwardKey(instance, FcitxInstanceGetCurrentIC(instance), FCITX_PRESS_KEY, FcitxKey_BackSpace, FcitxKeyState_None);
                if (table->tableType != FREE_PINYIN)
                    table->WubiDict->iHZLastInputCount--;
            }
            fwb->bHasQuickDelete = true;
            fwb->bIsTempEnglish = false;
            return IRV_DO_NOTHING;
        }
        else if (FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSwitchTable) && !fwb->bIsTempEnglish)
        {
            FreeWubiServiceSwitchTable(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if (FcitxHotkeyIsHotKey(sym, state, FreewbCAPS_LOCK) && !fwb->bIsTempEnglish)
        {
            FreeWubiResetInputState(FreeWubiGetInputState());
            // FreeWubiServiceSwitchCapState(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if (state == FcitxKeyState_Ctrl && sym >= '0' && sym <= '9')
        {
            int index = sym - '1';
            if (sym == '0')
            {
                index = 9;
            }
            if (index < 0 || index > FcitxCandidateWordGetCurrentWindowSize(candList) - 1)
                return IRV_DO_NOTHING;
            else
            {
                TABLECANDWORD *tableCandWord = FcitxCandidateWordGetByIndex(candList, index)->priv;
                if (tableCandWord->flag == CT_NORMAL)
                {
                    RECORD *recTemp = tableCandWord->candWord.record;
                    if (fwb->table->tableType == FREE_PINYIN)
                        adjustOrder(FcitxDBusGetConnection(fwb->owner), table, table->PinyinDict, recTemp->strHZ, recTemp->strCode);
                    else
                        adjustOrder(FcitxDBusGetConnection(fwb->owner), table, table->WubiDict, recTemp->strHZ, strCodeInput);
                }
                else if (tableCandWord->flag == CT_AUTOPHRASE)
                {
                    adjustOrder(FcitxDBusGetConnection(fwb->owner), table, table->WubiDict, tableCandWord->candWord.autoPhrase->strHZ, strCodeInput);
                }

                return FreeWubiGetCandWords(fwb);
            } // adjust order
        }
        else if (state == FcitxKeyState_Ctrl && digitalNumberTrans(sym) <= 10 && !fwb->bIsTempEnglish)
        {
            int index = digitalNumberTrans(sym) - 1;
            if (index < 0 || index > FcitxCandidateWordGetCurrentWindowSize(candList) - 1)
                return IRV_DO_NOTHING;
            else
            {
                TABLECANDWORD *tableCandWord = FcitxCandidateWordGetByIndex(candList, index)->priv;
                if (tableCandWord->flag == CT_NORMAL)
                {
                    RECORD *recTemp = tableCandWord->candWord.record;
                    if (recTemp->type == RECORDTYPE_NORMAL)
                        recTemp->type = RECORDTYPE_UNCOMMON;
                    else if (recTemp->type == RECORDTYPE_UNCOMMON)
                        recTemp->type = RECORDTYPE_NORMAL;
                    recTemp->owner->iTableChanged++;
                    SaveTableDict(table);
                }
                return FreeWubiGetCandWords(fwb);
            }
        }
        else if (state == FcitxKeyState_None && sym == FcitxKey_Delete && !fwb->bIsTempEnglish && FcitxCandidateWordPageCount(candList))
        {
            fwb->bisDelNumber = true;
            return IRV_DO_NOTHING;
        } // 候选状态按del
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_SWITCH_UNCOMMON))
        {
            fwb->bIsTempEnglish = false;
            if (fwb->pLastCommitRecord)
            {
                RECORD *recTemp = TableFindPhraseByCode(fwb->pLastCommitRecord->owner, fwb->pLastCommitRecord->strCode, fwb->pLastCommitRecord->strHZ);
                if (recTemp)
                {
                    if (recTemp->type == RECORDTYPE_NORMAL)
                    {
                        recTemp->type = RECORDTYPE_UNCOMMON;
                        FreeWubiServiceSwitchUncommon(FcitxDBusGetConnection(fwb->owner), recTemp->strHZ, 1);
                    }
                    else if (recTemp->type == RECORDTYPE_UNCOMMON)
                    {
                        recTemp->type = RECORDTYPE_NORMAL;
                        FreeWubiServiceSwitchUncommon(FcitxDBusGetConnection(fwb->owner), recTemp->strHZ, 0);
                    }
                    recTemp->owner->iTableChanged++;
                    SaveTableDict(table);
                }
            }
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_RECODE_PROOF))
        {
            fwb->bIsTempEnglish = false;
            fwb->config.bRecodeProof = !fwb->config.bRecodeProof;
            FreeWubiServiceSwitchRecodeProof(FcitxDBusGetConnection(fwb->owner), fwb->config.bRecodeProof);
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_CHTTRANS))
        {
            fwb->bIsTempEnglish = false;
            fwb->config.bIsTraditional = !fwb->config.bIsTraditional;
            FreeWubiServiceSwitchChttrans(FcitxDBusGetConnection(fwb->owner), fwb->config.bIsTraditional);
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_OPEN_CONFIG))
        {
            fwb->bIsTempEnglish = false;
            FreeWubiServiceOpenSysConf(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_VERSION))
        {
            fwb->bIsTempEnglish = false;
            FreeWubiServiceShowVersion(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_PROFEESINAL))
        {
            fwb->bIsTempEnglish = false;
            FreeWubiServiceOpenProfessionalConf(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_QUICK_TABLE))
        {
            fwb->bIsTempEnglish = false;
            FreeWubiServiceModQuickTable(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_USER_TABLE))
        {
            fwb->bIsTempEnglish = false;
            FreeWubiServiceModUserTable(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_WUBI_TABLE))
        {
            fwb->bIsTempEnglish = false;
            FreeWubiServiceModWubiTable(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_PINYIN_TABLE))
        {
            fwb->bIsTempEnglish = false;
            FreeWubiServiceModPinyinTable(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        else if (fwb->bIsTempEnglish && !strcmp(strCodeInput + 1, STR_CONF_DIR))
        {
            fwb->bIsTempEnglish = false;
            FreeWubiServiceOpenConfDir(FcitxDBusGetConnection(fwb->owner));
            return IRV_CLEAN;
        }
        if (!fwb->bIsTableDelPhrase && !fwb->bIsTableAdjustOrder && !fwb->bIsTempEnglish)
        {
            if (FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE))
            {
                if (fwb->config.bInputVoice)
                    playSound(SOUND_BACK);
                if (!FcitxInputStateGetRawInputBufferSize(input))
                {
                    FcitxInputStateSetIsInRemind(input, false);

                    FreeWubiPanelProxyCloseInputWindow();

                    return IRV_DONOT_PROCESS_CLEAN;
                }
                FcitxInputStateSetRawInputBufferSize(input, FcitxInputStateGetRawInputBufferSize(input) - 1);
                strCodeInput[FcitxInputStateGetRawInputBufferSize(input)] = '\0';
                if (FcitxInputStateGetRawInputBufferSize(input)) {
                    FreeWubiGetCandWords(arg);
                    FreeWubiPanelProxyShowInputWindow();

                    retVal = IRV_DO_NOTHING;
                }
                else {
                    FreeWubiResetInputState(FreeWubiGetInputState());

                    FreeWubiPanelProxyCloseInputWindow();

                    return IRV_DO_NOTHING;
                }
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_SPACE) && (FcitxInputStateGetRawInputBufferSize(input) || FcitxCandidateWordPageCount(candList) != 0))
            {
                if (fwb->config.bInputVoice)
                    playSound(SOUND_SAPCE);
                if (FcitxCandidateWordPageCount(candList) == 0)
                {
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), FcitxInputStateGetRawInputBuffer(input));
                    FcitxInputStateSetRawInputBufferSize(input, 0);
                    FcitxInputStateGetRawInputBuffer(input)[0] = '\0';
                    FcitxInputStateSetIsInRemind(input, false);
                    FreeWubiInputStateCleanInputWindow(input);
                    FreeWubiPanelProxyCloseInputWindow();
                    return IRV_DO_NOTHING;
                }
                else
                {
                    if (IRV_COMMIT_STRING == FcitxCandidateWordChooseByIndex(candList, 0)) {
                        FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                        FreeWubiResetInputState(FreeWubiGetInputState());
                        FreeWubiPanelProxyCloseInputWindow();

                        return IRV_DO_NOTHING;
                    }

                    return IRV_DO_NOTHING;
                }
            }
            else if (FcitxHotkeyIsHotKey(sym, state, fwb->config.hkSecondRecode))
            { // 分别选第二候选项
                if (FcitxCandidateWordGetByIndex(candList, 1))
                {
                    FcitxCandidateWordChooseByIndex(candList, 1);
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                    return IRV_CLEAN;
                }
                else
                    retVal = IRV_TO_PROCESS;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, fwb->config.hkThirdRecode))
            { // 单引号选第三候选项
                if (FcitxCandidateWordGetByIndex(candList, 2))
                {
                    FcitxCandidateWordChooseByIndex(candList, 2);
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                    return IRV_CLEAN;
                }
                else
                    retVal = IRV_TO_PROCESS;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, fwb->config.hkAlternativeNextPage))
            {
                if (FcitxCandidateWordHasNext(candList)) {
                    FcitxCandidateWordGoNextPage(candList);
                    FreeWubiPanelProxyShowInputWindow();

                    return IRV_DO_NOTHING;
                }

                //+2024-2-7
                if (FcitxCandidateWordGetCurrentWindowSize(candList) > 0) //|| FcitxInputStateGetRawInputBuffer(input))
                {                                                         //+2024-2-18
                    playSound(SOUND_EMPTY);
                    return IRV_DO_NOTHING;
                } //
                return IRV_TO_PROCESS;
                //
            }
            //+2024-2-7
            else if (FcitxHotkeyIsHotKey(sym, state, fwb->config.hkAlternativePrevPage))
            {
                if (FcitxCandidateWordHasPrev(candList)) {
                    FcitxCandidateWordGoPrevPage(candList);
                    FreeWubiPanelProxyShowInputWindow();
                    return IRV_DO_NOTHING;
                }

                if (FcitxCandidateWordGetCurrentWindowSize(candList) > 0) //|| FcitxInputStateGetRawInputBuffer(input))
                {                                                         //+2024-2-18
                    playSound(SOUND_EMPTY);
                    return IRV_DO_NOTHING;
                } //
                return IRV_TO_PROCESS;
            } //
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_ENTER) && FcitxInputStateGetRawInputBufferSize(input))
            {
                if (!fwb->config.bEnterClear)
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), strCodeInput);
                fwb->bIsTempEnglish = false;

                FreeWubiResetInputState(input);

                FreeWubiPanelProxyCloseInputWindow();

                return IRV_DO_NOTHING;
            }
            else
            { // 标点、数字等
                //puts("99999");
                //printf("sym=%d\n",sym);
                if (state == FcitxKeyState_None && sym >= '0' && sym <= '9')
                {
                    int num = FcitxCandidateWordGetCurrentWindowSize(candList);
                    if (num > 0)
                    {
                        int index = sym - '1';
                        if (sym == '0')
                        {
                            index = 9;
                        }
                        if (index < 0 || index > num - 1)
                        {
                            playSound(SOUND_EMPTY);
                        }
                        else
                        {
                            if (IRV_COMMIT_STRING == FcitxCandidateWordChooseByIndex(candList, index)) {
                                FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                                FreeWubiResetInputState(FreeWubiGetInputState());
                                FreeWubiPanelProxyCloseInputWindow();

                                return IRV_DO_NOTHING;
                            }
                        }
                    }
                }
                retVal = IRV_TO_PROCESS;
            }
            char *matchStr = NULL;
            if (profile->bUseWidePunc && FcitxHotkeyIsHotKeySimple(sym, state))
                matchStr = matchVkBoard(sym, VKM_INPUT_USER_MARK);
            if (matchStr && *matchStr && (!fwb->config.bAutoHalf || (strcmp(matchStr, "，") && strcmp(matchStr, "。"))))
            {
                if (FcitxCandidateWordPageCount(candList))
                {
                    FcitxCandidateWord *candWord = FcitxCandidateWordGetCurrentWindow(candList);
                    if (candWord->owner == fwb)
                    {
                        INPUT_RETURN_VALUE ret = FreeWubiGetCandWord(fwb, candWord);
                        if (ret & IRV_FLAG_PENDING_COMMIT_STRING)
                        {
                            strcat(output_str, matchStr);
                            FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
                            strCodeInput[0] = sym;
                            strCodeInput[1] = '\0';
                            FcitxInputStateSetRawInputBufferSize(input, 1);
                            FreeWubiResetInputState(FreeWubiGetInputState());
                            FreeWubiPanelProxyCloseInputWindow();
                            return IRV_DO_NOTHING;
                        }
                    }
                }
                //puts(matchStr);
                if(strcmp(matchStr,"—")==0||strcmp(matchStr,"…")==0)
                {
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), matchStr);
                }
                //else
                    FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), matchStr);
                FreeWubiResetInputState(FreeWubiGetInputState());
                return IRV_DO_NOTHING;
            }
        }
    }
    if (!FcitxInputStateGetRawInputBufferSize(input) && !FcitxInputStateGetIsInRemind(input) && !fwb->bIsTempEnglish)
    {
        if (FcitxHotkeyIsHotKey(sym, state, FCITX_SPACE) && fwb->config.bFullSpace)
        {
            FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), "　");
            return IRV_DO_NOTHING;
        }
        return IRV_TO_PROCESS;
    }
    if (!FcitxInputStateGetIsInRemind(input))
    {
        if (fwb->bIsTableDelPhrase || fwb->bIsTableAdjustOrder)
            FcitxInputStateSetIsDoInputOnly(input, true);
    }

    if (fwb->bIsTableDelPhrase || fwb->bIsTableAdjustOrder)
        FcitxInputStateSetShowCursor(input, false);
    else if (state == FcitxKey_None && sym == FcitxKey_comma && FcitxCandidateWordGetCurrentWindowSize(candList) > 1 && fwb->config.hkAlternativePrevPage[0].sym != FcitxKey_comma 
            && fwb->config.hkAlternativeNextPage[0].sym != FcitxKey_comma)
    {
        FcitxCandidateWordChooseByIndex(candList, 0);
        FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), output_str);
        FreeWubiResetInputState(FreeWubiGetInputState());
        FreeWubiPanelProxyCloseInputWindow();
    }
    else if (!FcitxInputStateGetIsInRemind(input) && sym != FcitxKey_comma)
    {
        FcitxInputStateSetShowCursor(input, true);
        FcitxInputStateSetCursorPos(input, strlen(FcitxInputStateGetRawInputBuffer(input)));
        FcitxInputStateSetClientCursorPos(input, strlen(FcitxInputStateGetRawInputBuffer(input)));

        FreeWubiPanelProxyShowInputWindow();
    }

    return retVal;
}

boolean TableCheckNoMatch(TableMetaData *table, const char *code)
{
    //     FcitxInstance *instance = table->owner->owner;
    //     FcitxInputState *input = FreeWubiGetInputState();
    //     FcitxCandidateWordList* candList = FreeWubiInputStateGetCandidateList(input);
    return (/*FcitxCandidateWordGetListSize(candList) == 0&&*/ TableFindFirstMatchCode(table, code, false, false) == -1);
}
void TableAddRemindCandWord(RECORD *record, TABLECANDWORD *tableCandWord)
{
    tableCandWord->flag = CT_REMIND;
    tableCandWord->candWord.record = record;
}
/*
 * 获取联想候选字列表
 */
#if 1
INPUT_RETURN_VALUE TableGetRemindCandWords(Fcitxfreewubi *fwb)
{
    TableMetaData* table = fwb->table;
    int iLength;
    RECORD *tableRemind = NULL;
    FcitxInputState *input = FreeWubiGetInputState();
    FcitxCandidateWordList *cand_list = FreeWubiInputStateGetCandidateList(input);
    if (!fwb->strTableRemindSource[0])
        return IRV_TO_PROCESS;

    FcitxInputStateGetRawInputBuffer(input)[0] = '\0';
    FcitxInputStateSetRawInputBufferSize(input, 0);
    FcitxCandidateWordReset(cand_list);

    iLength = fcitx_utf8_strlen(fwb->strTableRemindSource);
    tableRemind = table->WubiDict->recordHead->next;

    while (tableRemind != table->WubiDict->recordHead)
    {

        if (((iLength + 1) <= fcitx_utf8_strlen(tableRemind->strHZ)))
        {
            if (!fcitx_utf8_strncmp(tableRemind->strHZ,
                                    fwb->strTableRemindSource, iLength) &&
                fcitx_utf8_get_nth_char(tableRemind->strHZ, iLength))
            {
                TABLECANDWORD *tableCandWord = fcitx_utils_new(TABLECANDWORD);
                TableAddRemindCandWord(tableRemind, tableCandWord);
                FcitxCandidateWord candWord;
                candWord.callback = FreeWubiGetCandWord;
                candWord.owner = fwb;
                candWord.priv = tableCandWord;
                candWord.strExtra = NULL;
                candWord.strWord = strdup(tableCandWord->candWord.record->strHZ);
                candWord.wordType = MSG_OTHER;
                FcitxCandidateWordAppend(cand_list, &candWord);
            }
        }
        tableRemind = tableRemind->next;
    }

    FreeWubiInputStateCleanInputWindowUp(input);
    FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetPreedit(input),
                                         MSG_INPUT, _("#词组联想:"));
    FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetPreedit(input),
                                         MSG_INPUT, fwb->strTableRemindSource);
    FcitxInputStateSetShowCursor(input, true);
    FcitxInputStateSetCursorPos(input, strlen("#词组联想:") + strlen(fwb->strTableRemindSource));
    int count = FcitxCandidateWordPageCount(cand_list);
    FcitxInputStateSetIsInRemind(input, count);
    if (count)
    {
        return IRV_DISPLAY_CANDWORDS;
    }
    else
    {
        return IRV_CLEAN;
    }
}
#endif
/*
 * 第二个参数表示是否进入联想模式，实现自动上屏功能时，不能使用联想模式
 */
INPUT_RETURN_VALUE _TableGetCandWord(Fcitxfreewubi *fwb, TABLECANDWORD *tableCandWord, boolean _bRemind)
{
    char *pCandWord = NULL;
    TableMetaData* table = fwb->table;
    FcitxInstance *instance = fwb->owner;
    FcitxInputState *input = FreeWubiGetInputState();
    FcitxInputStateSetIsInRemind(input, false);

    switch (tableCandWord->flag)
    {
    case CT_NORMAL:
        pCandWord = tableCandWord->candWord.record->strHZ;
        if (!fwb->pLastCommitRecord)
            fwb->pLastCommitRecord = fcitx_utils_new(RECORD);
        fwb->pLastCommitRecord->owner = tableCandWord->candWord.record->owner;
        fwb->pLastCommitRecord->strCode = tableCandWord->candWord.record->strCode;
        fwb->pLastCommitRecord->strHZ = tableCandWord->candWord.record->strHZ;
        fwb->pLastCommitRecord->type = tableCandWord->candWord.record->type;
        break;
    case CT_REMIND:
        strcpy(fwb->strTableRemindSource, tableCandWord->candWord.record->strHZ + strlen(fwb->strTableRemindSource));
        strcpy(FcitxInputStateGetOutputString(input), fwb->strTableRemindSource);
        INPUT_RETURN_VALUE retVal = TableGetRemindCandWords(fwb);
        if (retVal == IRV_DISPLAY_CANDWORDS) {
            return IRV_COMMIT_STRING_REMIND;
        }
        else
            return IRV_COMMIT_STRING;
        if (fwb->pLastCommitRecord)
        {
            free(fwb->pLastCommitRecord);
            fwb->pLastCommitRecord = NULL;
        }
        break;
    case CT_REPEATE:
        pCandWord = fwb->pLastCommitRecord->strHZ;
        if (!fwb->pLastCommitRecord)
            fwb->pLastCommitRecord = fcitx_utils_new(RECORD);
        fwb->pLastCommitRecord->owner = tableCandWord->candWord.record->owner;
        fwb->pLastCommitRecord->strCode = tableCandWord->candWord.record->strCode;
        fwb->pLastCommitRecord->strHZ = tableCandWord->candWord.record->strHZ;
        fwb->pLastCommitRecord->type = tableCandWord->candWord.record->type;
        if (!fwb->pLastCommitRecord->strCode[0])
        {
            FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), fwb->pLastCommitRecord->strHZ);
            FreeWubiResetInputState(FreeWubiGetInputState());
            FcitxInstanceForwardKey(instance, FcitxInstanceGetCurrentIC(instance), FCITX_PRESS_KEY, FcitxKey_Left, FcitxKeyState_None);
            return IRV_CLEAN;
        }
        break;
    case CT_AUTOPHRASE:
        pCandWord = tableCandWord->candWord.autoPhrase->strHZ;
        tableCandWord->candWord.autoPhrase->iSelected = true;
        if (fwb->config.iAutoPhraseOpt == 2)
        {
            FreeWubiServiceAddUsrParse(FcitxDBusGetConnection(fwb->owner), fwb->table, 1, tableCandWord->candWord.autoPhrase->strHZ, tableCandWord->candWord.autoPhrase->strCode);
        }
        if (!fwb->pLastCommitRecord)
            fwb->pLastCommitRecord = fcitx_utils_new(RECORD);
        fwb->pLastCommitRecord->owner = table->WubiDict;
        fwb->pLastCommitRecord->strCode = tableCandWord->candWord.autoPhrase->strCode;
        fwb->pLastCommitRecord->strHZ = tableCandWord->candWord.autoPhrase->strHZ;
        break;
    case CT_QUICK:
        pCandWord = tableCandWord->candWord.qucikPhrase->value;
        if (!fwb->pLastCommitRecord)
            fwb->pLastCommitRecord = fcitx_utils_new(RECORD);
        fwb->pLastCommitRecord->owner = table->WubiDict;
        fwb->pLastCommitRecord->strCode = malloc(sizeof(char) * 2);
        fwb->pLastCommitRecord->strCode[0] = tableCandWord->candWord.qucikPhrase->key;
        fwb->pLastCommitRecord->strCode[1] = '\0';
        fwb->pLastCommitRecord->strHZ = tableCandWord->candWord.qucikPhrase->value;
        if (!fwb->pLastCommitRecord->strCode[0])
        {
            FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), fwb->pLastCommitRecord->strHZ);
            FreeWubiResetInputState(FreeWubiGetInputState());
            fwb->bNeedMoveCur = true;
            // FcitxInstanceForwardKey(instance, FcitxInstanceGetCurrentIC(instance), FCITX_PRESS_KEY, FcitxKey_Left, FcitxKeyState_None);
            return IRV_CLEAN;
        }
        break;
    }
    if (fwb->config.bPhraseRemind)
    {
        strcpy(fwb->strTableRemindSource, pCandWord);
        strcpy(FcitxInputStateGetOutputString(input), pCandWord);
        INPUT_RETURN_VALUE retVal = TableGetRemindCandWords(fwb);
        if (retVal == IRV_DISPLAY_CANDWORDS)
            FreeWubiPanelProxyShowInputWindow();

            return IRV_DO_NOTHING;
    }
    else
    {
        FreeWubiInputStateCleanInputWindow(input);
    }
    boolean needFree = false;
    if (tableCandWord->candWord.record && tableCandWord->candWord.record->type == RECORDTYPE_CONSTRUCT)
    {
        pCandWord = matchTime(pCandWord);
        needFree = true;
    }
    if (fwb->config.bIsTraditional && tableCandWord->candWord.record && tableCandWord->candWord.record->type != RECORDTYPE_S2T)
    {
        char *strTemp = s2tConvers(table, pCandWord);
        if (needFree)
            free(pCandWord);
        pCandWord = strTemp;
        needFree = true;
    }
    strcpy(FcitxInputStateGetOutputString(input), pCandWord);
    if (needFree)
        free(pCandWord);
    return IRV_COMMIT_STRING;
}
void TableAddCandWord(RECORD *record, TABLECANDWORD *tableCandWord)
{
    tableCandWord->flag = CT_NORMAL;
    tableCandWord->candWord.record = record;
}
void TableAddAutoCandWord(AUTOPHRASE *autoPhrase, TABLECANDWORD *tableCandWord)
{
    tableCandWord->flag = CT_AUTOPHRASE;
    tableCandWord->candWord.autoPhrase = autoPhrase;
}

static INPUT_RETURN_VALUE FreeWubiDoReleaseIntput(void *arg, FcitxKeySym sym, unsigned int state)
{
    //         printf("key:%c\n",sym);
    //     printf("state:%d\n",state);
    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
    FcitxInstance *instance = fwb->owner;
    //     FcitxInputState *input = FreeWubiGetInputState();
    //     int puncNumber = -1;
    //     if(state == FcitxKeyState_None)
    //         puncNumber = isPuncKey(sym);
    //     if(!fwb->bIsTempEnglish&&-1!=puncNumber&&fwb->config.bUseSmartPunc){
    //         FcitxInstanceForwardKey(instance, FcitxInstanceGetCurrentIC(instance), FCITX_PRESS_KEY, FcitxKey_Left, FcitxKeyState_None);
    //         return IRV_TO_PROCESS;
    //     }
    if (fwb->bNeedMoveCur)
        FcitxInstanceForwardKey(instance, FcitxInstanceGetCurrentIC(instance), FCITX_PRESS_KEY, FcitxKey_Left, FcitxKeyState_None);
    fwb->bNeedMoveCur = false;
    return IRV_TO_PROCESS;
}

static void FreeWubiOnClose(void *arg, FcitxIMCloseEventType event_type)
{
#ifdef DEBUG
    FcitxLog(INFO, _("FreeWubiOnClose"));
#endif
    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
    FcitxInstance *instance = fwb->owner;
    FcitxInputState *input = FreeWubiGetInputState();
    if (event_type == CET_SwitchIM)
    {
        FreeWubiServiceSwitchImState(FcitxDBusGetConnection(fwb->owner), IM_CLOSE_FREEWB);
        //		printf("freewb close\n");
    }
    else if (event_type == CET_ChangeByInactivate)
    {
        FreeWubiServiceSwitchImState(FcitxDBusGetConnection(fwb->owner), IM_TO_ENGLISH);
        //		printf("CET_ChangeByInactivate close\n");
        char *strCodeInput = FcitxInputStateGetRawInputBuffer(input);
        FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), strCodeInput);
        FreeWubiResetInputState(FreeWubiGetInputState());
    }
}

int TableFindPhraseByCodeNum(const TableDict *tableDict, const char *strCode, boolean gbk)
{
    RECORD *recTemp;
    int i = 0, findNum = 0;

    while (tableDict->recordIndex[i].record && strCode[0] != tableDict->recordIndex[i].cCode)
    {
        i++;
        if (i >= strlen(tableDict->strInputCode))
            return findNum;
    }
    recTemp = tableDict->recordIndex[i].record;
    while (recTemp != tableDict->recordHead)
    {
        if (recTemp->strCode[0] != strCode[0])
            break;

        if (strcmp(recTemp->strCode, strCode) == 0)
        {
            if (!gbk && recTemp->type == RECORDTYPE_UNCOMMON)
                break;
            ++findNum;
            //@note 空重码提醒2
        }
        recTemp = recTemp->next;
    }
    return findNum;
}

void UpdateHZLastInput(TableMetaData *table, const char *str)
{
    unsigned int i;
    unsigned int str_len = fcitx_utf8_strlen(str);
    TableDict *const tableDict = table->WubiDict;
    if (!tableDict)
        return;
    SINGLE_HZ *const hzLastInput = tableDict->hzLastInput;

    for (i = 0; i < str_len; i++)
    {
        unsigned int char_len = fcitx_utf8_char_len(str);
        strncpy(hzLastInput[tableDict->iHZLastInputCount % PHRASE_MAX_LENGTH].strHZ,
                str, char_len);
        hzLastInput[tableDict->iHZLastInputCount % PHRASE_MAX_LENGTH].strHZ[char_len] = '\0';
        str += char_len;
        tableDict->iHZLastInputCount++;
    }
}
static INPUT_RETURN_VALUE FreeWubiGetCandWord(void *arg, FcitxCandidateWord *candWord)
{
    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
    FcitxInstance *instance = fwb->owner;
    TableMetaData *table = fwb->table;
    FcitxInputState *input = FreeWubiGetInputState();
    TABLECANDWORD *tableCandWord = candWord->priv;
    INPUT_RETURN_VALUE retVal = _TableGetCandWord(fwb, tableCandWord, true);
    if (retVal & IRV_FLAG_PENDING_COMMIT_STRING)
    {
        fwb->bHasQuickDelete = false;
        if (fcitx_utf8_strlen(FcitxInputStateGetOutputString(input)) >= 1)
            UpdateHZLastInput(table, FcitxInputStateGetOutputString(input));
        if (fcitx_utf8_strlen(FcitxInputStateGetOutputString(input)) == 1 && fwb->config.iAutoPhraseOpt)
        {
            strcpy(table->autoRecord->recordHZ[table->autoRecord->recordIndex % 4].strHZ, FcitxInputStateGetOutputString(input));
            if (TableCreateAutoPhrase(fwb, table->autoRecord->recordIndex))
            {
                FcitxInputStateGetRawInputBuffer(input)[0] = '\0';
                FcitxInputStateSetRawInputBufferSize(input, 0);
                FcitxInputStateSetIsInRemind(input, 1);
                // FcitxCandidateWordReset(cand_list);
                retVal = IRV_COMMIT_STRING_REMIND;
            }
            table->autoRecord->recordIndex++;
        }
        else
        {
            table->autoRecord->recordIndex = 0;
        }
        if (fwb->pLastCommitRecord && fwb->config.bAutoAdjustOrder)
            adjustOrder(FcitxDBusGetConnection(fwb->owner), fwb->table, fwb->pLastCommitRecord->owner, fwb->pLastCommitRecord->strHZ, fwb->pLastCommitRecord->strCode);
    }
    return retVal;
}
static INPUT_RETURN_VALUE FreeWubiGetNoneCandWord(void *arg, FcitxCandidateWord *candWord)
{
    return IRV_CLEAN;
}
INPUT_RETURN_VALUE TableGetRepeateCandWords(Fcitxfreewubi *fwb)
{
    FcitxInstance *instance = fwb->owner;
    FcitxInputState *input = FreeWubiGetInputState();

    FcitxInstanceCleanInputWindowUp(instance);
    FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetPreedit(input), MSG_INPUT, FcitxInputStateGetRawInputBuffer(input));
    FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetClientPreedit(input), MSG_INPUT | MSG_DONOT_COMMIT_WHEN_UNFOCUS, FcitxInputStateGetRawInputBuffer(input));
    FcitxInputStateSetCursorPos(input, FcitxInputStateGetRawInputBufferSize(input));
    FcitxInputStateSetClientCursorPos(input, 0);

    TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
    tableCandWord->flag = CT_REPEATE;
    tableCandWord->candWord.record = fwb->pLastCommitRecord;
    FcitxCandidateWord candWord;
    candWord.callback = FreeWubiGetCandWord;
    candWord.owner = fwb;
    candWord.priv = tableCandWord;
    candWord.strExtra = NULL;
    char *speStrHZ = tableCandWord->candWord.record->strHZ;
    if (tableCandWord->candWord.record->type == RECORDTYPE_CONSTRUCT)
    {
        candWord.strWord = matchTime(speStrHZ);
    }
    else
        candWord.strWord = strdup(tableCandWord->candWord.record->strHZ);
    candWord.wordType = MSG_OTHER;
    FcitxCandidateWordAppend(FreeWubiInputStateGetCandidateList(input), &candWord);

    return IRV_DISPLAY_CANDWORDS;
}
INPUT_RETURN_VALUE TableGetQuickCandWords(Fcitxfreewubi *fwb)
{
    TableMetaData* table = fwb->table;
    FcitxInstance *instance = fwb->owner;
    FcitxInputState *input = FreeWubiGetInputState();
    FcitxCandidateWordList *candList = FreeWubiInputStateGetCandidateList(input);
    int retVal = IRV_DISPLAY_CANDWORDS;
    char output_str[10] = { '\0' };
    int puncNumber = -1;
    FcitxInstanceCleanInputWindowUp(instance);
    FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetPreedit(input), MSG_INPUT, FcitxInputStateGetRawInputBuffer(input));
    FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetClientPreedit(input), MSG_INPUT | MSG_DONOT_COMMIT_WHEN_UNFOCUS, FcitxInputStateGetRawInputBuffer(input));
    FcitxInputStateSetCursorPos(input, FcitxInputStateGetRawInputBufferSize(input));
    FcitxInputStateSetClientCursorPos(input, 0);
    FcitxInputStateSetShowCursor(input, true);
    QUCIK_TABLE *quickTable[3] = {NULL, NULL, NULL};
    if (FcitxInputStateGetRawInputBufferSize(input) == 1)
    {
        puncNumber = isPuncKey(FcitxInputStateGetRawInputBuffer(input)[0]);
        FcitxProfile *profile = FcitxInstanceGetProfile(instance);
        if (-1 != puncNumber && fwb->config.bUseSmartPunc)
        {
            matchPunc(output_str, puncNumber, profile->bUseWidePunc);
        }
        else
            strcpy(output_str, FcitxInputStateGetRawInputBuffer(input));
        switchTempKey(output_str, profile->bUseWidePunc);
        quickTable[0] = fcitx_utils_new(QUCIK_TABLE);
        if (-1 != puncNumber)
            quickTable[0]->key = 0;
        else
            quickTable[0]->key = FcitxInputStateGetRawInputBuffer(input)[0];
        strcpy(quickTable[0]->value, output_str);
        quickTable[0]->next = NULL;
    }
    else if (FcitxInputStateGetRawInputBufferSize(input) > 1)
    {
        if (!fwb->bIsTempEnglish)
            quickTable[0] = findQuickPharse(table, FcitxInputStateGetRawInputBuffer(input)[1]);
        else
        {
            char *strOut1 = matchMoney(FcitxInputStateGetRawInputBuffer(input) + 1);
            if (!strOut1)
                strOut1 = matchtimeManul(FcitxInputStateGetRawInputBuffer(input) + 1, 0);
            if (strOut1)
            {
                quickTable[0] = fcitx_utils_new(QUCIK_TABLE);
                quickTable[0]->key = FcitxInputStateGetRawInputBuffer(input)[0];
                strcpy(quickTable[0]->value, strOut1);
                quickTable[0]->next = NULL;
                free(strOut1);
            }
            char *strOut2 = matchNumber(FcitxInputStateGetRawInputBuffer(input) + 1);
            if (!strOut2)
                strOut2 = matchtimeManul(FcitxInputStateGetRawInputBuffer(input) + 1, 1);
            if (strOut2)
            {
                quickTable[1] = fcitx_utils_new(QUCIK_TABLE);
                quickTable[1]->key = FcitxInputStateGetRawInputBuffer(input)[0];
                strcpy(quickTable[1]->value, strOut2);
                quickTable[1]->next = NULL;
                free(strOut2);
            }
            char *strOut3 = matchNumberSim(FcitxInputStateGetRawInputBuffer(input) + 1);
            if (!strOut3)
                strOut3 = matchtimeManul(FcitxInputStateGetRawInputBuffer(input) + 1, 2);
            if (strOut3)
            {
                quickTable[2] = fcitx_utils_new(QUCIK_TABLE);
                quickTable[2]->key = FcitxInputStateGetRawInputBuffer(input)[0];
                strcpy(quickTable[2]->value, strOut3);
                quickTable[2]->next = NULL;
                free(strOut3);
            }
        }
    }
    int i = 0;
    for (; i < 3; i++)
    {
        if (quickTable[i])
        {
            TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
            tableCandWord->flag = CT_QUICK;
            tableCandWord->candWord.qucikPhrase = quickTable[i];
            FcitxCandidateWord candWord;
            candWord.callback = FreeWubiGetCandWord;
            candWord.owner = fwb;
            candWord.priv = tableCandWord;
            candWord.strExtra = NULL;
            candWord.strWord = strdup(tableCandWord->candWord.qucikPhrase->value);
            candWord.wordType = MSG_OTHER;
            FcitxCandidateWordAppend(FreeWubiInputStateGetCandidateList(input), &candWord);
        }
    }

    if (FcitxInputStateGetRawInputBufferSize(input) != 1 && !fwb->bIsTempEnglish)
    {
        retVal = FcitxCandidateWordChooseByIndex(candList, 0);
        if (retVal != IRV_COMMIT_STRING)
            retVal = IRV_CLEAN;
    }

    return retVal;
}
INPUT_RETURN_VALUE FreeWubiGetCandWords(void *arg)
{
    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
    TableMetaData *table = fwb->table;
    RECORD *recTemp;
    FcitxInstance *instance = fwb->owner;
    FcitxInputState *input = FreeWubiGetInputState();
    FcitxCandidateWordList *candList = FreeWubiInputStateGetCandidateList(input);
    FreeWubiInputStateCleanInputWindow(input);

    if (FcitxInputStateGetRawInputBuffer(input)[0] == '\0')
        return IRV_TO_PROCESS;

    if (FcitxInputStateGetIsInRemind(input))
        return TableGetRemindCandWords(fwb);

    if (FcitxInputStateGetRawInputBuffer(input)[0] == 'z' && FcitxInputStateGetRawInputBufferSize(input) == 1 && fwb->pLastCommitRecord)
    {
        return TableGetRepeateCandWords(fwb);
    }
    if (FcitxInputStateGetRawInputBuffer(input)[0] == fwb->config.QuickInputKey[0].sym || fwb->bIsTempEnglish || (FcitxInputStateGetRawInputBuffer(input)[0] == fwb->config.unCommonKey[0].sym && FcitxInputStateGetRawInputBufferSize(input) == 1))
        return TableGetQuickCandWords(fwb);

    char *code = FcitxInputStateGetRawInputBuffer(input);
    if (TableFindFirstMatchCode(table, code, !fwb->config.bCodeRemind, true) == -1)
    {
        if (FcitxInputStateGetRawInputBufferSize(input))
        {
            FcitxMessagesSetMessageCount(FcitxInputStateGetPreedit(input), 0);
            FcitxMessagesSetMessageCount(FcitxInputStateGetClientPreedit(input), 0);
            FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetPreedit(input), MSG_INPUT, FcitxInputStateGetRawInputBuffer(input));
            FcitxMessageType type = MSG_INPUT;
            FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetClientPreedit(input), type, FcitxInputStateGetRawInputBuffer(input));
            FcitxInputStateSetCursorPos(input, strlen(FcitxInputStateGetRawInputBuffer(input)));
            FcitxInputStateSetClientCursorPos(input, 0);
        }

        return IRV_DISPLAY_CANDWORDS;
    }

    UT_array candTemp, candWubi, candPinyin;
    utarray_init(&candTemp, fcitx_ptr_icd);
    utarray_init(&candWubi, fcitx_ptr_icd);
    utarray_init(&candPinyin, fcitx_ptr_icd);
    if (table->tableType != FREE_PINYIN)
    { // 五笔
        // printf("exseek=%d\n", fwb->config.bCodeRemind);
        while (table->WubiDict->currentRecord && table->WubiDict->currentRecord != table->WubiDict->recordHead)
        {
            if (FcitxInputStateGetRawInputBuffer(input)[0] == fwb->config.unCommonKey[0].sym)
            { // 临时拼音生僻字
                char *strInput = FcitxInputStateGetRawInputBuffer(input);
                strInput++;
                if (table->WubiDict->currentRecord->type == RECORDTYPE_UNCOMMON &&
                    !TableCompareCode(strInput, table->WubiDict->currentRecord->strCode, !fwb->config.bCodeRemind))
                {
                    if (fwb->config.bIsTraditional)
                    { // 简入繁出
                        char strSignelHZ[10] = {0};
                        int len = fcitx_utf8_strlen(table->WubiDict->currentRecord->strHZ);
                        if (len == 1)
                            strcpy(strSignelHZ, table->WubiDict->currentRecord->strHZ);
                        else
                            strncpy(strSignelHZ, table->WubiDict->currentRecord->strHZ, fcitx_utf8_char_len(table->WubiDict->currentRecord->strHZ));
                        recTemp = table->WubiDict->s2tSignleHZ[CalHZIndex(strSignelHZ)];
                        if (!recTemp)
                        {
                            TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                            TableAddCandWord(table->WubiDict->currentRecord, tableCandWord);
                            utarray_push_back(&candWubi, &tableCandWord);
                        }
                        else
                        {
                            while (recTemp != table->WubiDict->s2tRecordHead && !strncmp(recTemp->strCode, strSignelHZ, strlen(strSignelHZ)) && strcmp(recTemp->strCode, table->WubiDict->currentRecord->strHZ))
                            {
                                recTemp = recTemp->next;
                            }
                            if (recTemp == table->WubiDict->s2tRecordHead || strcmp(recTemp->strCode, table->WubiDict->currentRecord->strHZ))
                            {
                                TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                                TableAddCandWord(table->WubiDict->currentRecord, tableCandWord);
                                utarray_push_back(&candWubi, &tableCandWord);
                            }
                            else
                            {
                                while (recTemp != table->WubiDict->s2tRecordHead && !strcmp(recTemp->strCode, table->WubiDict->currentRecord->strHZ))
                                {
                                    TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                                    TableAddCandWord(recTemp, tableCandWord);
                                    tableCandWord->candWord.simpelRecord = table->WubiDict->currentRecord;
                                    utarray_push_back(&candWubi, &tableCandWord);
                                    recTemp = recTemp->next;
                                }
                            }
                        }
                    }
                    else
                    { // 正常输出
                        TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                        TableAddCandWord(table->WubiDict->currentRecord, tableCandWord);
                        utarray_push_back(&candWubi, &tableCandWord);
                    }
                }
            }
            else if ((fwb->config.bIsGBK || table->WubiDict->currentRecord->type != RECORDTYPE_UNCOMMON) &&
                     !TableCompareCode(FcitxInputStateGetRawInputBuffer(input), table->WubiDict->currentRecord->strCode, !fwb->config.bCodeRemind))
            {
                if (fwb->config.bIsTraditional)
                { // 简入繁出
                    char strSignelHZ[10] = {0};
                    int len = fcitx_utf8_strlen(table->WubiDict->currentRecord->strHZ);
                    if (len == 1)
                        strcpy(strSignelHZ, table->WubiDict->currentRecord->strHZ);
                    else
                        strncpy(strSignelHZ, table->WubiDict->currentRecord->strHZ, fcitx_utf8_char_len(table->WubiDict->currentRecord->strHZ));
                    recTemp = table->WubiDict->s2tSignleHZ[CalHZIndex(strSignelHZ)];
                    if (!recTemp)
                    {
                        TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                        TableAddCandWord(table->WubiDict->currentRecord, tableCandWord);
                        utarray_push_back(&candWubi, &tableCandWord);
                    }
                    else
                    {
                        while (recTemp != table->WubiDict->s2tRecordHead && !strncmp(recTemp->strCode, strSignelHZ, strlen(strSignelHZ)) && strcmp(recTemp->strCode, table->WubiDict->currentRecord->strHZ))
                        {
                            recTemp = recTemp->next;
                        }
                        if (recTemp == table->WubiDict->s2tRecordHead || strcmp(recTemp->strCode, table->WubiDict->currentRecord->strHZ))
                        {
                            //                                 printf("111%s%s\n",recTemp->strCode,recTemp->strHZ);
                            TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                            TableAddCandWord(table->WubiDict->currentRecord, tableCandWord);
                            utarray_push_back(&candWubi, &tableCandWord);
                        }
                        else
                        {
                            //                                 printf("222%s%s\n",recTemp->strCode,recTemp->strHZ);
                            while (recTemp != table->WubiDict->s2tRecordHead && !strcmp(recTemp->strCode, table->WubiDict->currentRecord->strHZ))
                            {
                                TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                                TableAddCandWord(recTemp, tableCandWord);
                                tableCandWord->candWord.simpelRecord = table->WubiDict->currentRecord;
                                utarray_push_back(&candWubi, &tableCandWord);
                                recTemp = recTemp->next;
                            }
                        }
                        //                             printf("%s%s\n",recTemp->strCode,recTemp->strHZ);
                    }
                }
                else
                {
                    TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                    TableAddCandWord(table->WubiDict->currentRecord, tableCandWord);
                    utarray_push_back(&candWubi, &tableCandWord);
                }
            }
            table->WubiDict->currentRecord = table->WubiDict->currentRecord->next;
        }
    } // 五笔
    /*        TABLECANDWORD** pcand = NULL;
        for (pcand = (TABLECANDWORD**) utarray_front(&candWubi);
                pcand != NULL;
                pcand = (TABLECANDWORD**) utarray_next(&candWubi, pcand)) {
                    TABLECANDWORD* tableCandWord = *pcand;
                    if(tableCandWord->candWord.record->type == RECORDTYPE_S2T)
                        printf("cccccc:%s,%s,%d\n",tableCandWord->candWord.simpelRecord->strCode,tableCandWord->candWord.simpelRecord->strHZ,FcitxInputStateGetRawInputBufferSize(input));
                }  */
    if (table->tableType != FREE_WUBI || FcitxInputStateGetRawInputBuffer(input)[0] == fwb->config.unCommonKey[0].sym)
    {
        while (table->PinyinDict->currentRecord && table->PinyinDict->currentRecord != table->PinyinDict->recordHead)
        {
            char *strInput = FcitxInputStateGetRawInputBuffer(input);
            if (FcitxInputStateGetRawInputBuffer(input)[0] == fwb->config.unCommonKey[0].sym && table->tableType == FREE_WUBI)
                strInput++;
            if (table->PinyinDict->currentRecord->type != RECORDTYPE_CONSTRUCT &&
                !TableCompareCode(strInput, table->PinyinDict->currentRecord->strCode, !fwb->config.bCodeRemind))
            {
                if (fwb->config.bIsTraditional)
                {
                    char strSignelHZ[10] = {0};
                    int len = fcitx_utf8_strlen(table->PinyinDict->currentRecord->strHZ);
                    if (len == 1)
                        strcpy(strSignelHZ, table->PinyinDict->currentRecord->strHZ);
                    else
                        strncpy(strSignelHZ, table->PinyinDict->currentRecord->strHZ, fcitx_utf8_char_len(table->PinyinDict->currentRecord->strHZ));
                    recTemp = table->WubiDict->s2tSignleHZ[CalHZIndex(strSignelHZ)];
                    if (!recTemp)
                    {
                        TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                        TableAddCandWord(table->PinyinDict->currentRecord, tableCandWord);
                        utarray_push_back(&candPinyin, &tableCandWord);
                    }
                    else
                    {
                        while (recTemp != table->WubiDict->s2tRecordHead && !strncmp(recTemp->strCode, strSignelHZ, strlen(strSignelHZ)) && strcmp(recTemp->strCode, table->PinyinDict->currentRecord->strHZ))
                        {
                            recTemp = recTemp->next;
                        }
                        if (recTemp == table->WubiDict->s2tRecordHead || strcmp(recTemp->strCode, table->PinyinDict->currentRecord->strHZ))
                        {
                            TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                            TableAddCandWord(table->PinyinDict->currentRecord, tableCandWord);
                            utarray_push_back(&candPinyin, &tableCandWord);
                        }
                        else
                        {
                            while (recTemp != table->WubiDict->s2tRecordHead && !strcmp(recTemp->strCode, table->PinyinDict->currentRecord->strHZ))
                            {
                                TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                                TableAddCandWord(recTemp, tableCandWord);
                                tableCandWord->candWord.simpelRecord = table->PinyinDict->currentRecord;
                                utarray_push_back(&candPinyin, &tableCandWord);
                                recTemp = recTemp->next;
                            }
                        }
                    }
                }
                else
                {
                    TABLECANDWORD *tableCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
                    TableAddCandWord(table->PinyinDict->currentRecord, tableCandWord);
                    utarray_push_back(&candPinyin, &tableCandWord);
                }
            }
            table->PinyinDict->currentRecord = table->PinyinDict->currentRecord->next;
        }
    }

    TABLECANDWORD *autoCandWord = NULL;
    if (fwb->config.iAutoPhraseOpt && table->currentPhrase)
    {
        if (!TableCompareCode(FcitxInputStateGetRawInputBuffer(input), table->currentPhrase->strCode, 1))
        {
            autoCandWord = fcitx_utils_malloc0(sizeof(TABLECANDWORD));
            TableAddAutoCandWord(table->currentPhrase, autoCandWord);
            utarray_push_back(&candWubi, &autoCandWord);
        }
    }

    sortCandwords(&candWubi, &candPinyin, &candTemp);
    boolean isRepeat = false;
    TABLECANDWORD **pcand = NULL;
    for (pcand = (TABLECANDWORD **)utarray_front(&candTemp);
         pcand != NULL;
         pcand = (TABLECANDWORD **)utarray_next(&candTemp, pcand))
    {
        TABLECANDWORD *tableCandWord = *pcand;
        FcitxCandidateWord candWord;
        candWord.callback = FreeWubiGetCandWord;
        candWord.owner = fwb;
        candWord.priv = tableCandWord;
        if (tableCandWord->flag == CT_AUTOPHRASE)
        {
            if (isRepeat)
                continue;
            candWord.strWord = strdup(tableCandWord->candWord.autoPhrase->strHZ);
        }
        else if (tableCandWord->flag == CT_NORMAL)
        {
            char *speStrHZ = tableCandWord->candWord.record->strHZ;
            if (tableCandWord->candWord.record->type == RECORDTYPE_CONSTRUCT)
            {
                if (autoCandWord && strcmp(tableCandWord->candWord.record->strCode, autoCandWord->candWord.autoPhrase->strCode) == 0 &&
                    strcmp(tableCandWord->candWord.record->strHZ, autoCandWord->candWord.autoPhrase->strHZ) == 0)
                    isRepeat = true; // 如果与自动造词的词组重复，则不显示自动造词的词组
                candWord.strWord = matchTime(speStrHZ);
            }
            else
            {
                candWord.strWord = strdup(tableCandWord->candWord.record->strHZ);
            }
        }
        if (fwb->config.bIsTraditional && tableCandWord->candWord.record->type != RECORDTYPE_S2T)
        {
            // free(candWord.strWord);
            char *strTemp = s2tConvers(table, candWord.strWord);
            free(candWord.strWord);
            candWord.strWord = strTemp;
        }

        candWord.strExtra = NULL;
        candWord.wordType = MSG_OTHER;
        const char *pstr = NULL;
        const char *pstrS2t = NULL;
        boolean isSinglePinyin = false;
        boolean isSingleS2t = false;
        if (tableCandWord->flag == CT_NORMAL)
        {
            if (tableCandWord->candWord.record->type == RECORDTYPE_S2T && fcitx_utf8_strlen(tableCandWord->candWord.record->strHZ) == 1)
            {
                pstrS2t = tableCandWord->candWord.record->strCode;
                isSingleS2t = true;
            }
            if ((tableCandWord->candWord.record->owner->tableType == FREE_PINYIN) || (tableCandWord->candWord.record->type == RECORDTYPE_S2T && tableCandWord->candWord.simpelRecord->owner->tableType == FREE_PINYIN))
            {
                if (fcitx_utf8_strlen(tableCandWord->candWord.record->strHZ) == 1)
                {
                    if (!strcmp(FcitxInputStateGetRawInputBuffer(input), tableCandWord->candWord.record->strCode) || (tableCandWord->candWord.record->type == RECORDTYPE_S2T && !strcmp(FcitxInputStateGetRawInputBuffer(input), tableCandWord->candWord.simpelRecord->strCode)))
                    {
                        recTemp = table->WubiDict->tableSingleHZ[CalHZIndex(tableCandWord->candWord.record->strHZ)];
                        if (!recTemp)
                            pstr = (char *)NULL;
                        else
                        {
                            pstr = recTemp->strCode;
                            isSinglePinyin = true;
                        }
                    }
                    else if ((!strcmp(FcitxInputStateGetRawInputBuffer(input) + 1, tableCandWord->candWord.record->strCode) && FcitxInputStateGetRawInputBuffer(input)[0] == fwb->config.unCommonKey[0].sym) || (tableCandWord->candWord.record->type == RECORDTYPE_S2T && !strcmp(FcitxInputStateGetRawInputBuffer(input) + 1, tableCandWord->candWord.simpelRecord->strCode) && FcitxInputStateGetRawInputBuffer(input)[0] == fwb->config.unCommonKey[0].sym))
                    {
                        recTemp = table->WubiDict->tableSingleHZ[CalHZIndex(tableCandWord->candWord.record->strHZ)];
                        if (!recTemp)
                            pstr = (char *)NULL;
                        else
                        {
                            pstr = recTemp->strCode;
                            isSinglePinyin = true;
                        }
                    }
                }
                else if (table->tableType == FREE_WUBI)
                {
                    if (tableCandWord->candWord.record->type == RECORDTYPE_S2T)
                        pstr = tableCandWord->candWord.simpelRecord->strCode + FcitxInputStateGetRawInputBufferSize(input) - 1;
                    else
                        pstr = tableCandWord->candWord.record->strCode + FcitxInputStateGetRawInputBufferSize(input) - 1;
                }
                else
                {
                    if (tableCandWord->candWord.record->type == RECORDTYPE_S2T)
                        pstr = tableCandWord->candWord.simpelRecord->strCode + FcitxInputStateGetRawInputBufferSize(input);
                    else
                        pstr = tableCandWord->candWord.record->strCode + FcitxInputStateGetRawInputBufferSize(input);
                }
            }
            else if (tableCandWord->candWord.record->type == RECORDTYPE_UNCOMMON && FcitxInputStateGetRawInputBuffer(input)[0] == fwb->config.unCommonKey[0].sym)
            {
                if (tableCandWord->candWord.record->type == RECORDTYPE_S2T)
                    pstr = tableCandWord->candWord.simpelRecord->strCode + FcitxInputStateGetRawInputBufferSize(input) - 1;
                else
                    pstr = tableCandWord->candWord.record->strCode + FcitxInputStateGetRawInputBufferSize(input) - 1;
                //                 printf("22:%s\n",pstr);
            }
            else
            {
                if (tableCandWord->candWord.record->type == RECORDTYPE_S2T)
                    pstr = tableCandWord->candWord.simpelRecord->strCode + FcitxInputStateGetRawInputBufferSize(input);
                else
                    pstr = tableCandWord->candWord.record->strCode + FcitxInputStateGetRawInputBufferSize(input);
            }
        }
        else if (tableCandWord->flag == CT_AUTOPHRASE)
            pstr = tableCandWord->candWord.autoPhrase->strCode + FcitxInputStateGetRawInputBufferSize(input);
        if (pstr)
        {
            if (pstrS2t)
                candWord.strExtra = fcitx_utils_malloc0(sizeof(char) * (strlen(pstr) + strlen(pstrS2t) + 7));
            else
                candWord.strExtra = fcitx_utils_malloc0(sizeof(char) * (strlen(pstr) + 5));

            *candWord.strExtra = ' ';
            if (isSingleS2t)
            {
                strcat(candWord.strExtra, "(");
                strcat(candWord.strExtra, pstrS2t);
                strcat(candWord.strExtra, ")");
            }
            if (isSinglePinyin)
                strcat(candWord.strExtra, "[");
            strcat(candWord.strExtra, pstr);
            if (isSinglePinyin)
                strcat(candWord.strExtra, "]");
            candWord.extraType = MSG_CODE;
        }
        FcitxCandidateWordAppend(candList, &candWord);
    }
    utarray_clear(&candTemp);
    utarray_clear(&candWubi);
    utarray_clear(&candPinyin);

    utarray_done(&candWubi);
    utarray_done(&candPinyin);
    utarray_done(&candTemp);
    INPUT_RETURN_VALUE retVal = IRV_DISPLAY_CANDWORDS;
    if (FcitxCandidateWordGetCurrentWindowSize(candList) == 1 && strlen(FcitxInputStateGetRawInputBuffer(input)) >= 4)
    { // 如果只有一个候选词，则送到客户程序中
        FcitxCandidateWord *candWord = FcitxCandidateWordGetCurrentWindow(candList);
        if (candWord->owner == fwb)
        {
            TABLECANDWORD *tableCandWord = candWord->priv;
            char *strCodeInput = FcitxInputStateGetRawInputBuffer(input);
            if (tableCandWord->flag == CT_NORMAL)
            {
                if (!strcmp(tableCandWord->candWord.record->strCode, strCodeInput) ||
                    (tableCandWord->candWord.record->type == RECORDTYPE_S2T && !strcmp(tableCandWord->candWord.simpelRecord->strCode, strCodeInput)) ||
                    (strCodeInput[0] == fwb->config.unCommonKey[0].sym && !strcmp(tableCandWord->candWord.record->strCode, strCodeInput + 1)) ||
                    ((strCodeInput[0] == fwb->config.unCommonKey[0].sym && tableCandWord->candWord.record->type == RECORDTYPE_S2T && !strcmp(tableCandWord->candWord.record->strCode, strCodeInput + 1))))
                    retVal = FcitxCandidateWordChooseByIndex(candList, 0);
            }
            else if (tableCandWord->flag == CT_AUTOPHRASE)
            {
                retVal = FcitxCandidateWordChooseByIndex(candList, 0);
            }
        }

        if (retVal & IRV_FLAG_PENDING_COMMIT_STRING) {
            FreeWubiInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), FcitxInputStateGetOutputString(input));
            FreeWubiResetInputState(FreeWubiGetInputState());

            return IRV_CLEAN;
        }
    }

    if (FcitxInputStateGetRawInputBufferSize(input))
    {
        FcitxMessagesSetMessageCount(FcitxInputStateGetPreedit(input), 0);
        FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetPreedit(input), MSG_INPUT, FcitxInputStateGetRawInputBuffer(input));
        FcitxInputStateSetCursorPos(input, strlen(FcitxInputStateGetRawInputBuffer(input)));
        FcitxCandidateWord *candWord = NULL;
        if ((candWord = FcitxCandidateWordGetFirst(candList)))
        {
            FcitxMessagesSetMessageCount(FcitxInputStateGetClientPreedit(input), 0);
            FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetClientPreedit(input), MSG_INPUT, candWord->strWord);
            FcitxInputStateSetClientCursorPos(input, 0);
        }
        else
        {
            FcitxMessagesSetMessageCount(FcitxInputStateGetClientPreedit(input), 0);
            FcitxMessageType type = MSG_INPUT;
            FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetClientPreedit(input), type, FcitxInputStateGetRawInputBuffer(input));
            FcitxInputStateSetClientCursorPos(input, 0);
        }
    }

    return retVal;
}

static boolean FreeWubiPhraseTips(void *arg)
{
    return true;
}

static void FreeWubiSave(void *arg)
{
    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
    TableMetaData *table = fwb->table;
    if (!table)
        return;
    SaveTableDict(table);
}

static INPUT_RETURN_VALUE FreeWubiKeyBlocker(void *arg, FcitxKeySym sym, unsigned int state)
{
    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
    TableMetaData *table = fwb->table;

    FcitxInstance *instance = fwb->owner;
    FcitxInputState *input = FreeWubiGetInputState();

    do
    {
        if (!FcitxHotkeyIsHotKeySimple(sym, state))
            break;
        FcitxCandidateWordList *cand_list;
        cand_list = FreeWubiInputStateGetCandidateList(input);
        if (FcitxCandidateWordPageCount(cand_list))
        {
            FcitxCandidateWord *candWord;
            candWord = FcitxCandidateWordGetCurrentWindow(cand_list);
            if (candWord->owner != fwb)
                break;
            INPUT_RETURN_VALUE ret = FreeWubiGetCandWord(fwb, candWord);
            if (!(ret & IRV_FLAG_PENDING_COMMIT_STRING))
                break;
            FreeWubiInstanceCommitString(
                instance, FcitxInstanceGetCurrentIC(instance),
                FcitxInputStateGetOutputString(input));
        }
        FcitxInputStateSetRawInputBufferSize(input, 0);
        FcitxInputStateGetRawInputBuffer(input)[0] = '\0';
        FcitxInputStateSetIsInRemind(input, false);
        FreeWubiInputStateCleanInputWindow(input);
        FcitxUIUpdateInputWindow(instance);
        return IRV_FLAG_FORWARD_KEY;
    } while (0);
    return FcitxStandardKeyBlocker(input, sym, state);
}

void TableMetaDataFree(TableMetaData *table)
{
    if (!table)
        return;
    FreeTableDict(table);
    freeAutoPhrase(table);
    FcitxConfigFree(&table->config);
    free(table);
}
void sortCandwords(UT_array *arry1, UT_array *arry2, UT_array *result)
{
    TABLECANDWORD **pcand1 = (TABLECANDWORD **)utarray_front(arry1);
    TABLECANDWORD **pcand2 = (TABLECANDWORD **)utarray_front(arry2);

    while (pcand1 && pcand2)
    {
        TABLECANDWORD *tableCandWord1 = *pcand1;
        TABLECANDWORD *tableCandWord2 = *pcand2;
        RECORD *re1;
        RECORD *re2;

        if (NULL == tableCandWord1 || NULL == tableCandWord2 || tableCandWord1->candWord.record == NULL || tableCandWord2->candWord.record == NULL)
            break; //+2024-2-23

        if (tableCandWord1->candWord.record->type == RECORDTYPE_S2T)
            re1 = tableCandWord1->candWord.simpelRecord;
        else
            re1 = tableCandWord1->candWord.record;

        if (tableCandWord2->candWord.record->type == RECORDTYPE_S2T)
            re2 = tableCandWord2->candWord.simpelRecord;
        else
            re2 = tableCandWord2->candWord.record;

        if (strcmp(re1->strCode, re2->strCode) <= 0)
        {
            utarray_push_back(result, &tableCandWord1);
            pcand1 = (TABLECANDWORD **)utarray_next(arry1, pcand1);
        }
        else
        {
            utarray_push_back(result, &tableCandWord2);
            pcand2 = (TABLECANDWORD **)utarray_next(arry2, pcand2);
        }
    }

    while (pcand1)
    {
        TABLECANDWORD *tableCandWord1 = *pcand1;
        utarray_push_back(result, &tableCandWord1);
        pcand1 = (TABLECANDWORD **)utarray_next(arry1, pcand1);
    }
    while (pcand2)
    {
        TABLECANDWORD *tableCandWord2 = *pcand2;
        utarray_push_back(result, &tableCandWord2);
        pcand2 = (TABLECANDWORD **)utarray_next(arry2, pcand2);
    }
}

int TableCreateAutoPhrase(Fcitxfreewubi *fwb, int iCount)
{
    TableMetaData* tableMetaData = fwb->table;
    FcitxInstance *instance = fwb->owner;
    FcitxInputState *input = FreeWubiGetInputState();
    FcitxCandidateWordList *cand_list = FreeWubiInputStateGetCandidateList(input);
    int hasRepeatRecord = 0;
    char repeatIndex[5] = "1.";
    char *strHZ = NULL;
    char strCode[5] = {0};
    AUTOPHRASE *autoPhrase = tableMetaData->autoPhrase->next;
    short i, j;
    i = (iCount - 3) < 0 ? 0 : (iCount - 3);
    while (i < iCount)
    {
        boolean bFindRepeat = false;
        RECORD *repeatRecord = NULL;
        if (strHZ)
        {
            free(strHZ);
            strHZ = NULL;
        }
        strHZ = malloc(sizeof(char) * (UTF8_MAX_LENGTH + 1) * (iCount - i));
        memset(strHZ, 0, sizeof(char) * (UTF8_MAX_LENGTH + 1) * (iCount - i));
        for (j = i; j <= iCount; j++)
        {
            strcat(strHZ, tableMetaData->autoRecord->recordHZ[j % 4].strHZ);
        }
        while (autoPhrase)
        {
            if (strcmp(autoPhrase->strHZ, strHZ) == 0)
            {
                bFindRepeat = true;
                break;
            }
            autoPhrase = autoPhrase->next;
        }
        repeatRecord = TableFindPhrase(tableMetaData->WubiDict, strHZ);
        if (repeatRecord || bFindRepeat)
        {
            if (fwb->config.bRemindExistWords && repeatRecord)
            {
                hasRepeatRecord++;
                repeatIndex[0] = '0' + hasRepeatRecord;
                FcitxInstanceCleanInputWindowUp(instance);
                FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetPreedit(input),
                                                     MSG_INPUT, _("#已有词组:"));
                FcitxInputStateSetShowCursor(input, true);
                FcitxInputStateSetCursorPos(input, strlen("#已有词组:"));

                FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetAuxDown(input),
                                                     MSG_INDEX, repeatIndex);
                FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetAuxDown(input),
                                                     MSG_FIRSTCAND, repeatRecord->strHZ);
                FcitxMessagesAddMessageStringsAtLast(FcitxInputStateGetAuxDown(input),
                                                     MSG_FIRSTCAND, repeatRecord->strCode);
            }
            i++;
            continue;
        }
        if (TableCalPhraseCode(tableMetaData->WubiDict, strHZ, strCode))
        {
            tableMetaData->insertPoint->next = fcitx_utils_new(AUTOPHRASE);
            tableMetaData->insertPoint->next->strCode = malloc(sizeof(char) * (strlen(strCode) + 1));
            strcpy(tableMetaData->insertPoint->next->strCode, strCode);
            tableMetaData->insertPoint->next->strHZ = malloc(sizeof(char) * (strlen(strHZ) + 1));
            strcpy(tableMetaData->insertPoint->next->strHZ, strHZ);
            tableMetaData->insertPoint->next->iSelected = false;
            tableMetaData->insertPoint = tableMetaData->insertPoint->next;
            tableMetaData->insertPoint->next = NULL;
        }
        i++;
    }

    free(strHZ);
    return hasRepeatRecord;
}
void freeAutoPhrase(TableMetaData *tableMetaData)
{
    AUTOPHRASE *autoPhrase = tableMetaData->autoPhrase->next;
    while (autoPhrase)
    {
        free(autoPhrase->strCode);
        free(autoPhrase->strHZ);
        AUTOPHRASE *temp = autoPhrase;
        autoPhrase = autoPhrase->next;
        free(temp);
    }
    free(tableMetaData->autoPhrase);
}
int digitalNumberTrans(FcitxKeySym sym)
{
    char *digitalStr = "!@#$%^&*()";
    int number = 0;
    while (*digitalStr)
    {
        number++;
        if (*digitalStr++ == sym)
            return number;
    }
    return 100;
}
void playSound(SoundType sType)
{
    if (sType > SOUND_NUM)
    {
        return;
    }
    char cmd[128] = {0};
    sprintf(cmd, "aplay %s%s%s > /dev/null 2>&1 &", SOUND_FILE_PATH, "/", soundData[sType]);
    system(cmd);
}

static void FreeWubiReloadConfig(void *arg)
{
    Fcitxfreewubi *fwb = (Fcitxfreewubi *)arg;
#ifdef DEBUG
    FcitxLog(INFO, _("FreewubiReloadConfig"));
#endif
    FcitxIM *cr_im = FcitxInstanceGetCurrentIM(fwb->owner);
    // printf("ccccccccccccccc   current im :%s\n",cr_im->uniqueName );
    reloadFreewb(fwb);
}

static void *FcitxFreeWubiCreate(FcitxInstance *instance)
{
    Fcitxfreewubi *freewubi = fcitx_utils_new(Fcitxfreewubi);
    freewubi->owner = instance;

    FreeWubiInputStateInitializeInstance();

    bindtextdomain("fcitx-freewubi", LOCALEDIR);

    run_freewb_panel();

    FreeWubiPanelProxyInitializeInstance(instance);
    FreeWubiPanelProxyOnTriggerOn();

    InternalInit(freewubi);
    FcitxIMEventHook imhook = {Fcitx4IMOnChanged, freewubi};
    FcitxInstanceRegisterIMChangedHook(freewubi->owner, imhook);

    FcitxIMIFace iface;
    memset(&iface, 0, sizeof(FcitxIMIFace));
    iface.ResetIM = FreeWubiResetStatus;
    iface.DoInput = FreeWubiDoInput;
    iface.GetCandWords = FreeWubiGetCandWords;
    iface.PhraseTips = FreeWubiPhraseTips;
    iface.Save = FreeWubiSave;
    iface.Init = FreeWubiInit;
    iface.ReloadConfig = FreeWubiReloadConfig;
    iface.KeyBlocker = FreeWubiKeyBlocker;
    //     iface.UpdateSurroundingText = FcitxfreewubiUpdateSurroundingText;
    iface.DoReleaseInput = FreeWubiDoReleaseIntput;
    iface.OnClose = FreeWubiOnClose;
    FcitxInstanceRegisterIMv2(
        freewubi->owner,
        freewubi,
        "freewb",
        _("Freewb"),
        _("Freewb"),
        iface,
        10,
        "zh_CN");

    boolean flags = true;
    FcitxInstanceSetContext(freewubi->owner, CONTEXT_DISABLE_QUICKPHRASE, &flags);
    FcitxUIRegisterStatus(freewubi->owner, freewubi->owner, _("属性设置"), _("属性设置"), _("属性设置"), freewb_settings_handler, tray_menu_handler_empty);

    reloadFreewb(freewubi);
    return freewubi;
}

static void freewb_settings_handler(void *arg)
{
    FcitxInstance *instance = (FcitxInstance *)arg;
    DBusConnection *conn = FcitxDBusGetConnection(instance);
    if (conn == NULL)
    {
        return;
    }

    FreeWubiServiceOpenSysConf(conn);
}

static boolean tray_menu_handler_empty(void *arg)
{
    return false;
}

static void Fcitx4IMOnChanged(void *arg)
{
    FcitxLog(INFO, "func : %s line :%d", __FUNCTION__, __LINE__);
    Fcitxfreewubi *freewubi = (Fcitxfreewubi *)arg;
    FcitxInputContext *ic = FcitxInstanceGetCurrentIC(freewubi->owner);
    if (ic == NULL)
    {
        return;
    }

    FcitxIM * im = FcitxInstanceGetCurrentIM(freewubi->owner);
    if (im == NULL)
    {
        return;
    }

    const char *im_name = im->uniqueName;
    if (strncmp(im_name, "freewb", sizeof("freewb")) == 0)
    {
        FcitxLog(INFO,"should activate freewb.");
        FreeWubiServiceSwitchImState(FcitxDBusGetConnection(freewubi->owner), IM_INTO_FREEWB);
        FcitxUISetStatusVisable(freewubi->owner, _("属性设置"), true);
    }
    else
    {
        FcitxLog(INFO,"should deactivate freewb.");
        FreeWubiServiceSwitchImState(FcitxDBusGetConnection(freewubi->owner), IM_CLOSE_FREEWB);
        FcitxUISetStatusVisable(freewubi->owner, _("属性设置"), false);
    }
}

static void FcitxFreeWubiDestroy(void *arg)
{
#ifdef DEBUG
    FcitxLog(INFO, _("FcitxFreeWubiDestroy"));
#endif
    FreeWubiPanelProxyDestroyInstance();
    FreeWubiInputStateDestroyInstance();

    Fcitxfreewubi *freewubi = (Fcitxfreewubi *)arg;
    FreeWubiServiceExitFreewbPanel(FcitxDBusGetConnection(freewubi->owner));
    TableMetaDataFree(freewubi->table);
    free(freewubi);
}

#ifdef __cplusplus
extern "C"
{
#endif
    // CONFIG_DEFINE_LOAD_AND_SAVE(freewubi, FcitxfreewubiConfig, "fcitx-freewubi");

    CONFIG_DESC_DEFINE(GetFreewubiGlobalConfigDesc, "fcitx-freewubi.desc")
    FCITX_DEFINE_PLUGIN(fcitx_freewubi, ime, FcitxIMClass) = {
        FcitxFreeWubiCreate,
        FcitxFreeWubiDestroy};

#ifdef __cplusplus
}
#endif

static boolean LoadFreeWubiGlobalInfo(Fcitxfreewubi *fwb)
{

    // FcitxLog(INFO,_("LoadFreeWubiGlobalInfo"));

    char *path = getFreewbPath();
    char *configPath;
    fcitx_utils_alloc_cat_str(configPath, path, "/config/", "config.ini");
    FILE *fpg = fopen(configPath, "r");

    freeGetOption(fwb);

    free(path);
    free(configPath);

    if (!fpg)
    {
        FcitxLog(ERROR, _("FreeWubi: open config.ini fail"));
        return false;
    }
    FcitxConfigFile *cgfile = FcitxConfigParseConfigFileFp(fpg, GetFreewubiGlobalConfigDesc());
    fclose(fpg);
    if (!cgfile)
    {
        FcitxLog(ERROR, _("FreeWubi: open fcitx-freewubi.desc fail"));
        return false;
    }

    FcitxfreewubiConfigConfigBind(&fwb->config, cgfile, GetFreewubiGlobalConfigDesc());
    freeWbConfigBindSync((FcitxGenericConfig *)&fwb->config);
    FreewbCTRL_ENTER[0].desc = NULL;
    FreewbCTRL_ENTER[0].state = FcitxKeyState_Ctrl;
    FreewbCTRL_ENTER[0].sym = FcitxKey_Return;
    FreewbCAPS_LOCK[0].desc = NULL;
    FreewbCAPS_LOCK[0].state = FcitxKeyState_None;
    FreewbCAPS_LOCK[0].sym = FcitxKey_Caps_Lock;
    fwb->hkReverseCheckByClip[0].desc = NULL;
    fwb->hkReverseCheckByClip[0].state = FcitxKeyState_Ctrl_Alt;
    fwb->hkReverseCheckByClip[0].sym = fwb->config.hkReverseCheck[0].sym;
    fwb->hkTableAddPhraseByClip[0].desc = NULL;
    fwb->hkTableAddPhraseByClip[0].state = FcitxKeyState_Ctrl_Alt;
    fwb->hkTableAddPhraseByClip[0].sym = fwb->config.hkTableAddPhrase[0].sym;

    char *ptr, key[128];
    char inifile[256];
    sprintf(inifile, "%s/.config/fcitx/conf/fcitx-freewubi.config", getenv("HOME"));
    FcitxLog(INFO, "func : %s line : %d ini filename: %s ", __FUNCTION__, __LINE__, inifile);
    INI *ini = fileToIni(inifile);

    strcpy(key, GetIniKeyString(ini, "快捷键", "setupOption", "CTRL_KEYCOMMA"));
    ptr = key + 5;
    fwb->config.hkSetup[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "switchSkin", "KEY_NONE"));
    ptr = key + 5;
    fwb->config.hkSwitchSkin[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "switchInputMode", "CTRL_KEY_BACKSLASH"));
    ptr = key + 5;
    fwb->config.hkSwitchFreeim[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "switchVKb", "CTRL_KEY_ESC"));
    ptr = key + 5;
    fwb->config.hkSwitchVKb[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "switchChttrans", "CTRL_KEY_J"));
    ptr = key + 5;
    fwb->config.hkSwitchChttrans[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "switchCharSet", "CTRL_KEY_M"));
    ptr = key;
    ptr += 5;
    fwb->config.hkSwitchCharSet[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "quickDelScreenItem", "KEY_NONE"));
    ptr = key;
    ptr += 5;
    fwb->config.hkQuickDelete[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "backFindCode", "CTRL_KEY_SLASH"));
    ptr = key;
    ptr += 5;
    fwb->config.hkReverseCheck[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "onlineAddWord", "CTRL_KEY_EQUAL"));
    ptr = key;
    ptr += 5;
    fwb->config.hkTableAddPhrase[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "onlineDelWord", "CTRL_KEY_DASH"));
    ptr = key;
    ptr += 5;
    fwb->config.hkTableDelPhrase[0].sym = FreewbHotkeyGetKeyList(ptr);

    strcpy(key, GetIniKeyString(ini, "快捷键", "tempPinyin", "`"));
    fwb->config.unCommonKey[0].sym = FreewbHotkeyGetKeyList(key);

    strcpy(key, GetIniKeyString(ini, "快捷键", "tempEnglish", ";"));
    fwb->config.tempEnglishKey[0].sym = FreewbHotkeyGetKeyList(key);

    strcpy(key, GetIniKeyString(ini, "快捷键", "shortcutInput", "'"));
    fwb->config.QuickInputKey[0].sym = FreewbHotkeyGetKeyList(key);

    fwb->config.bCodeRemind = GetIniKeyBool(ini, "基本设置", "codeRemind");
    fwb->config.bAutoAdjustOrder = GetIniKeyBool(ini, "基本设置", "autoAdjustFreq");
    fwb->config.bPhraseRemind = GetIniKeyBool(ini, "基本设置", "wordThink");
    fwb->config.bRemindExistWords = GetIniKeyBool(ini, "基本设置", "remindExistWord");
    fwb->config.bRecodeVoice = GetIniKeyBool(ini, "基本设置", "alertWhenEmptyCode");
    fwb->config.bEnterClear = GetIniKeyBool(ini, "基本设置", "enterClear");
    fwb->config.bShiftCommit = GetIniKeyBool(ini, "基本设置", "shiftCommitChar");

    fwb->config.iCandidateWordNumber = GetIniKeyInt(ini, "候选项", "candiWordCount", 3);

    strcpy(key, GetIniKeyString(ini, "候选项", "secondRecodeKey", ";"));
    fwb->config.hkSecondRecode[0].sym = FreewbHotkeyGetKeyList(key);
    fwb->config.hkSecondRecode[0].state = 0;

    strcpy(key, GetIniKeyString(ini, "候选项", "thirdRecodeKey", "'"));
    fwb->config.hkThirdRecode[0].state = 0;
    fwb->config.hkThirdRecode[0].sym = FreewbHotkeyGetKeyList(key);

    strcpy(key, GetIniKeyString(ini, "候选项", "prevPageKey", "-"));
    fwb->config.hkAlternativePrevPage[0].sym = FreewbHotkeyGetKeyList(key);
    fwb->config.hkAlternativePrevPage[0].state = 0;

    strcpy(key, GetIniKeyString(ini, "候选项", "nextPageKey", "="));
    fwb->config.hkAlternativeNextPage[0].sym = FreewbHotkeyGetKeyList(key);
    fwb->config.hkAlternativeNextPage[0].state = 0;

    free(ini);
    ini = NULL;
    return true;
}

static void InternalInit(Fcitxfreewubi *freewubi)
{

    // FcitxLog(INFO,_("InternalInit"));

    LoadFreeWubiGlobalInfo(freewubi);
    if (!freewubi->table)
    {
        freewubi->table = fcitx_utils_new(TableMetaData);
        freewubi->table->autoRecord = fcitx_utils_new(AUTORECORD);
        freewubi->table->autoRecord->recordIndex = 0;
        freewubi->table->autoPhrase = fcitx_utils_new(AUTOPHRASE);
        freewubi->table->insertPoint = freewubi->table->autoPhrase;
        freewubi->table->autoPhrase->next = NULL;
        freewubi->table->quickTable = fcitx_utils_new(QUCIK_TABLE);
        freewubi->table->quickTable->next = NULL;
        freewubi->table->autoEng = fcitx_utils_new(AUTO_ENG);
        freewubi->table->autoEng->next = NULL;

        freewubi->table->freeWubiConfig = &(freewubi->config);
        LoadTableDict(freewubi->table);
        freewubi->table->tableType = FREE_WUBI;
    }
    freewubi->bNeedMoveCur = false;
    freewubi->pLastCommitRecord = NULL;
    freewubi->bNotFirstStart = false;
    //    freewubi->bSwichImSeccess = false;
}

boolean reloadFreewb(Fcitxfreewubi *fwb)
{
    LoadFreeWubiGlobalInfo(fwb);
    if (fwb->config.bBackUpTable)
    {
        if (fwb->table)
            FreeTableDict(fwb->table);
        LoadTableDict(fwb->table);
        FreeWubiServiceResetTableFlag(FcitxDBusGetConnection(fwb->owner));
        fwb->config.bUserWordChanged = 1;
    }
    if (fwb->config.bUserWordChanged)
    {
        LoadUsrDict(fwb->table);
        fwb->config.bUserWordChanged = 0;
        FreeWubiServiceResetUerWordFlag(FcitxDBusGetConnection(fwb->owner));

        printf("reload user dict imtype=%d", fwb->table->tableType);
    }
    if (fwb->config.bQuickTableChanged)
    {
        freeQucikTable(fwb->table);
        LoadQuickTable(fwb->table);
        FreeWubiServiceResetQuickTableFlag(FcitxDBusGetConnection(fwb->owner));
    }
    LoadAutoEng(fwb->table, fwb->config.strAutoEng);

    if (fwb->config.iImType >= 3)
    {
        // fwb->config.iImType = fwb->table->tableType;
        FreeWubiServiceSwitchFreeIm(FcitxDBusGetConnection(fwb->owner), fwb->config.iImType);
    }
    else
    {
        fwb->table->tableType = fwb->config.iImType;
        FreeWubiServiceSwitchFreeIm(FcitxDBusGetConnection(fwb->owner), fwb->table->tableType);
    }

    setVKboard(fwb->config.strUsrKeyBoard, VKM_INPUT_USER_CHAR);
    setVKboard(fwb->config.strPuncKeyBoard, VKM_INPUT_USER_MARK);
}