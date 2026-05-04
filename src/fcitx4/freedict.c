
#include "freedict.h"

#include "libdbus_proxy.h"
#include "utf8_in_gb18030.h"

#define WUBI_TEMP_FILE "wubi_XXXXXX"
#define PINYIN_TEMP_FILE "pinyin_XXXXXX"
#define USER_TEMP_FILE "user_XXXXXX"
#define QUICK_PATH "quick_table.txt"

#define S2T_PATH "/usr/share/freewb/data/s2t/gbks2t.tab"
#define S2T_MANY_PATH "/usr/share/freewb/data/s2t/single.txt"
#define S2T_PHRASE_PATH "/usr/share/freewb/data/s2t/phrase.txt"

char *getFreewbPath()
{
    char *freewbBin = malloc(200 * sizeof(char));
    strcpy(freewbBin, getenv("HOME"));
    strcat(freewbBin, "/.local/freewb");
    return freewbBin;
}

boolean LoadDict(TableMetaData *tableMetaData, TableDict *tableDict, FILE *fpDict)
{
    boolean error = false;
    char strCode[MAX_CODE_LENGTH + 1];
    char *strHZ = 0;
    RECORD *recTemp;
    unsigned int i = 0;
    uint32_t iTemp;
    char cChar = 0, cTemp;
    int iRecordIndex;
    tableDict->pool = fcitx_memory_pool_create();
#define CHECK_LOAD_TABLE_ERROR(SIZE)                                                                                             \
    if (size < (SIZE))                                                                                                           \
    {                                                                                                                            \
        error = true;                                                                                                            \
        break;                                                                                                                   \
    }
    do
    {
        // 先读取码表的信息
        // 判断版本信息
        size_t size;
        // 读取名字
        size = fcitx_utils_read_uint32(fpDict, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);

        size = fread(tableDict->tableName, sizeof(char), iTemp + 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(iTemp + 1);
        // 读取词库信息
        size = fcitx_utils_read_uint32(fpDict, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);
        size = fread(tableDict->tableInfo, sizeof(char), iTemp + 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(iTemp + 1);
        // 读取生成日期
        size = fcitx_utils_read_uint32(fpDict, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);
        size = fread(tableDict->tableCreateTime, sizeof(char), iTemp + 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(iTemp + 1);
        // 读取编码截止键
        size = fcitx_utils_read_uint32(fpDict, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);
        size = fread(tableDict->strEndKeys, sizeof(char), iTemp + 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(iTemp + 1);
        // 读取特殊符号引导符
        size = fcitx_utils_read_uint32(fpDict, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);
        size = fread(tableDict->strSpecialKeys, sizeof(char), iTemp + 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(iTemp + 1);
        // 读取编码方案类型
        size = fcitx_utils_read_uint32(fpDict, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);
        size = fread(tableDict->strCodeType, sizeof(char), iTemp + 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(iTemp + 1);
        // 读取径直上屏的标点
        size = fcitx_utils_read_uint32(fpDict, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);
        size = fread(tableDict->strStraightUPKeys, sizeof(char), iTemp + 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(iTemp + 1);
        // 读取UsedCodes
        size = fcitx_utils_read_uint32(fpDict, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);
        tableDict->strInputCode = (char *)realloc(tableDict->strInputCode, sizeof(char) * (iTemp + 1));
        size = fread(tableDict->strInputCode, sizeof(char), iTemp + 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(iTemp + 1);
        // 读取WildChar
        size = fread(&(tableDict->cWildChar), sizeof(uint8_t), 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(1);

        // 建立索引
        size_t tmp_len = strlen(tableDict->strInputCode) + 1;
        tableDict->recordIndex = (RECORD_INDEX *)fcitx_memory_pool_alloc(tableDict->pool, tmp_len * sizeof(RECORD_INDEX));
        for (iTemp = 0; iTemp < tmp_len; iTemp++)
        {
            tableDict->recordIndex[iTemp].cCode = 0;
            tableDict->recordIndex[iTemp].record = NULL;
        }
        /********************************************************************/
        tableDict->iCodeLength = 4;
        ;
        size = fread(&(tableDict->bRule), sizeof(unsigned char), 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(1);
        if (tableDict->bRule)
        { // 表示有组词规则
            tableDict->rule = (RULE *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RULE) * (tableDict->iCodeLength - 1));
            for (i = 0; i < tableDict->iCodeLength - 1; i++)
            {
                size = fread(&(tableDict->rule[i].iFlag), sizeof(unsigned char), 1, fpDict);
                CHECK_LOAD_TABLE_ERROR(1);
                size = fread(&(tableDict->rule[i].iWords), sizeof(unsigned char), 1, fpDict);
                CHECK_LOAD_TABLE_ERROR(1);
                tableDict->rule[i].rule =
                    (RULE_RULE *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RULE_RULE) * tableDict->iCodeLength);
                for (iTemp = 0; iTemp < tableDict->iCodeLength; iTemp++)
                {
                    size = fread(&(tableDict->rule[i].rule[iTemp].iFlag), sizeof(unsigned char), 1, fpDict);
                    CHECK_LOAD_TABLE_ERROR(1);
                    size = fread(&(tableDict->rule[i].rule[iTemp].iWhich), sizeof(unsigned char), 1, fpDict);
                    CHECK_LOAD_TABLE_ERROR(1);
                    size = fread(&(tableDict->rule[i].rule[iTemp].iIndex), sizeof(unsigned char), 1, fpDict);
                    CHECK_LOAD_TABLE_ERROR(1);
                }
            }
        }
        tableDict->recordHead = (RECORD *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
        tableDict->currentRecord = tableDict->recordHead;

        size = fcitx_utils_read_uint32(fpDict, &tableDict->iRecordCount);
        CHECK_LOAD_TABLE_ERROR(1);

        for (i = 0; i < SINGLE_HZ_COUNT; i++)
        {
            tableDict->tableSingleHZ[i] = (RECORD *)NULL;
        }
        iRecordIndex = 0;
        size_t bufSize = 0;
        for (i = 0; i < tableDict->iRecordCount; i++)
        {
            size = fcitx_utils_read_uint32(fpDict, &iTemp);
            CHECK_LOAD_TABLE_ERROR(1);
            size = fread(strCode, sizeof(int8_t), iTemp, fpDict);
            CHECK_LOAD_TABLE_ERROR(iTemp);

            recTemp = (RECORD *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
            recTemp->owner = tableDict;
            recTemp->strCode = (char *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(char) * iTemp);
            memset(recTemp->strCode, 0, sizeof(char) * iTemp);
            strcpy(recTemp->strCode, strCode);

            size = fcitx_utils_read_uint32(fpDict, &iTemp);
            CHECK_LOAD_TABLE_ERROR(1);

            if (iTemp > UTF8_MAX_LENGTH * 30)
            {
                error = true;
                break;
            }
            if (iTemp > bufSize)
            {
                bufSize = iTemp;
                strHZ = realloc(strHZ, bufSize);
            }
            size = fread(strHZ, sizeof(int8_t), iTemp, fpDict);
            CHECK_LOAD_TABLE_ERROR(iTemp);

            recTemp->strHZ = (char *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(char) * iTemp);
            strcpy(recTemp->strHZ, strHZ);

            size = fread(&cTemp, sizeof(int8_t), 1, fpDict);
            CHECK_LOAD_TABLE_ERROR(1);
            recTemp->type = cTemp;

            /* 建立索引 */
            if (cChar != recTemp->strCode[0])
            {
                cChar = recTemp->strCode[0];
                tableDict->recordIndex[iRecordIndex].cCode = cChar;
                tableDict->recordIndex[iRecordIndex].record = recTemp;

                iRecordIndex++;
            }
            /******************************************************************/
            /** 为单字生成一个表   */

            if (fcitx_utf8_strlen(recTemp->strHZ) == 1)
            {
                RECORD **tableSingleHZ = NULL;
                // if (recTemp->type == RECORDTYPE_NORMAL)
                if (isalpha(recTemp->strCode[0]))
                    tableSingleHZ = tableDict->tableSingleHZ;

                if (tableSingleHZ)
                {
                    iTemp = CalHZIndex(recTemp->strHZ);
                    if (iTemp < SINGLE_HZ_COUNT)
                    {
                        if (tableSingleHZ[iTemp])
                        {
                            if (strlen(strCode) > strlen(tableDict->tableSingleHZ[iTemp]->strCode))
                            {
                                tableSingleHZ[iTemp] = recTemp;
                            }
                        }
                        else
                        {
                            tableSingleHZ[iTemp] = recTemp;
                        }
                    }
                }
            }
            tableDict->currentRecord->next = recTemp;
            recTemp->prev = tableDict->currentRecord;
            tableDict->currentRecord = recTemp;
        }
        if (strHZ)
        {
            free(strHZ);
            strHZ = NULL;
        }

        tableDict->currentRecord->next = tableDict->recordHead;
        tableDict->recordHead->prev = tableDict->currentRecord;
    } while (0);
    fclose(fpDict);
    if (error)
    {
        fcitx_memory_pool_destroy(tableDict->pool);
        tableDict->pool = NULL;
        return false;
    }
    return true;
}

boolean LoadTableDict(TableMetaData *tableMetaData)
{
    FILE *fpWubiDict = NULL;
    FILE *fpPinyinDict = NULL;
    // 读入码表
    char *tablepath;
    char *path = getFreewbPath();
    fcitx_utils_alloc_cat_str(tablepath, path, "/data/mb/", tableMetaData->freeWubiConfig->WubiPath);

    fpWubiDict = fopen(tablepath, "r");
    free(tablepath);
    fcitx_utils_alloc_cat_str(tablepath, path, "/data/mb/", tableMetaData->freeWubiConfig->PinyinPath);
    fpPinyinDict = fopen(tablepath, "r");
    free(tablepath);
    free(path);

    if (!fpWubiDict || !fpPinyinDict)
        return false;
    boolean loaderror = true;

    tableMetaData->PinyinDict = fcitx_utils_new(TableDict);
    tableMetaData->PinyinDict->tableType = FREE_PINYIN;
    loaderror &= LoadDict(tableMetaData, tableMetaData->PinyinDict, fpPinyinDict);
    tableMetaData->WubiDict = fcitx_utils_new(TableDict);
    tableMetaData->WubiDict->tableType = FREE_WUBI;
    loaderror &= LoadDict(tableMetaData, tableMetaData->WubiDict, fpWubiDict);
    loaderror &= LoadUsrDict(tableMetaData);
    loaderror &= LoadQuickTable(tableMetaData);
    loaderror &= LoadS2tDict(tableMetaData);
    loaderror &= LoadS2tManyDict(tableMetaData);
    loaderror &= LoadS2tPhraseDict(tableMetaData);
    if (tableMetaData->WubiDict && !tableMetaData->WubiDict->pool)
        return false;
    if (tableMetaData->PinyinDict && !tableMetaData->PinyinDict->pool)
        return false;
    FcitxLog(DEBUG, _("Load Table Dict OK"));
    return loaderror;
}

boolean LoadUsrDict(TableMetaData *tableMetaData)
{
    char *tablepath;
    char *path = getFreewbPath();
    FILE *fpUsrDict = NULL;
    if (FREE_WUBI == tableMetaData->tableType || FREE_WBPY == tableMetaData->tableType)
    {
        fcitx_utils_alloc_cat_str(tablepath, path, "/data/", tableMetaData->freeWubiConfig->usrPath);
        fpUsrDict = fopen(tablepath, "r");
        free(path);
        free(tablepath);
        if (!fpUsrDict)
        {
            FcitxLog(INFO, _("user_word.txt not exists."));
            return false;
        }
    }
    if (fpUsrDict && tableMetaData->WubiDict)
    {
        TableDict *tableDict = tableMetaData->WubiDict;
        tableDict->currentRecord = tableDict->recordHead->next;
        RECORD *recTemp;
        RECORD *insertPoint;
        char data[256] = {0};

        fscanf(fpUsrDict, "%s", data);
        char *temp = data;
        while (*temp == '\t' || *temp == ' ')
            temp++;
        if (*temp != '[')
        {
            FcitxLog(INFO, _("Load User Dict ERROR"));
            return false;
        }
        temp++;
        if (strncmp(temp, "UserWord", 8) != 0)
        {
            FcitxLog(INFO, _("Load User Dict ERROR"));
            return false;
        }
        int findIndex = 0;
        int i = 0;
        while (!feof(fpUsrDict))
        {
            memset(data, 0, 256);
            fscanf(fpUsrDict, "%s", data);
            if (!data[0])
                continue;
            temp = data;
            i = 0;
            boolean isRepeat = false;
            while (*temp && *temp++ != '=')
            {
                i++;
            }
            if (!*temp)
                continue;
            while (tableDict->currentRecord != tableDict->recordHead)
            {
                if (strncmp(data, tableDict->currentRecord->strCode, i) <= 0)
                    break;
                if (tableDict->currentRecord->type == RECORDTYPE_CONSTRUCT)
                {
                    //                     FcitxLog(INFO, _("11tableDict->currentRecord:%s  %s"),
                    //                     tableDict->currentRecord->strCode,tableDict->currentRecord->strHZ); FcitxLog(INFO,
                    //                     _("11data:%s  %s"), data,temp);
                    tableDict->currentRecord->prev->next = tableDict->currentRecord->next;
                    tableDict->currentRecord->next->prev = tableDict->currentRecord->prev;
                    tableDict->iRecordCount--;
                    while (tableDict->currentRecord->strCode[0] != tableDict->recordIndex[findIndex].cCode)
                    {
                        if (!tableDict->recordIndex[findIndex].cCode)
                            break;
                        findIndex++;
                    }
                    if (tableDict->recordIndex[findIndex].record == tableDict->currentRecord)
                        tableDict->recordIndex[findIndex].record = tableDict->currentRecord->next;
                }
                tableDict->currentRecord = tableDict->currentRecord->next;
            }
            insertPoint = tableDict->currentRecord;
            while (strncmp(data, tableDict->currentRecord->strCode, i) == 0)
            {
                if (strcmp(temp, tableDict->currentRecord->strHZ) == 0 && tableDict->currentRecord->type == RECORDTYPE_CONSTRUCT)
                {
                    tableDict->currentRecord = tableDict->currentRecord->next;
                    isRepeat = true;
                    break;
                }
                if (tableDict->currentRecord->type == RECORDTYPE_CONSTRUCT)
                {
                    //                     FcitxLog(INFO, _("22tableDict->currentRecord:%s  %s"),
                    //                     tableDict->currentRecord->strCode,tableDict->currentRecord->strHZ);
                    tableDict->currentRecord->prev->next = tableDict->currentRecord->next;
                    tableDict->currentRecord->next->prev = tableDict->currentRecord->prev;
                    tableDict->iRecordCount--;
                    while (tableDict->currentRecord->strCode[0] != tableDict->recordIndex[findIndex].cCode)
                    {
                        if (!tableDict->recordIndex[findIndex].cCode)
                            break;
                        findIndex++;
                    }
                    if (tableDict->recordIndex[findIndex].record == tableDict->currentRecord)
                        tableDict->recordIndex[findIndex].record = tableDict->currentRecord->next;
                }
                tableDict->currentRecord = tableDict->currentRecord->next;
            }
            if (isRepeat)
                continue;
            recTemp = (RECORD *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
            recTemp->owner = tableDict;
            recTemp->strCode = (char *)fcitx_memory_pool_alloc(tableDict->pool, (i + 1) * sizeof(char));
            memset(recTemp->strCode, 0, (i + 1) * sizeof(char));

            recTemp->strHZ = (char *)fcitx_memory_pool_alloc(tableDict->pool, strlen(temp) + 1);
            memset(recTemp->strHZ, 0, strlen(temp) + 1);
            strncpy(recTemp->strCode, data, i);
            strcpy(recTemp->strHZ, temp);
            recTemp->type = RECORDTYPE_CONSTRUCT;
            //             recTemp->iHit = 0;
            //             FcitxLog(INFO, _("33tableDict->currentRecord:%s  %s"),
            //             tableDict->currentRecord->strCode,tableDict->currentRecord->strHZ);
            recTemp->prev = insertPoint->prev;
            insertPoint->prev->next = recTemp;
            recTemp->next = insertPoint;
            insertPoint->prev = recTemp;
            findIndex = 0;
            while (insertPoint->strCode[0] != tableDict->recordIndex[findIndex].cCode)
            {
                if (!tableDict->recordIndex[findIndex].cCode)
                    break;
                findIndex++;
            }
            if (tableDict->recordIndex[findIndex].record == insertPoint)
                tableDict->recordIndex[findIndex].record = recTemp;
            tableDict->iTableChanged = 1;
            tableDict->iRecordCount++;
        }
        while (tableDict->currentRecord != tableDict->recordHead)
        {
            if (tableDict->currentRecord->type == RECORDTYPE_CONSTRUCT)
            {
                tableDict->currentRecord->prev->next = tableDict->currentRecord->next;
                tableDict->currentRecord->next->prev = tableDict->currentRecord->prev;
                tableDict->iRecordCount--;
                while (tableDict->currentRecord->strCode[0] != tableDict->recordIndex[findIndex].cCode)
                {
                    if (!tableDict->recordIndex[findIndex].cCode)
                        break;
                    findIndex++;
                }
                if (tableDict->recordIndex[findIndex].record == tableDict->currentRecord)
                    tableDict->recordIndex[findIndex].record = tableDict->currentRecord->next;
            }
            tableDict->currentRecord = tableDict->currentRecord->next;
        }
        if (tableDict->iTableChanged)
            SaveTableDict(tableMetaData);
    }
    return true;
}

boolean LoadS2tDict(TableMetaData *tableMetaData)
{
    FILE *fpS2TDict = NULL;
    fpS2TDict = fopen(S2T_PATH, "r");
    if (fpS2TDict && tableMetaData->WubiDict)
    {
        int i;
        TableDict *tableDict = tableMetaData->WubiDict;
        for (i = 0; i < SINGLE_HZ_COUNT; i++)
        {
            tableDict->s2tSignleHZ[i] = (RECORD *)NULL;
        }
        tableDict->s2tRecordHead = (RECORD *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
        tableDict->s2tCurrentRecord = tableDict->s2tRecordHead;
        RECORD *recTemp;
        char *data = NULL;
        size_t lenth;
        char *temp = NULL;
        int len = 0;
        while (1)
        {
            if (getline(&data, &lenth, fpS2TDict) == -1)
                break;
            if (!*data || *data == '\n')
                continue;
            if (fcitx_utf8_strlen(data) - 1 != 2)
            {
                FcitxLog(INFO, _("data format error:%s"), data);
                continue;
            }

            len = fcitx_utf8_char_len(data);
            temp = data + len;
            recTemp = (RECORD *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
            recTemp->owner = tableDict;
            recTemp->strCode = (char *)fcitx_memory_pool_alloc(tableDict->pool, len + 1);
            memset(recTemp->strCode, 0, len + 1);
            strncpy(recTemp->strCode, data, len);

            len = fcitx_utf8_char_len(temp);
            recTemp->strHZ = (char *)fcitx_memory_pool_alloc(tableDict->pool, len + 1);
            memset(recTemp->strHZ, 0, len + 1);
            strncpy(recTemp->strHZ, temp, len);
            recTemp->type = RECORDTYPE_S2T;
            /******************************************************************/
            /** 为单字生成一个表   */
            // 全部都是单字
            int iTemp = 0;
            boolean findRepeat = false;
            RECORD **tableSingleHZ = tableDict->s2tSignleHZ;
            if (tableSingleHZ)
            {
                iTemp = CalHZIndex(recTemp->strCode);
                if (iTemp < SINGLE_HZ_COUNT)
                {
                    if (tableSingleHZ[iTemp])
                    {
                        RECORD *temp = tableSingleHZ[iTemp];
                        while (!strcmp(temp->strCode, recTemp->strCode) && temp != tableDict->s2tCurrentRecord)
                        {
                            if (!strcmp(temp->strHZ, recTemp->strHZ))
                            {
                                findRepeat = true;
                                break;
                            }
                            temp = temp->next;
                        }
                        if (findRepeat)
                            continue;
                        if (!strcmp(temp->strCode, recTemp->strCode) && temp == tableDict->s2tCurrentRecord)
                        {
                            tableDict->s2tCurrentRecord->next = recTemp;
                            recTemp->prev = tableDict->s2tCurrentRecord;
                            tableDict->s2tCurrentRecord = tableDict->s2tCurrentRecord->next;
                        }
                        else
                        {
                            temp->prev->next = recTemp;
                            recTemp->prev = temp->prev;
                            recTemp->next = temp;
                            temp->prev = recTemp;
                        }
                    }
                    else
                    {
                        tableSingleHZ[iTemp] = recTemp;
                        tableDict->s2tCurrentRecord->next = recTemp;
                        recTemp->prev = tableDict->s2tCurrentRecord;
                        tableDict->s2tCurrentRecord = tableDict->s2tCurrentRecord->next;
                    }
                }
            }
        }
        tableDict->s2tCurrentRecord->next = tableDict->s2tRecordHead;
        tableDict->s2tRecordHead->prev = tableDict->s2tCurrentRecord;
    }
    return true;
}

boolean LoadS2tManyDict(TableMetaData *tableMetaData)
{
    FILE *fpS2TDict = NULL;
    fpS2TDict = fopen(S2T_MANY_PATH, "r");
    if (fpS2TDict && tableMetaData->WubiDict)
    {
        TableDict *tableDict = tableMetaData->WubiDict;
        tableDict->s2tCurrentRecord = tableDict->s2tRecordHead->prev;
        RECORD *recTemp;
        char *data = NULL;
        size_t lenth;
        char *temp = NULL;
        int len = 0;
        int len2 = 0;
        while (1)
        {
            if (getline(&data, &lenth, fpS2TDict) == -1)
                break;
            if (!*data || *data == '\n')
                continue;
            len = fcitx_utf8_char_len(data);
            temp = data + len;
            while (*temp && *temp != '\n')
            {
                recTemp = (RECORD *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
                recTemp->owner = tableDict;
                recTemp->strCode = (char *)fcitx_memory_pool_alloc(tableDict->pool, len + 1);
                memset(recTemp->strCode, 0, len + 1);
                strncpy(recTemp->strCode, data, len);
                while (*temp == ' ' || *temp == '\t')
                    temp++;
                if (!*temp || *temp == '\n')
                    break;
                len2 = fcitx_utf8_char_len(temp);
                recTemp->strHZ = (char *)fcitx_memory_pool_alloc(tableDict->pool, len2 + 1);
                memset(recTemp->strHZ, 0, len2 + 1);
                strncpy(recTemp->strHZ, temp, len2);
                recTemp->type = RECORDTYPE_S2T;
                /******************************************************************/
                /** 为单字生成一个表   */
                // 全部都是单字
                int iTemp = 0;
                boolean findRepeat = false;
                RECORD **tableSingleHZ = tableDict->s2tSignleHZ;
                if (tableSingleHZ)
                {
                    iTemp = CalHZIndex(recTemp->strCode);
                    if (iTemp < SINGLE_HZ_COUNT)
                    {
                        if (tableSingleHZ[iTemp])
                        {
                            RECORD *tempR = tableSingleHZ[iTemp];
                            while (!strcmp(tempR->strCode, recTemp->strCode) && tempR != tableDict->s2tCurrentRecord)
                            {
                                if (!strcmp(tempR->strHZ, recTemp->strHZ))
                                {
                                    findRepeat = true;
                                    break;
                                }
                                tempR = tempR->next;
                            }
                            if (findRepeat)
                            {
                                temp += len2;
                                continue;
                            }
                            if (!strcmp(tempR->strCode, recTemp->strCode) && tempR == tableDict->s2tCurrentRecord)
                            {
                                tableDict->s2tCurrentRecord->next = recTemp;
                                recTemp->prev = tableDict->s2tCurrentRecord;
                                tableDict->s2tCurrentRecord = tableDict->s2tCurrentRecord->next;
                            }
                            else
                            {
                                tempR->prev->next = recTemp;
                                recTemp->prev = tempR->prev;
                                recTemp->next = tempR;
                                tempR->prev = recTemp;
                            }
                        }
                        else
                        {
                            tableSingleHZ[iTemp] = recTemp;
                            tableDict->s2tCurrentRecord->next = recTemp;
                            recTemp->prev = tableDict->s2tCurrentRecord;
                            tableDict->s2tCurrentRecord = tableDict->s2tCurrentRecord->next;
                        }
                    }
                }
                temp += len2;
            }
        }
        tableDict->s2tCurrentRecord->next = tableDict->s2tRecordHead;
        tableDict->s2tRecordHead->prev = tableDict->s2tCurrentRecord;
    }
    return true;
}

boolean LoadS2tPhraseDict(TableMetaData *tableMetaData)
{
    FILE *fpS2TDict = NULL;
    fpS2TDict = fopen(S2T_PHRASE_PATH, "r");
    if (fpS2TDict && tableMetaData->WubiDict)
    {
        TableDict *tableDict = tableMetaData->WubiDict;
        tableDict->s2tCurrentRecord = tableDict->s2tRecordHead->prev;
        RECORD *recTemp;
        char *data = NULL;
        size_t lenth;
        char *temp = NULL;
        int len = 0;
        while (!feof(fpS2TDict))
        {
            if (getline(&data, &lenth, fpS2TDict) == -1)
                break;
            if (!*data || *data == '\n')
                continue;
            int i = fcitx_utf8_strlen(data) - 1;
            if (i % 2)
            {
                FcitxLog(INFO, _("data format error:%s"), data);
                continue;
            }
            len = 0;
            temp = data;
            for (i = i / 2; i > 0; i--)
            {
                len += fcitx_utf8_char_len(temp);
                temp += fcitx_utf8_char_len(temp);
            }
            recTemp = (RECORD *)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
            recTemp->owner = tableDict;
            recTemp->strCode = (char *)fcitx_memory_pool_alloc(tableDict->pool, len + 1);
            memset(recTemp->strCode, 0, len + 1);
            strncpy(recTemp->strCode, data, len);

            char *ttm = temp;
            len = 0;
            while (*ttm && *ttm != '\n')
            {
                len++;
                ttm++;
            }
            recTemp->strHZ = (char *)fcitx_memory_pool_alloc(tableDict->pool, len + 1);
            memset(recTemp->strHZ, 0, len + 1);
            strncpy(recTemp->strHZ, temp, len);
            recTemp->type = RECORDTYPE_S2T;
            /******************************************************************/
            /** 为单字生成一个表   */
            // 全部都是单字
            int iTemp = 0;
            boolean findRepeat = false;
            RECORD **tableSingleHZ = tableDict->s2tSignleHZ;
            if (tableSingleHZ)
            {
                char strCode[10] = {0};
                strncpy(strCode, recTemp->strCode, fcitx_utf8_char_len(recTemp->strCode));
                iTemp = CalHZIndex(strCode);
                if (iTemp < SINGLE_HZ_COUNT)
                {
                    if (tableSingleHZ[iTemp])
                    {
                        RECORD *tempR = tableSingleHZ[iTemp];
                        while (!strncmp(tempR->strCode, strCode, strlen(strCode)) &&
                               strlen(tempR->strCode) <= strlen(recTemp->strCode) && tempR != tableDict->s2tCurrentRecord)
                        {
                            if (!strcmp(tempR->strCode, recTemp->strCode) && !strcmp(tempR->strHZ, recTemp->strHZ))
                            {
                                findRepeat = true;
                                break;
                            }
                            tempR = tempR->next;
                        }
                        if (findRepeat)
                            continue;
                        if (!strncmp(tempR->strCode, strCode, strlen(strCode)) &&
                            strlen(tempR->strCode) <= strlen(recTemp->strCode) && tempR == tableDict->s2tCurrentRecord)
                        {
                            tableDict->s2tCurrentRecord->next = recTemp;
                            recTemp->prev = tableDict->s2tCurrentRecord;
                            tableDict->s2tCurrentRecord = tableDict->s2tCurrentRecord->next;
                        }
                        else
                        {
                            tempR->prev->next = recTemp;
                            recTemp->prev = tempR->prev;
                            recTemp->next = tempR;
                            tempR->prev = recTemp;
                        }
                    }
                    else
                    {
                        tableSingleHZ[iTemp] = recTemp;
                        tableDict->s2tCurrentRecord->next = recTemp;
                        recTemp->prev = tableDict->s2tCurrentRecord;
                        tableDict->s2tCurrentRecord = tableDict->s2tCurrentRecord->next;
                    }
                }
            }
        }
        tableDict->s2tCurrentRecord->next = tableDict->s2tRecordHead;
        tableDict->s2tRecordHead->prev = tableDict->s2tCurrentRecord;
    }
    return true;
}

void SaveDict(TableMetaData *tableMetaData, TableDict *tableDict)
{
    RECORD *recTemp;
    char *tempfile, *userfile;
    FILE *fpDict, *fpUser;
    uint32_t iTemp;
    unsigned int i;
    int fd, fd2;
    int8_t cTemp;
    char *path = getFreewbPath();
    if (!tableDict || !tableDict->iTableChanged)
        return;

    // make ~/.config/fcitx/table/ dir
    if (FREE_WUBI == tableDict->tableType)
    {
        fcitx_utils_alloc_cat_str(tempfile, path, "/data/", WUBI_TEMP_FILE);
        fcitx_utils_alloc_cat_str(userfile, path, "/data/", USER_TEMP_FILE);
    }
    else
    {
        fcitx_utils_alloc_cat_str(tempfile, path, "/data/", PINYIN_TEMP_FILE);
    }

    fd = mkstemp(tempfile);
    if (fd == -1)
    {
        printf("创建临时文件[%s]出错\n", tempfile);
        unlink(tempfile);
        free(tempfile);
        free(userfile);
        return;
    }

    fd2 = mkstemp(userfile);
    if (fd2 == -1)
    {
        printf("创建临时文件[%s]出错\n", userfile);
        unlink(tempfile);
        unlink(userfile);
        free(tempfile);
        free(userfile);
        return;
    }

    fpDict = NULL;
    fpUser = NULL;

    fpDict = fdopen(fd, "w");
    fpUser = fdopen(fd2, "w");

    if (!fpDict || !fpUser)
    {
        FcitxLog(ERROR, _("Save dict error"));
        free(tempfile);
        free(userfile);
        return;
    }

    boolean error = false;
    size_t size;
#define CHECK_WRITE_TABLE_ERROR(SIZE)                                                                                            \
    do                                                                                                                           \
    {                                                                                                                            \
        if (size < (SIZE))                                                                                                       \
        {                                                                                                                        \
            error = true;                                                                                                        \
            break;                                                                                                               \
        }                                                                                                                        \
    } while (0)

    // write version number
    do
    {
        // 写入名字
        iTemp = (uint32_t)strlen(tableDict->tableName);
        size = fcitx_utils_write_uint32(fpDict, iTemp);
        CHECK_WRITE_TABLE_ERROR(1);
        size = fwrite(tableDict->tableName, sizeof(char), iTemp + 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(iTemp + 1);
        // 写入词库信息
        iTemp = (uint32_t)strlen(tableDict->tableInfo);
        size = fcitx_utils_write_uint32(fpDict, iTemp);
        CHECK_WRITE_TABLE_ERROR(1);
        size = fwrite(tableDict->tableInfo, sizeof(char), iTemp + 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(iTemp + 1);
        // 写入生成日期
        iTemp = (uint32_t)strlen(tableDict->tableCreateTime);
        size = fcitx_utils_write_uint32(fpDict, iTemp);
        CHECK_WRITE_TABLE_ERROR(1);
        size = fwrite(tableDict->tableCreateTime, sizeof(char), iTemp + 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(iTemp + 1);
        // 写入编码截止键
        iTemp = (uint32_t)strlen(tableDict->strEndKeys);
        size = fcitx_utils_write_uint32(fpDict, iTemp);
        CHECK_WRITE_TABLE_ERROR(1);
        size = fwrite(tableDict->strEndKeys, sizeof(char), iTemp + 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(iTemp + 1);
        // 特殊符号引导符
        iTemp = (uint32_t)strlen(tableDict->strSpecialKeys);
        size = fcitx_utils_write_uint32(fpDict, iTemp);
        CHECK_WRITE_TABLE_ERROR(1);
        size = fwrite(tableDict->strSpecialKeys, sizeof(char), iTemp + 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(iTemp + 1);
        // 写入编码方案类型
        iTemp = (uint32_t)strlen(tableDict->strCodeType);
        size = fcitx_utils_write_uint32(fpDict, iTemp);
        CHECK_WRITE_TABLE_ERROR(1);
        size = fwrite(tableDict->strCodeType, sizeof(char), iTemp + 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(iTemp + 1);
        // 写入径直上屏的标点
        iTemp = (uint32_t)strlen(tableDict->strStraightUPKeys);
        size = fcitx_utils_write_uint32(fpDict, iTemp);
        CHECK_WRITE_TABLE_ERROR(1);
        size = fwrite(tableDict->strStraightUPKeys, sizeof(char), iTemp + 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(iTemp + 1);
        // 写入UsedCodes
        iTemp = strlen(tableDict->strInputCode);
        size = fcitx_utils_write_uint32(fpDict, iTemp);
        CHECK_WRITE_TABLE_ERROR(1);

        size = fwrite(tableDict->strInputCode, sizeof(char), iTemp + 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(iTemp + 1);
        // 写入WildChar
        size = fwrite(&(tableDict->cWildChar), sizeof(unsigned char), 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(1);
        // 写入组词规则
        size = fwrite(&(tableDict->bRule), sizeof(unsigned char), 1, fpDict);
        CHECK_WRITE_TABLE_ERROR(1);
        if (tableDict->bRule)
        { // table contains rule
            for (i = 0; i < tableDict->iCodeLength - 1; i++)
            {
                size = fwrite(&(tableDict->rule[i].iFlag), sizeof(unsigned char), 1, fpDict);
                CHECK_WRITE_TABLE_ERROR(1);
                size = fwrite(&(tableDict->rule[i].iWords), sizeof(unsigned char), 1, fpDict);
                CHECK_WRITE_TABLE_ERROR(1);
                for (iTemp = 0; iTemp < tableDict->iCodeLength; iTemp++)
                {
                    size = fwrite(&(tableDict->rule[i].rule[iTemp].iFlag), sizeof(unsigned char), 1, fpDict);
                    CHECK_WRITE_TABLE_ERROR(1);
                    size = fwrite(&(tableDict->rule[i].rule[iTemp].iWhich), sizeof(unsigned char), 1, fpDict);
                    CHECK_WRITE_TABLE_ERROR(1);
                    size = fwrite(&(tableDict->rule[i].rule[iTemp].iIndex), sizeof(unsigned char), 1, fpDict);
                    CHECK_WRITE_TABLE_ERROR(1);
                }
            }
        }

        if (FREE_WUBI == tableDict->tableType)
        {
            fprintf(fpUser, "[UserWord]\n");
        }

        size = fcitx_utils_write_uint32(fpDict, tableDict->iRecordCount);
        CHECK_WRITE_TABLE_ERROR(1);
        recTemp = tableDict->recordHead->next;
        while (recTemp != tableDict->recordHead)
        {
            iTemp = strlen(recTemp->strCode) + 1;
            size = fcitx_utils_write_uint32(fpDict, iTemp);
            CHECK_WRITE_TABLE_ERROR(1);

            size = fwrite(recTemp->strCode, sizeof(char), iTemp, fpDict);
            CHECK_WRITE_TABLE_ERROR(iTemp);

            iTemp = strlen(recTemp->strHZ) + 1;
            size = fcitx_utils_write_uint32(fpDict, iTemp);
            CHECK_WRITE_TABLE_ERROR(1);
            size = fwrite(recTemp->strHZ, sizeof(char), iTemp, fpDict);
            CHECK_WRITE_TABLE_ERROR(iTemp);

            cTemp = recTemp->type;
            size = fwrite(&cTemp, sizeof(int8_t), 1, fpDict);
            CHECK_WRITE_TABLE_ERROR(1);

            if (FREE_WUBI == tableDict->tableType && cTemp == RECORDTYPE_CONSTRUCT && recTemp->strCode && recTemp->strHZ)
            {
                fprintf(fpUser, "%s=%s\n", recTemp->strCode, recTemp->strHZ);
            }

            recTemp = recTemp->next;
        }
    } while (0);
    if (fclose(fpDict) == EOF || fclose(fpUser) == EOF)
    {
        error = true;
    }
    if (!error)
    {
        char *pstr;
        if (FREE_WUBI == tableDict->tableType)
        {
            fcitx_utils_alloc_cat_str(pstr, path, "/data/mb/", tableMetaData->freeWubiConfig->WubiPath);
        }
        else
        {
            fcitx_utils_alloc_cat_str(pstr, path, "/data/mb/", tableMetaData->freeWubiConfig->PinyinPath);
        }
        if (access(pstr, 0))
            unlink(pstr);
        rename(tempfile, pstr);
        free(pstr);

        if (FREE_WUBI == tableDict->tableType)
        {
            fcitx_utils_alloc_cat_str(pstr, path, "/data/", "user_word.txt");
            if (access(pstr, 0))
                unlink(pstr);
            rename(userfile, pstr);
            free(pstr);
        }
    }

    else
    {
        unlink(tempfile);
        unlink(userfile);
        puts("Write Table dict failed");
    }
    free(tempfile);
    free(userfile);

    FcitxLog(DEBUG, _("Rename OK"));

    tableDict->iTableChanged = 0;
    free(path);
}

void SaveTableDict(TableMetaData *tableMetaData)
{
    SaveDict(tableMetaData, tableMetaData->WubiDict);
    SaveDict(tableMetaData, tableMetaData->PinyinDict);
}

void FreeTableDict(TableMetaData *tableMetaData)
{
    TableDict *tableDict;
    if (tableMetaData->WubiDict)
    {
        tableDict = tableMetaData->WubiDict;

        if (!tableDict->recordHead)
            return;

        if (tableDict->iTableChanged)
            SaveDict(tableMetaData, tableDict);

        fcitx_memory_pool_destroy(tableDict->pool);
        free(tableDict);
        tableMetaData->WubiDict = NULL;
    }
    if (tableMetaData->PinyinDict)
    {
        tableDict = tableMetaData->PinyinDict;

        if (!tableDict->recordHead)
            return;

        if (tableDict->iTableChanged)
            SaveDict(tableMetaData, tableDict);

        fcitx_memory_pool_destroy(tableDict->pool);
        free(tableDict);
        tableMetaData->PinyinDict = NULL;
    }
}

// /*
//  *根据字串判断词库中是否有某个字/词，注意该函数会忽略拼音词组
//  */
RECORD *TableFindPhrase(const TableDict *tableDict, const char *strHZ)
{
    RECORD *recTemp;
    char strTemp[UTF8_MAX_LENGTH + 1];
    int i;

    // 首先，先查找第一个汉字的编码
    strncpy(strTemp, strHZ, fcitx_utf8_char_len(strHZ));
    strTemp[fcitx_utf8_char_len(strHZ)] = '\0';

    recTemp = tableDict->tableSingleHZ[CalHZIndex(strTemp)];
    if (!recTemp)
        return (RECORD *)NULL;

    // 然后根据该编码找到检索的起始点
    i = 0;
    while (recTemp->strCode[0] != tableDict->recordIndex[i].cCode)
        i++;

    recTemp = tableDict->recordIndex[i].record;
    while (recTemp != tableDict->recordHead)
    {
        if (recTemp->strCode[0] != tableDict->recordIndex[i].cCode)
            break;
        if (!strcmp(recTemp->strHZ, strHZ))
        {
            return recTemp;
        }

        recTemp = recTemp->next;
    }

    return (RECORD *)NULL;
}

RECORD *TableFindPhraseByCode(const TableDict *tableDict, const char *strCode, const char *strHZ)
{
    RECORD *recTemp;
    int i;
    i = 0;
    while (tableDict->recordIndex[i].record && strCode[0] != tableDict->recordIndex[i].cCode)
        i++;

    recTemp = tableDict->recordIndex[i].record;
    while (recTemp != tableDict->recordHead)
    {
        if (recTemp->strCode[0] != strCode[0])
            break;
        if (!strcmp(recTemp->strHZ, strHZ) && !strcmp(recTemp->strCode, strCode))
        {
            return recTemp;
        }

        recTemp = recTemp->next;
    }

    return (RECORD *)NULL;
}

boolean TableCalPhraseCode(TableDict *tableDict, char *strHZ, char *strCode)
{
    unsigned char i;
    unsigned char i1, i2;
    size_t iLen;
    char strTemp[UTF8_MAX_LENGTH + 1] = {
        '\0',
    };
    RECORD *recTemp;
    boolean bFindCode = true;

    iLen = fcitx_utf8_strlen(strHZ);
    if (iLen >= tableDict->iCodeLength)
    {
        i2 = tableDict->iCodeLength;
        i1 = 1;
    }
    else
    {
        i2 = iLen;
        i1 = 0;
    }

    for (i = 0; i < tableDict->iCodeLength - 1; i++)
    {
        if (tableDict->rule[i].iWords == i2 && tableDict->rule[i].iFlag == i1)
            break;
    }
    if (i == tableDict->iCodeLength - 1)
        return false;
    int codeIdx = 0;
    for (i1 = 0; i1 < tableDict->iCodeLength; i1++)
    {
        int clen;
        char *ps;
        if (tableDict->rule[i].rule[i1].iFlag)
        {
            ps = fcitx_utf8_get_nth_char(strHZ, tableDict->rule[i].rule[i1].iWhich - 1);
            clen = fcitx_utf8_char_len(ps);
            strncpy(strTemp, ps, clen);
        }
        else
        {
            ps = fcitx_utf8_get_nth_char(strHZ, iLen - tableDict->rule[i].rule[i1].iWhich);
            clen = fcitx_utf8_char_len(ps);
            strncpy(strTemp, ps, clen);
        }

        int hzIndex = CalHZIndex(strTemp);

        if (tableDict->tableSingleHZ[hzIndex])
        {
            recTemp = tableDict->tableSingleHZ[hzIndex];
        }
        else
        {
            bFindCode = false;
            break;
        }

        if (strlen(recTemp->strCode) >= tableDict->rule[i].rule[i1].iIndex)
        {
            strCode[codeIdx] = recTemp->strCode[tableDict->rule[i].rule[i1].iIndex - 1];
            codeIdx++;
        }
    }

    return bFindCode;
}

// /*
//  *判断某个词组是不是已经在词库中,有返回NULL，无返回插入点
//  */
// RECORD         *TableHasPhrase(const TableDict* tableDict, const char *strCode, const char *strHZ)
// {
//     RECORD         *recTemp;
//     int             i;
//
//     i = 0;
//     while (strCode[0] != tableDict->recordIndex[i].cCode)
//         i++;
//
//     recTemp = tableDict->recordIndex[i].record;
//     while (recTemp != tableDict->recordHead) {
//         if (recTemp->type != RECORDTYPE_PINYIN) {
//             if (strcmp(recTemp->strCode, strCode) > 0)
//                 break;
//             else if (!strcmp(recTemp->strCode, strCode)) {
//                 if (!strcmp(recTemp->strHZ, strHZ))     //该词组已经在词库中
//                     return NULL;
//             }
//         }
//         recTemp = recTemp->next;
//     }
//
//     return recTemp;
// }

// void TableInsertPhrase(TableDict* tableDict, const char *strCode, const char *strHZ)
// {
//     RECORD         *insertPoint, *dictNew;
//
//     insertPoint = TableHasPhrase(tableDict, strCode, strHZ);
//
//     if (!insertPoint)
//         return;
//
//     dictNew = (RECORD*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
//     dictNew->strCode = (char*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(char) * (tableDict->iCodeLength + 1));
//     dictNew->type = RECORDTYPE_NORMAL;
//     strcpy(dictNew->strCode, strCode);
//     dictNew->strHZ = (char*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(char) * (strlen(strHZ) + 1));
//     strcpy(dictNew->strHZ, strHZ);
//     dictNew->iHit = 0;
//     dictNew->iIndex = tableDict->iTableIndex;
//
//     dictNew->prev = insertPoint->prev;
//     insertPoint->prev->next = dictNew;
//     insertPoint->prev = dictNew;
//     dictNew->next = insertPoint;
//
//     tableDict->iRecordCount++;
//     tableDict->iTableChanged++;
// }

// /*
//  * 根据字串删除词组
//  */
// void TableDelPhraseByHZ(TableDict* tableDict, const char *strHZ)
// {
//     RECORD         *recTemp;
//
//     recTemp = TableFindPhrase(tableDict, strHZ);
//     if (recTemp)
//         TableDelPhrase(tableDict, recTemp);
// }

// void TableDelPhrase(TableDict* tableDict, RECORD * record)
// {
//     record->prev->next = record->next;
//     record->next->prev = record->prev;
//
//     /*
//      * since we use memory pool, don't free record
//      * though free list is currently not supported, but it's ok
//      * people will not delete phrase so many times
//      */
//
//     tableDict->iRecordCount--;
//     tableDict->iTableChanged++;
// }

// void TableUpdateHitFrequency(TableMetaData* tableMetaData, RECORD * record)
// {
//         TableDict *dict;
//         if(record->owner->tableType==FREE_WUBI)
//             dict = tableMetaData->WubiDict;
//         else
//             dict = tableMetaData->PinyinDict;
//         dict->iTableChanged++;
//         record->iHit++;
//         record->iIndex = ++dict->iTableIndex;
// }

int TableCompareCode(const char *strUser, const char *strDict, boolean exactMatch)
{
    int i;
    size_t tmp_len = strlen(strUser);
    for (i = 0; i < tmp_len; i++)
    {
        if (!strDict[i])
            return strUser[i];
        if (strUser[i] != strDict[i])
            return (strUser[i] - strDict[i]);
    }
    if (exactMatch)
    {
        if (strlen(strUser) != strlen(strDict))
            return -999; // 随意的一个值
    }

    return 0;
}

int TableFindFirstMatchCode(TableMetaData *tableMetaData, const char *strCodeInput, boolean exactMatch,
                            boolean cacheCurrentRecord)
{
    int i = 0;
    TableDict *WubiDict, *PinyinDict;
    int findMatch = -1;
    boolean isTempPinyin = false;
    if (tableMetaData->freeWubiConfig->unCommonKey[0].sym == strCodeInput[0])
    {
        strCodeInput++;
        if (tableMetaData->tableType == FREE_WUBI)
            isTempPinyin = true;
    }
    do
    {
        if (tableMetaData->tableType != FREE_PINYIN)
        {
            WubiDict = tableMetaData->WubiDict;
            if (!WubiDict->recordHead)
                break;
            while (strCodeInput[0] != WubiDict->recordIndex[i].cCode)
            {
                if (!WubiDict->recordIndex[i].cCode)
                    break;
                i++;
            }
            RECORD *record = NULL;
            RECORD **pRecord = &record;
            if (cacheCurrentRecord)
                pRecord = &WubiDict->currentRecord;

            *pRecord = WubiDict->recordIndex[i].record;
            if (!*pRecord)
                break;
            while (*pRecord != WubiDict->recordHead)
            {
                if (!TableCompareCode(strCodeInput, (*pRecord)->strCode, exactMatch))
                {
                    findMatch++;
                    break;
                }
                (*pRecord) = (*pRecord)->next;
                i++;
            }
        }
    } while (0);
    do
    {
        if (tableMetaData->tableType != FREE_WUBI || isTempPinyin)
        {
            i = 0;
            PinyinDict = tableMetaData->PinyinDict;
            if (!PinyinDict->recordHead)
                break;
            while (strCodeInput[0] != PinyinDict->recordIndex[i].cCode)
            {
                if (!PinyinDict->recordIndex[i].cCode)
                    break;
                i++;
            }
            RECORD *record = NULL;
            RECORD **pRecord = &record;
            if (cacheCurrentRecord)
                pRecord = &PinyinDict->currentRecord;

            *pRecord = PinyinDict->recordIndex[i].record;
            if (!*pRecord)
                break;
            while (*pRecord != PinyinDict->recordHead)
            {
                if (!TableCompareCode(strCodeInput, (*pRecord)->strCode, exactMatch))
                {
                    findMatch++;
                    break;
                }
                (*pRecord) = (*pRecord)->next;
                i++;
            }
        }
    } while (0);
    if (tableMetaData->autoPhrase)
    {
        tableMetaData->currentPhrase = tableMetaData->autoPhrase->next;
        while (tableMetaData->currentPhrase)
        {
            if (!TableCompareCode(strCodeInput, tableMetaData->currentPhrase->strCode, 1))
            {
                findMatch++;
                break;
            }
            tableMetaData->currentPhrase = tableMetaData->currentPhrase->next;
        }
    }

    return findMatch; // Not found
}

boolean IsInputKey(const TableMetaData *tableMetaData, int iKey)
{
    if (iKey >= 'a' && iKey <= 'z')
        return true;
    char *p = tableMetaData->WubiDict->strInputCode;
    if (!p)
        return false;
    while (*p)
    {
        if (iKey != '/' && iKey == *p)
            return true;
        p++;
    }
    return false;
}

boolean IsUncommonKey(const TableMetaData *tableMetaData, int iKey, int state)
{
    int p = tableMetaData->freeWubiConfig->unCommonKey[0].sym;
    if (p == iKey && state == FcitxKeyState_None)
        return true;
    return false;
}

static int cmpi(const void *a, const void *b)
{
    return (*((int *)a)) - (*((int *)b));
}

unsigned int CalHZIndex(char *strHZ)
{
    unsigned int iutf = 0;
    int l = fcitx_utf8_char_len(strHZ);
    unsigned char *utf = (unsigned char *)strHZ;
    unsigned int *res;
    int idx;

    if (l == 2)
    {
        iutf = *utf++ << 8;
        iutf |= *utf++;
    }
    else if (l == 3)
    {
        iutf = *utf++ << 16;
        iutf |= *utf++ << 8;
        iutf |= *utf++;
    }
    else if (l == 4)
    {
        iutf = *utf++ << 24;

        iutf |= *utf++ << 16;
        iutf |= *utf++ << 8;
        iutf |= *utf++;
    }

    res = bsearch(&iutf, fcitx_utf8_in_gb18030, 63360, sizeof(int), cmpi);
    if (res)
        idx = res - fcitx_utf8_in_gb18030;
    else
        idx = 63361;
    return idx;
}

void adjustOrder(TableMetaData *tableMetaData, TableDict *dict, char *wordText, char *wordCode)
{
    int i = 0;
    boolean findRepeat = false;
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
    if (strcmp(wordCode, record->strCode) == 0 && strcmp(wordText, record->strHZ) == 0)
        return;
    RECORD *insertPoint = record;
    RECORD *recTemp = NULL;
    while (strcmp(wordCode, record->strCode) == 0)
    {
        if (strcmp(wordText, record->strHZ) == 0)
        {
            recTemp = record;
            findRepeat = true;
            record->prev->next = record->next;
            record->next->prev = record->prev;
            /*            if(dict->recordIndex[i].record==record)
                            dict->recordIndex[i].record = record->next; */
            dict->iTableChanged = 1;
            dict->iRecordCount--;
            break;
        }
        record = record->next;
    }
    if (!findRepeat)
    {
        recTemp = (RECORD *)fcitx_memory_pool_alloc(dict->pool, sizeof(RECORD));
        recTemp->owner = dict;
        recTemp->type = RECORDTYPE_CONSTRUCT;
        recTemp->strCode = (char *)fcitx_memory_pool_alloc(dict->pool, strlen(wordCode) + 1);
        memset(recTemp->strCode, 0, strlen(wordCode) + 1);
        strcpy(recTemp->strCode, wordCode);
        recTemp->strHZ = (char *)fcitx_memory_pool_alloc(dict->pool, strlen(wordText) + 1);
        memset(recTemp->strHZ, 0, strlen(wordText) + 1);
        strcpy(recTemp->strHZ, wordText);

        //         recTemp->iHit = 0;
    }
    recTemp->prev = insertPoint->prev;
    insertPoint->prev->next = recTemp;
    recTemp->next = insertPoint;
    insertPoint->prev = recTemp;
    if (dict->recordIndex[i].record == insertPoint)
        dict->recordIndex[i].record = recTemp;
    dict->iTableChanged = 1;
    dict->iRecordCount++;
    if (!findRepeat)
    {
        addWordPhraseAndSaveToDict(tableMetaData, dict, 1, wordText, wordCode);
    }
    SaveTableDict(tableMetaData);
}

boolean LoadQuickTable(TableMetaData *tableMetaData)
{
    char *tablepath;
    char *path = getFreewbPath();
    FILE *fpQuickDict = NULL;
    size_t len;
    fcitx_utils_alloc_cat_str(tablepath, path, "/data/", QUICK_PATH);
    fpQuickDict = fopen(tablepath, "r");
    free(path);
    free(tablepath);
    if (!fpQuickDict)
        return false;
    else
    {
        QUCIK_TABLE *quickTable = tableMetaData->quickTable;
        char *data = NULL;
        char *temp = NULL;
        while (1)
        {
            //             memset(data,0,256);
            //             fscanf(fpQuickDict,"%s",data);
            if (getline(&data, &len, fpQuickDict) == -1)
                break;
            if (!data[0] || data[0] == '#')
                continue;
            temp = data;
            while (*temp)
            {
                if (*temp == '=')
                {
                    *temp++ = '\0';
                    break;
                }
                temp++;
            }
            char *ttemp = temp;
            while (*ttemp)
            {
                if (*ttemp == '\n')
                    *ttemp = '\0';
                ttemp++;
            }
            if (!*temp || strlen(data) != 1)
                continue;
            if (!quickTable->next)
                quickTable->next = fcitx_utils_new(QUCIK_TABLE);
            quickTable = quickTable->next;
            quickTable->key = *data;
            strcpy(quickTable->value, temp);
            quickTable->next = NULL;
            //             free(temp);
            //             free(data);
        }
    }

    return true;
}

void freeQucikTable(TableMetaData *tableMetaData)
{
    if (!tableMetaData->quickTable)
        return;
    QUCIK_TABLE *quickTable = tableMetaData->quickTable->next;
    while (quickTable)
    {
        QUCIK_TABLE *temp = quickTable;
        quickTable = quickTable->next;
        free(temp);
    }
    tableMetaData->quickTable->next = NULL;
}

boolean LoadAutoEng(TableMetaData *tableMetaData, const char *strAutoEng)
{
    freeAutoEng(tableMetaData);
    if (strAutoEng)
    {
        const char *data = strAutoEng;
        AUTO_ENG *autoEng = tableMetaData->autoEng;
        while (*data == ' ' || *data == '\t')
            data++;
        const char *data2 = data;
        int i = 0;
        while (*data && *data != '\n')
        {
            if (*data == ' ' || *data == '\t')
            {
                if (!autoEng->next)
                    autoEng->next = fcitx_utils_new(AUTO_ENG);
                autoEng = autoEng->next;
                autoEng->next = NULL;
                memset(autoEng->value, 0, 10);
                strncpy(autoEng->value, data2, i);
                i = 0;
                data++;
                while (*data == ' ' || *data == '\t')
                    data++;
                data2 = data;
            }
            else
            {
                i++;
                data++;
            }
        }
        if (data2 != data)
        {
            if (!autoEng->next)
                autoEng->next = fcitx_utils_new(AUTO_ENG);
            autoEng = autoEng->next;
            autoEng->next = NULL;
            memset(autoEng->value, 0, 10);
            strncpy(autoEng->value, data2, i);
        }
    }
    return true;
}

void freeAutoEng(TableMetaData *tableMetaData)
{
    if (!tableMetaData->autoEng)
        return;
    AUTO_ENG *autoEng = tableMetaData->autoEng->next;
    while (autoEng)
    {
        AUTO_ENG *temp = autoEng;
        autoEng = autoEng->next;
        free(temp);
    }
    tableMetaData->autoEng->next = NULL;
}

boolean isAutoEngStr(TableMetaData *tableMetaData, char *str)
{
    if (!tableMetaData->autoEng)
        return false;
    AUTO_ENG *autoEng = tableMetaData->autoEng->next;
    while (autoEng)
    {
        if (!strcmp(autoEng->value, str))
            return true;
        autoEng = autoEng->next;
    }
    return false;
}

QUCIK_TABLE *findQuickPharse(TableMetaData *tableMetaData, FcitxKeySym sym)
{
    if (!tableMetaData->quickTable)
        return NULL;
    QUCIK_TABLE *quickTable = tableMetaData->quickTable->next;
    while (quickTable)
    {
        if (quickTable->key == sym)
        {
            return quickTable;
        }
        quickTable = quickTable->next;
    }
    return NULL;
}

char *s2tConvers(TableMetaData *tableMetaData, const char *simpel)
{
    if (!simpel || !*simpel)
        return NULL;
    char *tradition = malloc(strlen(simpel) + 10);
    memset(tradition, 0, strlen(simpel) + 10);
    char simpelHZ[6] = {0};

    while (*simpel)
    {
        int len = fcitx_utf8_char_len(simpel);
        memset(simpelHZ, 0, 6);
        strncpy(simpelHZ, simpel, len);
        RECORD *recTemp = tableMetaData->WubiDict->s2tSignleHZ[CalHZIndex(simpelHZ)];
        if (recTemp && !strcmp(simpelHZ, recTemp->strCode))
            strcat(tradition, recTemp->strHZ);
        else
            strcat(tradition, simpelHZ);
        simpel += len;
    }
    return tradition;
}

void addWordPhraseAndSaveToDict(TableMetaData *tableMetaData, TableDict *dict, int flg, char *wordText, char *wordCode)
{
    if (flg == 1 && dict)
    {
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
        SaveTableDict(tableMetaData);
    }
}

void deleteWordPhraseAndSaveToDict(TableMetaData *tableMetaData, TableDict *dict, int flg, char *wordText, char *wordCode)
{
    if (flg == 1 && dict)
    {
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
        SaveTableDict(tableMetaData);
    }
}
