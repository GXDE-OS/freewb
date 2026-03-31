#include "freewbConversionTool.h"

#include "../inputmethod/freedict.h"
#define CHECK_OPTION(str, x) (strstr((str), strConst[x]) == (str))

#define STR_DESCRIPTION 0
#define STR_NAME 1
#define STR_INFORMATION 2
#define STR_TIME 3
#define STR_FINISH 4
#define STR_SPECIAL_SYMBOL 5
#define STR_CODETYPE 6
#define STR_STRAIGHT_UP 7
#define STR_KEYCODE 8
#define STR_WILDCHAR 9
#define STR_RULE 10
#define STR_UNCOMMONWORDS 11
#define STR_CONSTRUCTPHRASE 12
#define STR_REMIND 13
#define STR_TEXT 14
// #define STR_PROMPT 7

#define CONST_STR_SIZE 15

#define MAX_CODE_LENGTH 30
// #define FH_MAX_LENGTH  10
// #define TABLE_AUTO_SAVE_AFTER 1024
// #define AUTO_PHRASE_COUNT 10000
// #define SINGLE_HZ_COUNT 66000

char *strConst[CONST_STR_SIZE] = {"[Description]", "Name=", "词库信息=", "生成日期:", "编码截止键=", "特殊符号引导符=", "编码方案类型=", "径直上屏的标点=", "UsedCodes=", "WildChar=", "[rule]", ":生僻字词", ":用户词组", "联想词组", "[Text]"};

char strName[30] = "五笔字型";
char strInforMation[100] = {0};
char strTime[20] = "2019-07-15 09:00";
char strEndkey[5] = {0};
char strSpecial[5] = {0};
char strCodeType[5] = {0};
char strStraightUp[10] = {0};
char strInputCode[100] = "\0";
char cWildChar = 'z';
// char cPromptKey = '&';
char cPhraseKey = '^';
char cUncommonWords = '~';
char cRemindWords = '!';

int txt2mb(char *txtPath, char *mbPath, int *HZcount)
{
    if (!txtPath || !mbPath || !HZcount)
    {
        printf("param erro!\n");
        fflush(stdout);
        return 0;
    }
    FILE *fpTxt;
    RECORD *temp, *head, *newRec, *current;
    char *buf = NULL, *buf1 = NULL;
    size_t len;
    *HZcount = 0;
    int i;
    int bRule = 0;
    RULE *rule = NULL;
    char *pstr = 0;
    char strTemp[10];
    int8_t type;
    uint32_t total = 0;
    fpTxt = fopen(txtPath, "rt");
    if (!fpTxt)
    {
        printf("Cannot open source file %s.\n", txtPath);
        fflush(stdout);
        return 0;
    }
    while (1)
    {
        if (getline(&buf, &len, fpTxt) == -1)
            break;
        i = strlen(buf) - 1;
        while ((i >= 0) && (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\r'))
            buf[i--] = '\0';
        pstr = buf;
        //         if (*pstr == ' ')
        //             pstr++;
        if (pstr[0] == '#')
            continue;
        if (strncmp(pstr, "---", 3) == 0 || strncmp(pstr, "★★★", 3) == 0)
            continue;
        if (CHECK_OPTION(pstr, STR_DESCRIPTION))
            continue;
        else if (CHECK_OPTION(pstr, STR_NAME))
        {
            pstr += strlen(strConst[STR_NAME]);
            strcpy(strName, pstr);
        }
        else if (CHECK_OPTION(pstr, STR_INFORMATION))
        {
            pstr += strlen(strConst[STR_INFORMATION]);
            strcpy(strInforMation, pstr);
        }
        else if (CHECK_OPTION(pstr, STR_TIME))
        {
            pstr += strlen(strConst[STR_TIME]);
            strcpy(strTime, pstr);
        }
        else if (CHECK_OPTION(pstr, STR_FINISH))
        {
            pstr += strlen(strConst[STR_FINISH]);
            strcpy(strEndkey, pstr);
        }
        else if (CHECK_OPTION(pstr, STR_SPECIAL_SYMBOL))
        {
            pstr += strlen(strConst[STR_SPECIAL_SYMBOL]);
            strcpy(strSpecial, pstr);
        }
        else if (CHECK_OPTION(pstr, STR_CODETYPE))
        {
            pstr += strlen(strConst[STR_CODETYPE]);
            strcpy(strCodeType, pstr);
        }
        else if (CHECK_OPTION(pstr, STR_STRAIGHT_UP))
        {
            pstr += strlen(strConst[STR_STRAIGHT_UP]);
            strcpy(strStraightUp, pstr);
        }
        else if (CHECK_OPTION(pstr, STR_KEYCODE))
        {
            pstr += strlen(strConst[STR_KEYCODE]);
            strcpy(strInputCode, pstr);
        }
        else if (CHECK_OPTION(pstr, STR_WILDCHAR))
        {
            pstr += strlen(strConst[STR_WILDCHAR]);
            cWildChar = *pstr;
        }
        else if (CHECK_OPTION(pstr, STR_UNCOMMONWORDS))
        {
            cUncommonWords = *pstr;
        }
        else if (CHECK_OPTION(pstr, STR_CONSTRUCTPHRASE))
        {
            cPhraseKey = *pstr;
        }
        else if (CHECK_OPTION(pstr, STR_REMIND))
        {
            cRemindWords = *pstr;
        }
        else if (CHECK_OPTION(pstr, STR_RULE))
        {
            bRule = 1;
        }
        else if (CHECK_OPTION(pstr, STR_TEXT))
        {
            break;
        }
        else
            continue;
    }
    if (!strInputCode[0])
    {
        printf("Source File Format Error!\n");
        fflush(stdout);
        return 0;
        ;
    }
    if (!CHECK_OPTION(pstr, STR_TEXT))
    {
        printf("Source File Format Error!\n");
        fflush(stdout);
        printf("%s\n", pstr);
        return (0);
    }
    if (bRule)
    {
        char *strRules[3] = {"e2=p11+p12+p21+p22", "e3=p11+p21+p31+p32", "a4=p11+p21+p31+n11"};
        int iCodeLength = 4;
        int iTemp = 0;
        rule = (RULE *)malloc(sizeof(RULE) * (iCodeLength - 1));

        for (iTemp = 0; iTemp < (iCodeLength - 1); iTemp++)
        {

            rule[iTemp].rule = (RULE_RULE *)malloc(sizeof(RULE_RULE) * iCodeLength);
            buf = strRules[iTemp];
            i = strlen(buf) - 1;

            while ((i >= 0) && (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\r'))
                buf[i--] = '\0';
            pstr = buf;
            switch (*pstr)
            {

            case 'e':

            case 'E':
                rule[iTemp].iFlag = 0;
                break;

            case 'a':

            case 'A':
                rule[iTemp].iFlag = 1;
                break;

            default:
                printf("2   Phrase rules are not suitable!\n");
                printf("\t\t%s\n", buf);
                fflush(stdout);
                return 0;
            }

            pstr++;

            char *p = pstr;

            while (*p && *p != '=')
                p++;

            if (!(*p))
            {
                printf("3   Phrase rules are not suitable!\n");
                printf("\t\t%s\n", buf);
                fflush(stdout);
                return 0;
            }

            strncpy(strTemp, pstr, p - pstr);

            strTemp[p - pstr] = '\0';
            rule[iTemp].iWords = atoi(strTemp);

            p++;

            for (i = 0; i < iCodeLength; i++)
            {
                while (*p == ' ')
                    p++;

                switch (*p)
                {

                case 'p':

                case 'P':
                    rule[iTemp].rule[i].iFlag = 1;
                    break;

                case 'n':

                case 'N':
                    rule[iTemp].rule[i].iFlag = 0;
                    break;

                default:
                    printf("4   Phrase rules are not suitable!\n");
                    printf("\t\t%s\n", buf);
                    fflush(stdout);
                    return 0;
                }

                p++;

                rule[iTemp].rule[i].iWhich = *p++ - '0';
                rule[iTemp].rule[i].iIndex = *p++ - '0';

                while (*p == ' ')
                    p++;

                if (i != (iCodeLength - 1))
                {
                    if (*p != '+')
                    {
                        printf("5   Phrase rules are not suitable!\n");
                        printf("\t\t%s  %d\n", buf, iCodeLength);
                        exit(1);
                    }

                    p++;
                }
            }
        }
    }
    head = (RECORD *)malloc(sizeof(RECORD));
    head->next = head;
    head->prev = head;
    current = head;
    buf = NULL;
    while (getline(&buf, &len, fpTxt) != -1)
    {
        if (buf1)
            free(buf1);
        buf1 = fcitx_utils_trim(buf);
        char *p = buf1;

        while (*p && !isspace(*p))
            p++;

        if (*p == '\0')
            continue;

        while (*p == ' ' || *p == '\t')
        {
            *p = '\0';
            p++;
        }
        pstr = buf1;
        while (*p && *p != '\n')
        {
            char *strHZ = p;
            while (!isspace(*p) && *p)
                p++;
            while (*p == ' ' || *p == '\t')
            {
                *p = '\0';
                p++;
            }
            type = RECORDTYPE_NORMAL;

            if (strHZ[0] == cUncommonWords)
            {
                strHZ++;
                type = RECORDTYPE_UNCOMMON;
            }
            else if (strHZ[0] == cPhraseKey)
            {
                strHZ++;
                type = RECORDTYPE_CONSTRUCT;
            }
            else if (strHZ[0] == cRemindWords)
            {
                strHZ++;
                //             type =
            }

            // 查找是否重复
            temp = current;

            if (temp != head)
            {
                if (strcmp(temp->strCode, pstr) >= 0)
                {
                    while (temp != head && strcmp(temp->strCode, pstr) >= 0)
                    {
                        if (!strcmp(temp->strHZ, strHZ) && !strcmp(temp->strCode, pstr))
                        {
                            printf("Delete:  %s %s\n", pstr, strHZ);
                            goto _next;
                        }

                        temp = temp->prev;
                    }

                    if (temp == head)
                        temp = temp->next;

                    while (temp != head && strcmp(temp->strCode, pstr) <= 0)
                        temp = temp->next;
                }
                else
                {
                    while (temp != head && strcmp(temp->strCode, pstr) <= 0)
                    {
                        if (!strcmp(temp->strHZ, strHZ) && !strcmp(temp->strCode, pstr))
                        {
                            printf("Delete:  %s %s\n", pstr, strHZ);
                            goto _next;
                        }

                        temp = temp->next;
                    }
                }
            }

            // 插在temp的前面
            newRec = (RECORD *)fcitx_utils_malloc0(sizeof(RECORD));

            newRec->strCode = (char *)fcitx_utils_malloc0(sizeof(char) * (strlen(pstr) + 1));

            newRec->strHZ = (char *)fcitx_utils_malloc0(sizeof(char) * (strlen(strHZ) + 1));

            strcpy(newRec->strCode, pstr);

            strcpy(newRec->strHZ, strHZ);

            newRec->type = type;

            temp->prev->next = newRec;

            newRec->next = temp;

            newRec->prev = temp->prev;

            temp->prev = newRec;

            current = newRec;
            total++;
        _next:
            continue;
        }
    }
    if (buf)
        free(buf);
    if (buf1)
        free(buf1);

    fclose(fpTxt);

    FILE *fpMB = fopen(mbPath, "w");
    int iTemp;
    if (!fpMB)
    {
        printf("\nCannot create target file!\n\n");
        fflush(stdout);
        return 0;
    }
    // 写入名字
    iTemp = (uint32_t)strlen(strName);
    fcitx_utils_write_uint32(fpMB, iTemp);
    fwrite(strName, sizeof(char), iTemp + 1, fpMB);
    // 写入词库信息
    iTemp = (uint32_t)strlen(strInforMation);
    fcitx_utils_write_uint32(fpMB, iTemp);
    fwrite(strInforMation, sizeof(char), iTemp + 1, fpMB);
    // 写入生成日期
    iTemp = (uint32_t)strlen(strTime);
    fcitx_utils_write_uint32(fpMB, iTemp);
    fwrite(strTime, sizeof(char), iTemp + 1, fpMB);
    // 写入编码截止键
    iTemp = (uint32_t)strlen(strEndkey);
    fcitx_utils_write_uint32(fpMB, iTemp);
    fwrite(strEndkey, sizeof(char), iTemp + 1, fpMB);
    // 特殊符号引导符
    iTemp = (uint32_t)strlen(strSpecial);
    fcitx_utils_write_uint32(fpMB, iTemp);
    fwrite(strSpecial, sizeof(char), iTemp + 1, fpMB);
    // 写入编码方案类型
    iTemp = (uint32_t)strlen(strCodeType);
    fcitx_utils_write_uint32(fpMB, iTemp);
    fwrite(strCodeType, sizeof(char), iTemp + 1, fpMB);
    // 写入径直上屏的标点
    iTemp = (uint32_t)strlen(strStraightUp);
    fcitx_utils_write_uint32(fpMB, iTemp);
    fwrite(strStraightUp, sizeof(char), iTemp + 1, fpMB);
    // 写入UsedCodes
    iTemp = (uint32_t)strlen(strInputCode);
    fcitx_utils_write_uint32(fpMB, iTemp);
    fwrite(strInputCode, sizeof(char), iTemp + 1, fpMB);
    // 写入WildChar
    fwrite(&cWildChar, sizeof(unsigned char), 1, fpMB);
    // 写入组词规则
    fwrite(&bRule, sizeof(unsigned char), 1, fpMB);

    if (bRule)
    {
        int iCodeLength = 4;
        for (i = 0; i < iCodeLength - 1; i++)
        {
            fwrite(&(rule[i].iFlag), sizeof(unsigned char), 1, fpMB);
            fwrite(&(rule[i].iWords), sizeof(unsigned char), 1, fpMB);

            for (iTemp = 0; iTemp < iCodeLength; iTemp++)
            {
                fwrite(&(rule[i].rule[iTemp].iFlag), sizeof(unsigned char), 1, fpMB);
                fwrite(&(rule[i].rule[iTemp].iWhich), sizeof(unsigned char), 1, fpMB);
                fwrite(&(rule[i].rule[iTemp].iIndex), sizeof(unsigned char), 1, fpMB);
            }
        }
    }

    fcitx_utils_write_uint32(fpMB, total);
    current = head->next;

    while (current != head)
    {
        // printf("%s ,\t%s\n",current->strCode,current->strHZ);
        iTemp = strlen(current->strCode) + 1;
        fcitx_utils_write_uint32(fpMB, iTemp);
        fwrite(current->strCode, sizeof(char), iTemp, fpMB);
        iTemp = strlen(current->strHZ) + 1;
        fcitx_utils_write_uint32(fpMB, iTemp);
        fwrite(current->strHZ, sizeof(char), iTemp, fpMB);
        fwrite(&(current->type), sizeof(int8_t), 1, fpMB);
        current = current->next;
        *HZcount = (*HZcount) + 1;
    }

    fclose(fpMB);
    return 1;
}

int mb2txt(char *txtPath, char *mbPath, int *HZcount)
{
    if (!txtPath || !mbPath || !HZcount)
    {
        printf("param erro!\n");
        fflush(stdout);
        return 0;
    }
    FILE *fpMB;
    RECORD *head, *current;
    *HZcount = 0;
    int i;
    uint32_t bRule = 0;
    RULE *rule = NULL;
    char *strCode = NULL;
    char *strHZ = NULL;
    int8_t type;
    uint32_t total = 0;
    uint32_t iTemp = 0;
    fpMB = fopen(mbPath, "rt");
    if (!fpMB)
    {
        printf("Cannot open source file %s.\n", mbPath);
        fflush(stdout);
        return 0;
    }
#define CHECK_LOAD_TABLE_ERROR(SIZE)                                                                                                                                                                                                                                                                       \
    if (size < (SIZE))                                                                                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                                                                      \
        printf("码表格式错误");                                                                                                                                                                                                                                                                            \
        fflush(stdout);                                                                                                                                                                                                                                                                                    \
        return 0;                                                                                                                                                                                                                                                                                          \
    }
    size_t size;
    // 读取名字
    size = fcitx_utils_read_uint32(fpMB, &iTemp);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(strName, sizeof(char), iTemp + 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(iTemp + 1);
    // 读取词库信息
    size = fcitx_utils_read_uint32(fpMB, &iTemp);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(strInforMation, sizeof(char), iTemp + 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(iTemp + 1);
    // 读取生成日期
    size = fcitx_utils_read_uint32(fpMB, &iTemp);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(strTime, sizeof(char), iTemp + 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(iTemp + 1);
    // 读取编码截止键
    size = fcitx_utils_read_uint32(fpMB, &iTemp);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(strEndkey, sizeof(char), iTemp + 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(iTemp + 1);
    // 读取特殊符号引导符
    size = fcitx_utils_read_uint32(fpMB, &iTemp);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(strSpecial, sizeof(char), iTemp + 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(iTemp + 1);
    // 读取编码方案类型
    size = fcitx_utils_read_uint32(fpMB, &iTemp);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(strCodeType, sizeof(char), iTemp + 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(iTemp + 1);
    // 读取径直上屏的标点
    size = fcitx_utils_read_uint32(fpMB, &iTemp);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(strStraightUp, sizeof(char), iTemp + 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(iTemp + 1);
    // 读取UsedCodes
    size = fcitx_utils_read_uint32(fpMB, &iTemp);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(strInputCode, sizeof(char), iTemp + 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(iTemp + 1);
    // 读取WildChar
    size = fread(&cWildChar, sizeof(uint8_t), 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(1);
    // 读取组词规则
    size = fread(&bRule, sizeof(uint8_t), 1, fpMB);
    CHECK_LOAD_TABLE_ERROR(1);
    if (bRule)
    {
        int iCodeLength = 4;
        rule = (RULE *)malloc(sizeof(RULE) * (iCodeLength - 1));
        for (i = 0; i < iCodeLength - 1; i++)
        {
            size = fread(&(rule[i].iFlag), sizeof(unsigned char), 1, fpMB);
            CHECK_LOAD_TABLE_ERROR(1);
            size = fread(&(rule[i].iWords), sizeof(unsigned char), 1, fpMB);
            CHECK_LOAD_TABLE_ERROR(1);
            rule[i].rule = (RULE_RULE *)malloc(sizeof(RULE_RULE) * iCodeLength);
            for (iTemp = 0; iTemp < iCodeLength; iTemp++)
            {
                size = fread(&(rule[i].rule[iTemp].iFlag), sizeof(unsigned char), 1, fpMB);
                CHECK_LOAD_TABLE_ERROR(1);
                size = fread(&(rule[i].rule[iTemp].iWhich), sizeof(unsigned char), 1, fpMB);
                CHECK_LOAD_TABLE_ERROR(1);
                size = fread(&(rule[i].rule[iTemp].iIndex), sizeof(unsigned char), 1, fpMB);
                CHECK_LOAD_TABLE_ERROR(1);
            }
        }
    }
    head = (RECORD *)malloc(sizeof(RECORD));
    current = head;
    RECORD *recTemp;
    size = fcitx_utils_read_uint32(fpMB, &total);
    CHECK_LOAD_TABLE_ERROR(1);
    for (i = 0; i < total; i++)
    {
        size = fcitx_utils_read_uint32(fpMB, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);
        if (strCode)
        {
            free(strCode);
            strCode = NULL;
        }
        strCode = malloc(sizeof(int8_t) * iTemp);
        size = fread(strCode, sizeof(int8_t), iTemp, fpMB);
        CHECK_LOAD_TABLE_ERROR(iTemp);
        recTemp = (RECORD *)fcitx_utils_malloc0(sizeof(RECORD));
        recTemp->strCode = (char *)fcitx_utils_malloc0(sizeof(char) * iTemp);
        memset(recTemp->strCode, 0, sizeof(char) * iTemp);
        strcpy(recTemp->strCode, strCode);
        size = fcitx_utils_read_uint32(fpMB, &iTemp);
        CHECK_LOAD_TABLE_ERROR(1);
        if (strHZ)
        {
            free(strHZ);
            strHZ = NULL;
        }
        strHZ = malloc(sizeof(int8_t) * iTemp);
        size = fread(strHZ, sizeof(int8_t), iTemp, fpMB);
        CHECK_LOAD_TABLE_ERROR(iTemp);

        recTemp->strHZ = (char *)fcitx_utils_malloc0(sizeof(char) * iTemp);
        strcpy(recTemp->strHZ, strHZ);

        size = fread(&type, sizeof(int8_t), 1, fpMB);
        CHECK_LOAD_TABLE_ERROR(1);
        recTemp->type = type;
        //         printf("%s %s\n",recTemp->strCode,recTemp->strHZ);
        current->next = recTemp;
        recTemp->prev = current;
        current = current->next;
    }
    current->next = head;
    head->prev = current;
    if (strCode)
    {
        free(strCode);
        strCode = NULL;
    }
    if (strHZ)
    {
        free(strHZ);
        strHZ = NULL;
    }
    fclose(fpMB);
    FILE *fpTXT = fopen(txtPath, "w");
    if (!fpTXT)
    {
        printf("\nCannot create target file!\n\n");
        fflush(stdout);
        return 0;
    }
    fprintf(fpTXT, "%s\n", strConst[STR_DESCRIPTION]);
    // 写入名字
    fprintf(fpTXT, "%s%s\n", strConst[STR_NAME], strName);
    fprintf(fpTXT, "-----------------------------------------\n");
    // 写入词库信息
    fprintf(fpTXT, "%s%s\n", strConst[STR_INFORMATION], strInforMation);
    // 写入生成日期
    fprintf(fpTXT, "%s%s\n", strConst[STR_TIME], strTime);
    fprintf(fpTXT, "-----------------------------------------\n");
    // 写入编码截止键
    fprintf(fpTXT, "%s%s\n", strConst[STR_FINISH], strEndkey);
    // 特殊符号引导符
    fprintf(fpTXT, "%s%s\n", strConst[STR_SPECIAL_SYMBOL], strSpecial);
    // 写入编码方案类型
    fprintf(fpTXT, "%s%s\n", strConst[STR_CODETYPE], strCodeType);
    // 写入径直上屏的标点
    fprintf(fpTXT, "%s%s\n", strConst[STR_STRAIGHT_UP], strStraightUp);
    // 写入UsedCodes
    fprintf(fpTXT, "%s%s\n", strConst[STR_KEYCODE], strInputCode);
    // 写入WildChar
    fprintf(fpTXT, "%s%c\n", strConst[STR_WILDCHAR], cWildChar);
    fprintf(fpTXT, "-----------------------------------------\n");
    // 写入组词规则
    fprintf(fpTXT, "%s\n", strConst[STR_RULE]);
    fprintf(fpTXT, "三字词=p11+p21+p31+p32\n");
    fprintf(fpTXT, "-----------------------------------------\n");
    // 写入生癖词
    fprintf(fpTXT, "~:生僻字词\n");
    fprintf(fpTXT, "^:用户词组\n");
    fprintf(fpTXT, "!联想词组\n");
    fprintf(fpTXT, "★★★必须按编码顺序排列，否则词库会出错★★★\n");
    fprintf(fpTXT, "-----------------------------------------\n");

    fprintf(fpTXT, "%s\n", strConst[STR_TEXT]);

    current = head->next;
    while (!isalpha(current->strCode[0]) && current != head)
    {
        current = current->next;
    }
    strCode = current->strCode;
    fprintf(fpTXT, "%s", strCode);
    while (current != head)
    {
        if (strcmp(current->strCode, strCode) != 0)
        {
            strCode = current->strCode;
            fprintf(fpTXT, "\n%s", strCode);
        }
        if (current->type == RECORDTYPE_CONSTRUCT)
            fprintf(fpTXT, " %c%s", cPhraseKey, current->strHZ);
        else if (current->type == RECORDTYPE_UNCOMMON)
            fprintf(fpTXT, " %c%s", cUncommonWords, current->strHZ);
        else
            fprintf(fpTXT, " %s", current->strHZ);
        current = current->next;
        (*HZcount)++;
    }
    current = head->next;
    while (!isalpha(current->strCode[0]) && current != head)
    {
        if (strcmp(current->strCode, strCode) != 0)
        {
            strCode = current->strCode;
            fprintf(fpTXT, "\n%s", strCode);
        }
        if (current->type == RECORDTYPE_CONSTRUCT)
            fprintf(fpTXT, " %c%s", cPhraseKey, current->strHZ);
        else if (current->type == RECORDTYPE_UNCOMMON)
            fprintf(fpTXT, " %c%s", cUncommonWords, current->strHZ);
        else
            fprintf(fpTXT, " %s", current->strHZ);
        current = current->next;
        (*HZcount)++;
    }
    fclose(fpMB);
    return 1;
}
