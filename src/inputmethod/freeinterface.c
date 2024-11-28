#include <libintl.h>
#include "fcitx-utils/log.h"
#include "freewubi-internal.h"
#include "freedict.h"
#include <dbus/dbus.h>

void addUsrParseDirect(Fcitxfreewubi *fwb,char* wordText, char* wordCode){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args; 
    int flg=1;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_generate_usr_word"); // name of the signal/method
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    if (!dbus_message_append_args(
            msg,
            DBUS_TYPE_INT32, &flg,
            DBUS_TYPE_STRING, &wordText,
            DBUS_TYPE_STRING, &wordCode,
            DBUS_TYPE_INVALID
            )) {
        FcitxLog(DEBUG, "Out Of Memory!");
    return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);    
}



void addUsrParse(Fcitxfreewubi *fwb,int flg,char* wordText, char* wordCode)
{

    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args; 
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_generate_usr_word"); // name of the signal/method
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    if (!dbus_message_append_args(
            msg,
            DBUS_TYPE_INT32, &flg,
            DBUS_TYPE_STRING, &wordText,
            DBUS_TYPE_STRING, &wordCode,
            DBUS_TYPE_INVALID
            )) {
        FcitxLog(DEBUG, "Out Of Memory!");
    return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
    
    if(flg==1&&fwb->table->WubiDict){
        TableDict *dict = fwb->table->WubiDict;
        int i=0;
        if(!dict->recordHead)
            return;
        while (wordCode[0] != dict->recordIndex[i].cCode) {
            if (!dict->recordIndex[i].cCode)
                break;
            ++i;
        }
        RECORD* record = dict->recordIndex[i].record;
        if(!record)
            return;
        while (record != dict->recordHead) {
            if(strcmp(wordCode,record->strCode)<=0)
                break;
            record = record->next;
        }   
        while(strcmp(wordCode,record->strCode)==0){
            if(strcmp(wordText,record->strHZ)==0&&record->type==RECORDTYPE_CONSTRUCT)
                return;
            record = record->next;
        }
        RECORD *recTemp = (RECORD*)fcitx_memory_pool_alloc(dict->pool, sizeof(RECORD));
        recTemp->owner = dict;
        recTemp->strCode = (char*)fcitx_memory_pool_alloc(dict->pool,strlen(wordCode)+1);
        memset(recTemp->strCode,0,strlen(wordCode)+1);
        strcpy(recTemp->strCode,wordCode);
        recTemp->strHZ = (char*)fcitx_memory_pool_alloc(dict->pool,strlen(wordText)+1);
        memset(recTemp->strHZ,0,strlen(wordText)+1);
        strcpy(recTemp->strHZ,wordText);
        recTemp->type = RECORDTYPE_CONSTRUCT;
//         recTemp->iHit = 0;
        
        recTemp->prev = record->prev;
        record->prev->next = recTemp;
        recTemp->next = record;
        record->prev = recTemp;
        dict->iTableChanged = 1;
        dict->iRecordCount++;
        SaveTableDict(fwb->table);
    }
}
void deleteUsrParse(Fcitxfreewubi *fwb,int flg,char* wordText, char* wordCode){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args; 
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_delete_usr_word"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    if (!dbus_message_append_args(
            msg,
            DBUS_TYPE_INT32, &flg,
            DBUS_TYPE_STRING, &wordText,
            DBUS_TYPE_STRING, &wordCode,
            DBUS_TYPE_INVALID
            )) {
        FcitxLog(DEBUG, "Out Of Memory!");
    return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);  
    
    if(flg==1&&fwb->table->WubiDict){
        TableDict *dict = fwb->table->WubiDict;
        int i=0;
        if(!dict->recordHead)
            return;
        while (wordCode[0] != dict->recordIndex[i].cCode) {
            if (!dict->recordIndex[i].cCode)
                break;
            ++i;
        }
        RECORD* record = dict->recordIndex[i].record;
        if(!record)
            return;
        while (record != dict->recordHead) {
            if(strcmp(wordCode,record->strCode)<=0)
                break;
            record = record->next;
        }   
        while(strcmp(wordCode,record->strCode)==0){
            if(strcmp(wordText,record->strHZ)==0/*&&record->type == RECORDTYPE_CONSTRUCT*/){
                record->prev->next = record->next;
                record->next->prev = record->prev;
    /*
     * since we use memory pool, don't free record
     * though free list is currently not supported, but it's ok
     * people will not delete phrase so many times*/
                if(dict->recordIndex[i].record==record)
                    dict->recordIndex[i].record = record->next;    
                dict->iTableChanged = 1;
                dict->iRecordCount--;
                SaveTableDict(fwb->table);
                return;
            }
            record = record->next;
        }

    }    
}
void resetUerWordFlag(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_usr_word_load_ok"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);    
    
}
void resetQuickTableFlag(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_quick_table_load_ok"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);    
    
}
void resetTableFlag(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_ime_table_load_ok"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);    
    
}
void exitFreewbPanel(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                       "slot_dbus_panel_exit"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void dictQuery(Fcitxfreewubi *fwb,char* wordText){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_dict_query"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &wordText, DBUS_TYPE_INVALID);
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void switchFreeIm(Fcitxfreewubi *fwb,int imState){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_switch_internal_input_method"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    dbus_message_append_args(msg, DBUS_TYPE_INT32, &imState, DBUS_TYPE_INVALID);
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);       
}

void switchImState(Fcitxfreewubi *fwb,int imState)
{
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                        "/",
                                        "com.freewb.host",
                                        "slot_dbus_switch_freewb"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    dbus_message_append_args(msg, DBUS_TYPE_INT32, &imState, DBUS_TYPE_INVALID);
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
    //printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa switch imstate :%d\n", imState);
    return;
}
void switchToolbarState(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_switch_toolbar_hide_flg"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}

void switchCapState(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_switch_caps_state"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}

void switchCandiwinState(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_switch_candiwin_hide_flg"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void switchSkin(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_switch_skin"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void switchVk(Fcitxfreewubi *fwb,int flg){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_switch_vk"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    dbus_message_append_args(msg, DBUS_TYPE_INT32, &flg, DBUS_TYPE_INVALID);
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);    
}
void switchCharSet(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_switch_char_set"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void switchUncommon(Fcitxfreewubi *fwb,char *wordText, int flg ){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args; 
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_word_freq_switch_ok"); // name of the signal/method
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    if (!dbus_message_append_args(
            msg,
            DBUS_TYPE_STRING, &wordText,
            DBUS_TYPE_INT32, &flg,
            DBUS_TYPE_INVALID
            )) {
        FcitxLog(DEBUG, "Out Of Memory!");
    return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}
void switchSmartPunc(Fcitxfreewubi *fwb,int flg){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args; 
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_set_mark_auto_pairs_flg"); // name of the signal/method
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    if (!dbus_message_append_args(
            msg,
            DBUS_TYPE_INT32, &flg,
            DBUS_TYPE_INVALID
            )) {
        FcitxLog(DEBUG, "Out Of Memory!");
    return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);    
}
void switchRecodeProof(Fcitxfreewubi *fwb,int flg){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args; 
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_set_recode_calib_flg"); // name of the signal/method
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    if (!dbus_message_append_args(
            msg,
            DBUS_TYPE_INT32, &flg,
            DBUS_TYPE_INVALID
            )) {
        FcitxLog(DEBUG, "Out Of Memory!");
    return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}
void switchChttrans(Fcitxfreewubi *fwb,int flg){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args; 
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_switch_simp_or_trad"); // name of the signal/method
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    if (!dbus_message_append_args(
            msg,
            DBUS_TYPE_INT32, &flg,
            DBUS_TYPE_INVALID
            )) {
        FcitxLog(DEBUG, "Out Of Memory!");
    return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);    
}
void openSysConf(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_open_ui_setting"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void showVersion(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_show_version_info"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void openProfessionalConf(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_open_advanced_setting"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void modQuickTable(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_edit_quick_table"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void modUserTable(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_edit_usr_table"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void modWubiTable(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_edit_wubi_table"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void modPinyinTable(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_edit_pinyin_table"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void openConfDir(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_open_freewb_dir"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void closeVkBoard(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_close_vk"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void switchTable(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_switch_lexicon"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);     
}
void setCharWidth(Fcitxfreewubi *fwb ,int charWidth, int PuncMode ){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args; 
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_set_charWidth_and_markMode"); // name of the signal/method
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return;
    }
    dbus_message_iter_init_append(msg,&args);
    if (!dbus_message_append_args(
            msg,
            DBUS_TYPE_INT32, &charWidth,
            DBUS_TYPE_INT32, &PuncMode,
            DBUS_TYPE_INVALID
            )) {
        FcitxLog(DEBUG, "Out Of Memory!");
    return;
    }
    if(!dbus_connection_send(conn,msg,&serial)){
        FcitxLog(DEBUG, "send msg erro!");
        return;
    }
    dbus_message_unref(msg);
}
int createFreewbPanel(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args;
    DBusPendingCall *pending = NULL;
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_create_freewb_panel"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return 0;
    }
    if(!dbus_connection_send_with_reply(conn, msg, &pending,-1)){
        FcitxLog(DEBUG, "send msg erro!");
        return 0;
    }
    int result = 0;
    if(!pending){
        FcitxLog(INFO, "connec erro!");
        dbus_message_unref(msg);
        return 0;
    }
    dbus_connection_flush(conn);
    dbus_message_unref(msg);
    
    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if(!msg)
        FcitxLog(INFO, "msg erro!");
    dbus_pending_call_unref(pending);
    
    if(!dbus_message_iter_init(msg,&args)){
        FcitxLog(INFO, "init erro!");
        return 0;
    }
    if(DBUS_TYPE_INT32!=dbus_message_iter_get_arg_type(&args)){
        FcitxLog(INFO, "type erro!");
        return 0;
    }
    dbus_message_iter_get_basic(&args,&result);

    // free the message
    dbus_message_unref(msg);
    return result; 
    
}
char* getClipboard(Fcitxfreewubi *fwb){
    DBusConnection *conn = getFreeDbusConn(fwb);
    DBusMessage* msg;
    DBusMessageIter args;
    DBusPendingCall *pending = NULL;
    msg = dbus_message_new_method_call("com.freewb.www",
                                       "/",
                                       "com.freewb.host",
                                      "slot_dbus_get_clipboard_text"); // name of the signal
    if (NULL == msg) {
        FcitxLog(DEBUG, "set msg erro!");
        return 0;
    }
    if(!dbus_connection_send_with_reply(conn, msg, &pending,-1)){
        FcitxLog(DEBUG, "send msg erro!");
        return 0;
    }
    char* result = NULL;
    if(!pending){
        FcitxLog(INFO, "connec erro!");
        dbus_message_unref(msg);
        return 0;
    }
    dbus_connection_flush(conn);
    dbus_message_unref(msg);
    
    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if(!msg)
        FcitxLog(INFO, "msg erro!");
    dbus_pending_call_unref(pending);
    
    if(!dbus_message_iter_init(msg,&args)){
        FcitxLog(INFO, "init erro!");
        return 0;
    }
    if(DBUS_TYPE_STRING!=dbus_message_iter_get_arg_type(&args)){
        FcitxLog(INFO, "type erro!");
        return 0;
    }
    dbus_message_iter_get_basic(&args,&result);

    // free the message
    dbus_message_unref(msg);
    return result; 
}
