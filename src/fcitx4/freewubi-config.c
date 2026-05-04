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
#include "freewubi-config.h"

CONFIG_BINDING_BEGIN(FcitxfreewubiConfig)
CONFIG_BINDING_REGISTER("基本设置", "codeRemind", bCodeRemind)
CONFIG_BINDING_REGISTER("基本设置", "autoAdjustFreq", bAutoAdjustOrder)
CONFIG_BINDING_REGISTER("基本设置", "wordThink", bPhraseRemind)
CONFIG_BINDING_REGISTER("基本设置", "remindExistWord", bRemindExistWords)
CONFIG_BINDING_REGISTER("基本设置", "alertWhenEmptyCode", bRecodeVoice)
CONFIG_BINDING_REGISTER("基本设置", "enterClear", bEnterClear)
CONFIG_BINDING_REGISTER("基本设置", "shiftCommitChar", bShiftCommit)

CONFIG_BINDING_REGISTER("快捷键", "tempPinyin", unCommonKey)
CONFIG_BINDING_REGISTER("快捷键", "tempEnglish", tempEnglishKey)
CONFIG_BINDING_REGISTER("快捷键", "shortcutInput", QuickInputKey)

// //+2024-2-5
CONFIG_BINDING_REGISTER("快捷键", "setupOption", hkSetup)
//
CONFIG_BINDING_REGISTER("快捷键", "switchSkin", hkSwitchSkin)
CONFIG_BINDING_REGISTER("快捷键", "switchInputMode", hkSwitchFreeim)

CONFIG_BINDING_REGISTER("快捷键", "switchVKb", hkSwitchVKb)
CONFIG_BINDING_REGISTER("快捷键", "switchChttrans", hkSwitchChttrans)
CONFIG_BINDING_REGISTER("快捷键", "switchCharSet", hkSwitchCharSet)
CONFIG_BINDING_REGISTER("快捷键", "switchLexicon", hkSwitchTable)
CONFIG_BINDING_REGISTER("快捷键", "quickDelScreenItem", hkQuickDelete)
CONFIG_BINDING_REGISTER("快捷键", "backFindCode", hkReverseCheck)

CONFIG_BINDING_REGISTER("快捷键", "onlineAddWord", hkTableAddPhrase)
CONFIG_BINDING_REGISTER("快捷键", "onlineDelWord", hkTableDelPhrase)
CONFIG_BINDING_REGISTER("快捷键", "showHideCandiWin", hkShowHideCandiWin)
CONFIG_BINDING_REGISTER("快捷键", "showHideToolbar", hkShowHideToolbar)
CONFIG_BINDING_REGISTER("快捷键", "markAutoPair", hkSmartPunc)

CONFIG_BINDING_REGISTER("候选项", "candiWordCount", iCandidateWordNumber)
CONFIG_BINDING_REGISTER("候选项", "secondRecodeKey", hkSecondRecode)
CONFIG_BINDING_REGISTER("候选项", "thirdRecodeKey", hkThirdRecode)
CONFIG_BINDING_REGISTER("候选项", "prevPageKey", hkAlternativePrevPage)
CONFIG_BINDING_REGISTER("候选项", "nextPageKey", hkAlternativeNextPage)
CONFIG_BINDING_END()

typedef FcitxConfigSyncResult (*FreewbConfigOptionFunc)(FcitxConfigOption *, FcitxConfigSync);
static void FreewbConfigSyncValue(FcitxGenericConfig *config, FcitxConfigGroup *group, FcitxConfigOption *option,
                                  FcitxConfigSync sync);
static FcitxConfigSyncResult FreewbConfigOptionInteger(FcitxConfigOption *option, FcitxConfigSync sync);
static FcitxConfigSyncResult FreewbConfigOptionBoolean(FcitxConfigOption *option, FcitxConfigSync sync);
static FcitxConfigSyncResult FreewbConfigOptionEnum(FcitxConfigOption *option, FcitxConfigSync sync);
static FcitxConfigSyncResult FreewbConfigOptionString(FcitxConfigOption *option, FcitxConfigSync sync);
static FcitxConfigSyncResult FreewbConfigOptionHotkey(FcitxConfigOption *option, FcitxConfigSync sync);
static FcitxConfigSyncResult FreewbConfigOptionChar(FcitxConfigOption *option, FcitxConfigSync sync);

static void FreewbHotkeySetKey(const char *str, FcitxHotkey *hotkey);
static boolean FreewbHotkeyParseKey(const char *strKey, FcitxKeySym *sym, unsigned int *state);
//    sstatic int FreewbHotkeyGetKeyList(const char *strKey);

#define FreewbConfigOptionFile FreewbConfigOptionString

void freeWbConfigBindSync(FcitxGenericConfig *config)
{
    FcitxConfigFile *cfile = config->configFile;
    FcitxConfigFileDesc *cdesc = NULL;

    if (!cfile)
        return;

    cdesc = cfile->fileDesc;

    HASH_FOREACH(groupdesc, cdesc->groupsDesc, FcitxConfigGroupDesc)
    {
        FcitxConfigGroup *group = NULL;
        HASH_FIND_STR(cfile->groups, groupdesc->groupName, group);

        HASH_FOREACH(optiondesc, groupdesc->optionsDesc, FcitxConfigOptionDesc)
        {
            FcitxConfigOption *option = NULL;

            if (group)
            {
                HASH_FIND_STR(group->options, optiondesc->optionName, option);
            }
            FreewbConfigSyncValue(config, group, option, Raw2Value);

            // printf("%s:%s=%s\n",groupdesc->groupName,optiondesc->optionName,option->rawValue);
        }
    }
}

void FreewbConfigSyncValue(FcitxGenericConfig *config, FcitxConfigGroup *group, FcitxConfigOption *option, FcitxConfigSync sync)
{
    FcitxConfigOptionDesc *codesc = option->optionDesc;

    FreewbConfigOptionFunc f = NULL;

    if (codesc == NULL)
        return;

    if (sync == Value2Raw)
        if (option->filter)
        {
            option->filter(config, group, option, option->value.untype, sync, option->filterArg);
        }

    switch (codesc->type)
    {

    case T_Integer:
        // puts("int====");
        f = FreewbConfigOptionInteger;
        break;

    case T_Color:
        break;

    case T_Boolean:

        f = FreewbConfigOptionBoolean;
        // puts("bool====");
        break;

    case T_Enum:
        // puts("enum===");

        f = FreewbConfigOptionEnum;
        break;

    case T_String:
        // puts("string===");

        f = FreewbConfigOptionString;
        break;

    case T_I18NString:
        break;

    case T_Hotkey:
        // puts("hotkey====");

        f = FreewbConfigOptionHotkey;
        break;

    case T_File:
        f = FreewbConfigOptionFile;
        break;

    case T_Font:
        break;

    case T_Char:
        // puts("char===");

        f = FreewbConfigOptionChar;
        break;
    }

    FcitxConfigSyncResult r = SyncNoBinding;

    if (f)
        r = f(option, sync);

    if (r == SyncInvalid)
    {
        if (codesc->rawDefaultValue)
        {
            FcitxLog(WARNING, _("Option %s is Invalid, Use Default Value %s"), option->optionName, codesc->rawDefaultValue);
            fcitx_utils_free(option->rawValue);
            option->rawValue = strdup(codesc->rawDefaultValue);

            if (sync == Raw2Value)
                f(option, sync);
        }
        else
        {
            FcitxLog(ERROR, _("Option %s is Invalid."), option->optionName);
        }
    }

    if (sync == Raw2Value)
        if (option->filter)
            option->filter(config, group, option, option->value.untype, sync, option->filterArg);
}

FcitxConfigSyncResult FreewbConfigOptionInteger(FcitxConfigOption *option, FcitxConfigSync sync)
{
    if (!option->value.integer)
        return SyncNoBinding;

    switch (sync)
    {

    case Raw2Value:
    {
        int value = atoi(option->rawValue);
        if (value > option->optionDesc2->constrain.integerConstrain.max ||
            value < option->optionDesc2->constrain.integerConstrain.min)
            return SyncInvalid;
        *option->value.integer = value;
        return SyncSuccess;
    }

    case Value2Raw:
        if (*option->value.integer > option->optionDesc2->constrain.integerConstrain.max ||
            *option->value.integer < option->optionDesc2->constrain.integerConstrain.min)
            return SyncInvalid;

        if (option->rawValue)
            free(option->rawValue);

        sprintf(option->rawValue, "%d", *option->value.integer);

        return SyncSuccess;

    case ValueFree:
        return SyncSuccess;
    }

    return SyncInvalid;
}

FcitxConfigSyncResult FreewbConfigOptionBoolean(FcitxConfigOption *option, FcitxConfigSync sync)
{
    if (!option->value.boolvalue)
        return SyncNoBinding;

    switch (sync)
    {

    case Raw2Value:

        if (strcasecmp(option->rawValue, "true") == 0)
            *option->value.boolvalue = true;
        else
            *option->value.boolvalue = false;

        return SyncSuccess;

    case Value2Raw:
        if (*option->value.boolvalue)
            fcitx_utils_string_swap(&option->rawValue, "true");
        else
            fcitx_utils_string_swap(&option->rawValue, "false");

        return SyncSuccess;

    case ValueFree:
        return SyncSuccess;
    }

    return SyncInvalid;
}

FcitxConfigSyncResult FreewbConfigOptionEnum(FcitxConfigOption *option, FcitxConfigSync sync)
{
    if (!option->value.enumerate || !option->optionDesc)
        return SyncNoBinding;

    FcitxConfigOptionDesc *codesc = option->optionDesc;

    FcitxConfigEnum *cenum = &codesc->configEnum;

    int i = 0;

    switch (sync)
    {

    case Raw2Value:

        for (i = 0; i < cenum->enumCount; i++)
        {
            if (strcmp(cenum->enumDesc[i], option->rawValue) == 0)
            {
                *option->value.enumerate = i;
                return SyncSuccess;
            }
        }

        return SyncInvalid;

    case Value2Raw:

        if (*option->value.enumerate < 0 || *option->value.enumerate >= cenum->enumCount)
            return SyncInvalid;

        fcitx_utils_string_swap(&option->rawValue, cenum->enumDesc[*option->value.enumerate]);

        return SyncSuccess;

    case ValueFree:
        return SyncSuccess;
    }

    return SyncInvalid;
}

FcitxConfigSyncResult FreewbConfigOptionString(FcitxConfigOption *option, FcitxConfigSync sync)
{
    if (!option->value.string)
        return SyncNoBinding;

    switch (sync)
    {

    case Raw2Value:
        if (option->optionDesc2->constrain.stringConstrain.maxLength &&
            strlen(option->rawValue) > option->optionDesc2->constrain.stringConstrain.maxLength)
            return SyncInvalid;
        fcitx_utils_string_swap(option->value.string, option->rawValue);

        return SyncSuccess;

    case Value2Raw:
        if (option->optionDesc2->constrain.stringConstrain.maxLength &&
            strlen(*option->value.string) > option->optionDesc2->constrain.stringConstrain.maxLength)
            return SyncInvalid;
        fcitx_utils_string_swap(&option->rawValue, *option->value.string);

        return SyncSuccess;

    case ValueFree:
        fcitx_utils_free(*option->value.string);
        *option->value.string = NULL;
        return SyncSuccess;
    }

    return SyncInvalid;
}

FcitxConfigSyncResult FreewbConfigOptionHotkey(FcitxConfigOption *option, FcitxConfigSync sync)
{
    /* we assume all hotkey can have 2 candidate key */
    if (!option->value.hotkey)
        return SyncNoBinding;

    switch (sync)
    {

    case Raw2Value:

        if (option->value.hotkey[0].desc)
        {
            free(option->value.hotkey[0].desc);
            option->value.hotkey[0].desc = NULL;
        }

        if (option->value.hotkey[1].desc)
        {
            free(option->value.hotkey[1].desc);
            option->value.hotkey[1].desc = NULL;
        }

        FreewbHotkeySetKey(option->rawValue, option->value.hotkey);

        return SyncSuccess;

    case Value2Raw:

        if (option->rawValue)
            free(option->rawValue);

        if (option->value.hotkey[1].desc)
        {
            fcitx_utils_alloc_cat_str(option->rawValue, option->value.hotkey[0].desc, " ", option->value.hotkey[1].desc);
        }
        else if (option->value.hotkey[0].desc)
        {
            option->rawValue = strdup(option->value.hotkey[0].desc);
        }
        else
        {
            option->rawValue = strdup("");
        }
        return SyncSuccess;

    case ValueFree:
        FcitxHotkeyFree(option->value.hotkey);
        return SyncSuccess;
    }

    return SyncInvalid;
}

FcitxConfigSyncResult FreewbConfigOptionChar(FcitxConfigOption *option, FcitxConfigSync sync)
{
    if (!option->value.chr)
        return SyncNoBinding;

    switch (sync)
    {
    case Raw2Value:
        *option->value.chr = *option->rawValue;
        return SyncSuccess;

    case Value2Raw:
        option->rawValue = realloc(option->rawValue, 2);
        option->rawValue[0] = *option->value.chr;
        option->rawValue[1] = '\0';
        return SyncSuccess;

    case ValueFree:
        return SyncSuccess;
    }

    return SyncInvalid;
}

void FreewbHotkeySetKey(const char *str, FcitxHotkey *hotkey)
{
    char *p;
    char *strKey;
    int i = 0, j = 0, k;

    char *strKeys = fcitx_utils_trim(str);
    if (NULL == strKeys)
    {
    }

    p = strKeys;

    for (k = 0; k < 2; k++)
    {
        FcitxKeySym sym;
        unsigned int state;
        i = 0;

        while (p[i] != ' ' && p[i] != '\0')
            i++;

        strKey = strndup(p, i);

        strKey[i] = '\0';
        if (FreewbHotkeyParseKey(strKey, &sym, &state))
        {
            hotkey[j].sym = sym;
            hotkey[j].state = state;
            hotkey[j].desc = fcitx_utils_trim(strKey);
            j++;
        }

        free(strKey);

        if (p[i] == '\0')
            break;

        p = &p[i + 1];
    }

    for (; j < 2; j++)
    {
        hotkey[j].sym = 0;
        hotkey[j].state = 0;
        hotkey[j].desc = NULL;
    }

    free(strKeys);
}

boolean FreewbHotkeyParseKey(const char *strKey, FcitxKeySym *sym, unsigned int *state)
{
    const char *p;
    int iKey;
    int iKeyState = 0;
    p = strKey;
    if (strcasestr(p, "CTRL+"))
    {
        iKeyState |= FcitxKeyState_Ctrl;
        p += strlen("CTRL+");
    }

    if (strcasestr(p, "ALT+"))
    {
        iKeyState |= FcitxKeyState_Alt;
        p += strlen("ALT+");
    }

    if (strcasestr(strKey, "SHIFT+"))
    {
        iKeyState |= FcitxKeyState_Shift;
        p += strlen("SHIFT+");
    }

    if (strcasestr(strKey, "SUPER+"))
    {
        iKeyState |= FcitxKeyState_Super;
        p += strlen("SUPER+");
    }

    iKey = FreewbHotkeyGetKeyList(p);

    if (iKey == -1)
        return false;

    *sym = iKey;

    *state = iKeyState;

    return true;
}

typedef struct _KEY_LIST
{
    /**
     * string name for the key in fcitx
     **/
    char *strKey;
    /**
     * the keyval for the key.
     **/
    FcitxKeySym code;
} KEY_LIST;

KEY_LIST freewbKeyList[] = {{"KEY_F1", FcitxKey_F1},
                            {"KEY_F2", FcitxKey_F2},
                            {"KEY_F3", FcitxKey_F3},
                            {"KEY_F4", FcitxKey_F4},
                            {"KEY_F5", FcitxKey_F5},
                            {"KEY_F6", FcitxKey_F6},
                            {"KEY_F7", FcitxKey_F7},
                            {"KEY_F8", FcitxKey_F8},
                            {"KEY_F9", FcitxKey_F9},
                            {"KEY_F10", FcitxKey_F10},
                            {"KEY_F11", FcitxKey_F11},
                            {"KEY_F12", FcitxKey_F12},
                            {"KEY_A", FcitxKey_A},
                            {"KEY_B", FcitxKey_B},
                            {"KEY_C", FcitxKey_C},
                            {"KEY_D", FcitxKey_D},
                            {"KEY_E", FcitxKey_E},
                            {"KEY_F", FcitxKey_F},
                            {"KEY_G", FcitxKey_G},
                            {"KEY_H", FcitxKey_H},
                            {"KEY_I", FcitxKey_I},
                            {"KEY_J", FcitxKey_J},
                            {"KEY_K", FcitxKey_K},
                            {"KEY_L", FcitxKey_L},
                            {"KEY_M", FcitxKey_M},
                            {"KEY_N", FcitxKey_N},
                            {"KEY_O", FcitxKey_O},
                            {"KEY_P", FcitxKey_P},
                            {"KEY_Q", FcitxKey_Q},
                            {"KEY_R", FcitxKey_R},
                            {"KEY_S", FcitxKey_S},
                            {"KEY_T", FcitxKey_T},
                            {"KEY_U", FcitxKey_U},
                            {"KEY_V", FcitxKey_V},
                            {"KEY_W", FcitxKey_W},
                            {"KEY_X", FcitxKey_X},
                            {"KEY_Y", FcitxKey_Y},
                            {"KEY_Z", FcitxKey_Z},
                            {"KEY_HOME", FcitxKey_Home},
                            {"KEY_END", FcitxKey_End},
                            {"KEY_UP", FcitxKey_Up},
                            {"KEY_DOWN", FcitxKey_Down},
                            {"KEY_LEFT", FcitxKey_Left},
                            {"KEY_RIGHT", FcitxKey_Right},
                            {"KEY_BACKQUOTE", FcitxKey_grave},
                            {"KEY_DASH", FcitxKey_minus},
                            {"KEY_EQUAL", FcitxKey_equal},
                            {"KEY_LEFT_BRACKET", FcitxKey_bracketleft},
                            {"KEY_RIGHT_BRACKET", FcitxKey_bracketright},
                            {"KEY_BACK_SLASH", FcitxKey_backslash},
                            {"KEY_SEMICOLON", FcitxKey_semicolon},
                            {"KEY_QUOTE", FcitxKey_apostrophe},
                            {"KEY_COMMA", FcitxKey_comma},
                            {"KEY_PERIOD", FcitxKey_period},
                            {"KEY_SLASH", FcitxKey_slash},

                            {"`", FcitxKey_grave},
                            {"-", FcitxKey_minus},
                            {"=", FcitxKey_equal},
                            {"[", FcitxKey_bracketleft},
                            {"]", FcitxKey_bracketright},
                            {"\\", FcitxKey_backslash},
                            {";", FcitxKey_semicolon},
                            {"'", FcitxKey_apostrophe},
                            {",", FcitxKey_comma},
                            {".", FcitxKey_period},
                            {"/", FcitxKey_slash},
                            {"A", FcitxKey_A},
                            {"B", FcitxKey_B},
                            {"C", FcitxKey_C},
                            {"D", FcitxKey_D},
                            {"E", FcitxKey_E},
                            {"F", FcitxKey_F},
                            {"G", FcitxKey_G},
                            {"H", FcitxKey_H},
                            {"I", FcitxKey_I},
                            {"J", FcitxKey_J},
                            {"K", FcitxKey_K},
                            {"L", FcitxKey_L},
                            {"M", FcitxKey_M},
                            {"N", FcitxKey_N},
                            {"O", FcitxKey_O},
                            {"P", FcitxKey_P},
                            {"Q", FcitxKey_Q},
                            {"R", FcitxKey_R},
                            {"S", FcitxKey_S},
                            {"T", FcitxKey_T},
                            {"U", FcitxKey_U},
                            {"V", FcitxKey_V},
                            {"W", FcitxKey_W},
                            {"X", FcitxKey_X},
                            {"Y", FcitxKey_Y},
                            {"Z", FcitxKey_Z},
                            {"KEY_BACKSPACE", FcitxKey_BackSpace},
                            {"KEY_TAB", FcitxKey_Tab},
                            {"KEY_CAPS", FcitxKey_Caps_Lock},
                            {"KEY_ENTER", FcitxKey_Return},
                            {"KEY_INSERT", FcitxKey_Insert},
                            {"KEY_DEL", FcitxKey_Delete},
                            {"KEY_SPACE", FcitxKey_space},
                            {"KEY_ESC", FcitxKey_Escape},
                            {"KEY_LEFT_SHIFT", FcitxKey_Shift_L},
                            {"KEY_RIGHT_SHIFT", FcitxKey_Shift_R},
                            {"KEY_LEFT_CTRL", FcitxKey_Control_L},
                            {"KEY_RIGHT_CTRL", FcitxKey_Control_R},
                            {"KEY_PAGE_UP", FcitxKey_Page_Up},
                            {"KEY_PAGE_DOWN", FcitxKey_Page_Down},

                            {"KEY_NONEs", FcitxKey_None},
                            {"\0", 0}};

int FreewbHotkeyGetKeyList(const char *strKey)
{
    int i;

    i = 0;
    for (;;)
    {
        if (!freewbKeyList[i].code)
            break;

        if (0 == strcasecmp(strKey, freewbKeyList[i].strKey))
            return freewbKeyList[i].code;
        i++;
    }

    if (strlen(strKey) == 1)
        return strKey[0];

    return -1;
}

char *FreewbHotkeyGetKeyChar(int sym)
{
    int i;

    i = 0;
    for (;;)
    {
        if (!freewbKeyList[i].code)
            break;

        if (freewbKeyList[i].code == sym)
            return freewbKeyList[i].strKey;
        i++;
    }

    return NULL;
}