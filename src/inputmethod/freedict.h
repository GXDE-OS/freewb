#ifndef FREEDICT_H
#define FREEDICT_H

#include <ctype.h>

#include <dbus/dbus.h>

#include "fcitx-utils/memory.h"
#include "fcitx/candidate.h"

#include "freewubi-config.h"

#define MAX_CODE_LENGTH 50
#define PHRASE_MAX_LENGTH 128
#define TABLE_AUTO_SAVE_AFTER 1024
#define AUTO_PHRASE_COUNT 10000
#define SINGLE_HZ_COUNT 66000

#define RECORDTYPE_NORMAL 0x0
#define RECORDTYPE_PINYIN 0x1
#define RECORDTYPE_CONSTRUCT 0x2
#define RECORDTYPE_UNCOMMON 0x3
#define RECORDTYPE_S2T 0x4
struct _TableDict;

typedef struct
{
    unsigned char iFlag;  // 1 --> 正序   0 --> 逆序
    unsigned char iWhich; // 第几个字
    unsigned char iIndex; // 第几个编码
} RULE_RULE;

typedef struct
{
    unsigned char iWords; // 多少个字
    unsigned char iFlag;  // 1 --> 大于等于iWords  0 --> 等于iWords
    RULE_RULE *rule;
} RULE;

typedef struct _RECORD
{
    char *strCode;
    char *strHZ;
    struct _RECORD *next;
    struct _RECORD *prev;
    int8_t type;
    struct _TableDict *owner;
} RECORD;

typedef struct _AUTOPHRASE
{
    char *strHZ;
    char *strCode;
    char iSelected;
    struct _AUTOPHRASE *next; // 构造一个单向链表
} AUTOPHRASE;

typedef struct _QUCIK_TABLE
{
    unsigned char key;
    char value[UTF8_MAX_LENGTH * 128];
    struct _QUCIK_TABLE *next;
} QUCIK_TABLE;

/* 根据键码生成一个简单的索引，指向该键码起始的第一个记录 */
typedef struct
{
    RECORD *record;
    char cCode;
} RECORD_INDEX;

typedef struct
{
    char strHZ[UTF8_MAX_LENGTH + 1];
} SINGLE_HZ;

typedef struct
{
    SINGLE_HZ recordHZ[4]; // 上屏的单字
    int recordIndex;       // 记录上屏的单字个数    当上屏不是单字时清零
} AUTORECORD;

typedef struct _AUTO_ENG
{
    char value[10];
    struct _AUTO_ENG *next;
} AUTO_ENG;

typedef enum
{
    CM_NONE,
    CM_ALT,
    CM_CTRL,
    CM_SHIFT,
    _CM_COUNT
} ChooseModifier;

typedef enum
{
    FREE_WUBI,
    FREE_WBPY,
    FREE_PINYIN
} TableType;

typedef struct _TableDict
{
    char tableName[30];
    char tableInfo[100];
    char tableCreateTime[20];
    char strEndKeys[5];
    char strSpecialKeys[5];
    char strCodeType[5];
    char strStraightUPKeys[10];
    char cWildChar;
    TableType tableType;
    char *strInputCode;
    RECORD_INDEX *recordIndex;
    unsigned char iCodeLength;
    unsigned char bRule;
    RULE *rule;
    uint32_t iRecordCount;
    RECORD *tableSingleHZ[SINGLE_HZ_COUNT];
    RECORD *s2tSignleHZ[SINGLE_HZ_COUNT];
    unsigned int iTableIndex;
    RECORD *currentRecord;
    RECORD *recordHead;
    RECORD *s2tRecordHead;
    RECORD *s2tCurrentRecord;
    int iTableChanged;
    int iHZLastInputCount;
    SINGLE_HZ hzLastInput[PHRASE_MAX_LENGTH]; // Records last HZ input
    //     RECORD* promptCode[256];
    FcitxMemoryPool *pool;
} TableDict;

typedef struct
{
    FcitxGenericConfig config;

    FcitxfreewubiConfig* freeWubiConfig;

    // zyp add
    TableType tableType;
    TableDict *WubiDict;
    TableDict *PinyinDict;
    AUTOPHRASE *autoPhrase;    // 智能词组
    AUTOPHRASE *currentPhrase; // 当前词组
    AUTOPHRASE *insertPoint;   // 智能词组插入点
    AUTORECORD *autoRecord;    // 智能词组记录的汉字

    QUCIK_TABLE *quickTable; // 快速码表
    AUTO_ENG *autoEng;       // 自动转英文字符串
} TableMetaData;

boolean LoadTableDict(TableMetaData *tableMetaData);
void SaveTableDict(TableMetaData *tableMetaData);
void FreeTableDict(TableMetaData *tableMetaData);
boolean LoadUsrDict(TableMetaData *tableMetaData);
boolean LoadS2tDict(TableMetaData *tableMetaData);       // 普通简体繁体转换表
boolean LoadS2tManyDict(TableMetaData *tableMetaData);   // 一对多简体繁体转换表
boolean LoadS2tPhraseDict(TableMetaData *tableMetaData); // 特殊词组简体繁体转换表
boolean TableCalPhraseCode(TableDict *tableDict, char *strHZ, char *strCode);
int TableCompareCode(const char *strUser, const char *strDict, boolean exactMatch);
int TableFindFirstMatchCode(TableMetaData *tableMetaData, const char *strCodeInput, boolean exactMatch, boolean cacheCurrentRecord);
RECORD *TableFindPhrase(const TableDict *tableDict, const char *strHZ);
RECORD *TableFindPhraseByCode(const TableDict *tableDict, const char *strCode, const char *strHZ);
boolean IsInputKey(const TableMetaData *tableMetaData, int iKey);
boolean IsUncommonKey(const TableMetaData *tableMetaData, int iKey, int state);
unsigned int CalHZIndex(char *strHZ);
char *getFreewbPath();
void adjustOrder(DBusConnection* conn, TableMetaData *tableMetaData, TableDict *dict, char *wordText, char *wordCode);
boolean LoadQuickTable(TableMetaData *tableMetaData);
void freeQucikTable(TableMetaData *tableMetaData);
QUCIK_TABLE *findQuickPharse(TableMetaData *tableMetaData, FcitxKeySym sym);

boolean LoadAutoEng(TableMetaData *tableMetaData, const char* strAutoEng);
void freeAutoEng(TableMetaData *tableMetaData);
boolean isAutoEngStr(TableMetaData *tableMetaData, char *str);

char *s2tConvers(TableMetaData *tableMetaData, const char *simpel);
CONFIG_BINDING_DECLARE(TableMetaData);

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 0;
