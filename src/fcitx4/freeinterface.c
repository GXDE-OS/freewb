#include "freeinterface.h"

#include "ipc/ipc.h"

static DBusMessage *createSettingsMethodCallMessage(const char *methodName)
{
    return dbus_message_new_method_call(FREEWUBI_SETTINGS_SERVICENAME, FREEWUBI_SETTINGS_OBJECTPATH, FREEWUBI_SETTINGS_INTERFACE, methodName);
}

void FreeWubiServiceAddUsrParseDirect(DBusConnection *conn, char *wordText, char *wordCode)
{
    DBusMessage *msg;
    DBusMessageIter args;
    int flg = 1;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_generate_usr_word");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &flg, DBUS_TYPE_STRING, &wordText, DBUS_TYPE_STRING, &wordCode, DBUS_TYPE_INVALID))
    {
        FcitxLog(DEBUG, "Out Of Memory!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceAddUsrParse(DBusConnection *conn, TableMetaData *table, int flg, char *wordText, char *wordCode)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_generate_usr_word");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &flg, DBUS_TYPE_STRING, &wordText, DBUS_TYPE_STRING, &wordCode, DBUS_TYPE_INVALID))
    {
        FcitxLog(DEBUG, "Out Of Memory!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);

    if (flg == 1 && table->WubiDict)
    {
        TableDict *dict = table->WubiDict;
        int i = 0;
        if (!dict->recordHead)
            return;
        while (wordCode[0] != dict->recordIndex[i].cCode)
        {
            if (!dict->recordIndex[i].cCode)
                break;
            ++i;
        }
        RECORD *record = dict->recordIndex[i].record;
        if (!record)
            return;
        while (record != dict->recordHead)
        {
            if (strcmp(wordCode, record->strCode) <= 0)
                break;
            record = record->next;
        }
        while (strcmp(wordCode, record->strCode) == 0)
        {
            if (strcmp(wordText, record->strHZ) == 0 && record->type == RECORDTYPE_CONSTRUCT)
                return;
            record = record->next;
        }
        RECORD *recTemp = (RECORD *)fcitx_memory_pool_alloc(dict->pool, sizeof(RECORD));
        recTemp->owner = dict;
        recTemp->strCode = (char *)fcitx_memory_pool_alloc(dict->pool, strlen(wordCode) + 1);
        memset(recTemp->strCode, 0, strlen(wordCode) + 1);
        strcpy(recTemp->strCode, wordCode);
        recTemp->strHZ = (char *)fcitx_memory_pool_alloc(dict->pool, strlen(wordText) + 1);
        memset(recTemp->strHZ, 0, strlen(wordText) + 1);
        strcpy(recTemp->strHZ, wordText);
        recTemp->type = RECORDTYPE_CONSTRUCT;
        //         recTemp->iHit = 0;

        recTemp->prev = record->prev;
        record->prev->next = recTemp;
        recTemp->next = record;
        record->prev = recTemp;
        dict->iTableChanged = 1;
        dict->iRecordCount++;
        SaveTableDict(table);
    }
}

void FreeWubiServiceDeleteUsrParse(DBusConnection *conn, TableMetaData *table, int flg, char *wordText, char *wordCode)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_delete_usr_word");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &flg, DBUS_TYPE_STRING, &wordText, DBUS_TYPE_STRING, &wordCode, DBUS_TYPE_INVALID))
    {
        FcitxLog(DEBUG, "Out Of Memory!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);

    if (flg == 1 && table->WubiDict)
    {
        TableDict *dict = table->WubiDict;
        int i = 0;
        if (!dict->recordHead)
            return;
        while (wordCode[0] != dict->recordIndex[i].cCode)
        {
            if (!dict->recordIndex[i].cCode)
                break;
            ++i;
        }
        RECORD *record = dict->recordIndex[i].record;
        if (!record)
            return;
        while (record != dict->recordHead)
        {
            if (strcmp(wordCode, record->strCode) <= 0)
                break;
            record = record->next;
        }
        while (strcmp(wordCode, record->strCode) == 0)
        {
            if (strcmp(wordText, record->strHZ) == 0 /*&&record->type == RECORDTYPE_CONSTRUCT*/)
            {
                record->prev->next = record->next;
                record->next->prev = record->prev;
                /*
                 * since we use memory pool, don't free record
                 * though free list is currently not supported, but it's ok
                 * people will not delete phrase so many times*/
                if (dict->recordIndex[i].record == record)
                    dict->recordIndex[i].record = record->next;
                dict->iTableChanged = 1;
                dict->iRecordCount--;
                SaveTableDict(table);
                return;
            }
            record = record->next;
        }
    }
}

void FreeWubiServiceResetUerWordFlag(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_usr_word_load_ok");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceResetQuickTableFlag(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_quick_table_load_ok");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceResetTableFlag(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_ime_table_load_ok");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceExitFreewbPanel(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_panel_exit");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceDictQuery(DBusConnection *conn, char *wordText)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_dict_query");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &wordText, DBUS_TYPE_INVALID);
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchFreeIm(DBusConnection *conn, int imState)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_internal_input_method");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    dbus_message_append_args(msg, DBUS_TYPE_INT32, &imState, DBUS_TYPE_INVALID);
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchImState(DBusConnection *conn, int imState)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_freewb");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    dbus_message_append_args(msg, DBUS_TYPE_INT32, &imState, DBUS_TYPE_INVALID);
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
    // printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa switch imstate :%d\n", imState);
    return;
}

void FreeWubiServiceSwitchToolbarState(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_toolbar_hide_flg");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchCapState(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_caps_state");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchCandiwinState(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_candiwin_hide_flg");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchSkin(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_skin");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchVk(DBusConnection *conn, int flg)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_vk");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    dbus_message_append_args(msg, DBUS_TYPE_INT32, &flg, DBUS_TYPE_INVALID);
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchCharSet(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_char_set");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchUncommon(DBusConnection *conn, char *wordText, int flg)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_word_freq_switch_ok");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &wordText, DBUS_TYPE_INT32, &flg, DBUS_TYPE_INVALID))
    {
        FcitxLog(DEBUG, "Out Of Memory!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchSmartPunc(DBusConnection *conn, int flg)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_set_mark_auto_pairs_flg");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &flg, DBUS_TYPE_INVALID))
    {
        FcitxLog(DEBUG, "Out Of Memory!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchRecodeProof(DBusConnection *conn, int flg)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_set_recode_calib_flg");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &flg, DBUS_TYPE_INVALID))
    {
        FcitxLog(DEBUG, "Out Of Memory!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchChttrans(DBusConnection *conn, int flg)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_simp_or_trad");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &flg, DBUS_TYPE_INVALID))
    {
        FcitxLog(DEBUG, "Out Of Memory!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceOpenSysConf(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_open_ui_setting");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceShowVersion(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_show_version_info");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceOpenProfessionalConf(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_open_advanced_setting");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceModQuickTable(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_edit_quick_table");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceModUserTable(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_edit_usr_table");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceModWubiTable(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_edit_wubi_table");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceModPinyinTable(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_edit_pinyin_table");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceOpenConfDir(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_open_freewb_dir");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceCloseVkBoard(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_close_vk");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSwitchTable(DBusConnection *conn)
{
    DBusMessage *msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_switch_lexicon");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

void FreeWubiServiceSetCharWidth(DBusConnection *conn, int charWidth, int PuncMode)
{
    DBusMessage *msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = createSettingsMethodCallMessage("slot_dbus_set_charWidth_and_markMode");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &charWidth, DBUS_TYPE_INT32, &PuncMode, DBUS_TYPE_INVALID))
    {
        FcitxLog(DEBUG, "Out Of Memory!");
        return;
    }
    if (!dbus_connection_send(conn, msg, &serial))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}

int FreeWubiServiceCreateFreewbPanel(DBusConnection *conn)
{
    DBusMessage *msg;
    DBusMessageIter args;
    DBusPendingCall *pending = NULL;
    msg = createSettingsMethodCallMessage("slot_dbus_create_freewb_panel");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return 0;
    }
    if (!dbus_connection_send_with_reply(conn, msg, &pending, -1))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return 0;
    }
    int result = 0;
    if (!pending)
    {
        FcitxLog(INFO, "connec erro!");
        dbus_message_unref(msg);
        return 0;
    }
    dbus_connection_flush(conn);
    dbus_message_unref(msg);

    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if (!msg)
        FcitxLog(INFO, "msg erro!");
    dbus_pending_call_unref(pending);

    if (!dbus_message_iter_init(msg, &args))
    {
        FcitxLog(INFO, "init erro!");
        return 0;
    }
    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args))
    {
        FcitxLog(INFO, "type erro!");
        return 0;
    }
    dbus_message_iter_get_basic(&args, &result);

    // free the message
    dbus_message_unref(msg);
    return result;
}

char *FreeWubiServiceGetClipboard(DBusConnection *conn)
{
    DBusMessage *msg;
    DBusMessageIter args;
    DBusPendingCall *pending = NULL;
    msg = createSettingsMethodCallMessage("slot_dbus_get_clipboard_text");
    if (NULL == msg)
    {
        FcitxLog(DEBUG, "set msg erro!");
        return 0;
    }
    if (!dbus_connection_send_with_reply(conn, msg, &pending, -1))
    {
        FcitxLog(DEBUG, "send msg erro!");
        return 0;
    }
    char *result = NULL;
    if (!pending)
    {
        FcitxLog(INFO, "connec erro!");
        dbus_message_unref(msg);
        return 0;
    }
    dbus_connection_flush(conn);
    dbus_message_unref(msg);

    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if (!msg)
        FcitxLog(INFO, "msg erro!");
    dbus_pending_call_unref(pending);

    if (!dbus_message_iter_init(msg, &args))
    {
        FcitxLog(INFO, "init erro!");
        return 0;
    }
    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
    {
        FcitxLog(INFO, "type erro!");
        return 0;
    }
    dbus_message_iter_get_basic(&args, &result);

    // free the message
    dbus_message_unref(msg);
    return result;
}
