#include "ini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcitx-utils/utils.h>

// 这个函数在string.h中应该有的
// strdup()在内部调用了malloc()为变量分配内存，不需要使用返回的字符串时，需要用free()释放相应的内存空间，否则会造成内存泄漏。
char *strdup(const char *str)
{
    if (str == NULL)
        return NULL;
    int len = strlen(str);
    char *ret = (char *)malloc(len + 1);
    strncpy(ret, str, len + 1);
    return ret;
}

// 从文件读取一个ini配置
INI *fileToIni(const char *filename)
{
    INI *ini = NULL;
    FILE *fp = NULL;
    SC *pIndex; // 当前使用节点
    K_V *pKV;   // 当前使用节点的当前键值对

    int i, j;
    char buf[4096];
    char *key = buf;
    char *value = &buf[1024];
    int len = 0;

    if (filename == NULL)
    {
        return NULL;
    }
    ini = (INI *)malloc(sizeof(INI));
    ini->filename = strdup(filename);
    // ini.secs = (SC**)malloc(secsSize * sizeof(SC*));
    // bzero(ini.secs,secsSize*sizeof(SC*));	//全部置为NULL
    ini->seclist = NULL;

    // int flags = O_RDWR | O_CREAT | O_TRUNC | O_EXCL; // 读写方式打开，若文件不存在则创建，截断文件内容，独占方式
    // mode_t mode = 0666; // 设置文件权限

    // int fd = open(path, flags, mode);
    // if (fd == -1) {
    //     perror("open");
    //     return 1;
    // }

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        puts("file open error");
        return NULL;
    }

    while (!feof(fp))
    {
        // getline(buf,4096,fp);
        buf[0] = '\0'; // 读取到文件最后，需要再读一次feof才会为true
        fgets(buf, 4096, fp);
        // 这里是为了加速
        if (buf[0] == '\n' || buf[0] == ';')
        { //';'开头的为注释
            continue;
        }
        // 自己解析(考虑=两边的空白)
        len = strlen(buf);
        if (len < 3)
        {             // 一行小于3个字符，不用解析
            continue; // 解决读取到文件最后，需要再读一次才会使得fin.eof为true
        }
        // 获取key开始位置，取出前面的空白字符
        for (i = 0; i < len; ++i)
        {
            if (buf[i] != '\t' && buf[i] != ' ')
            {
                key = &buf[i];
                break;
            }
        }

        // 如果是注释，不理。是节点，加入到节点中
        if (key[0] == ';')
        {
            continue;
        } // 还可能是注释';'
        if (key[0] == '[')
        { // 读取到了一个节点
            for (value = key + 1; *value != '\0'; ++value)
            {
                if (*value == ']')
                {
                    *value = '\0';
                    break;
                }
            }
            // 判断是否是第一个节点
            if (ini->seclist == NULL)
            {
                // 创建首节点
                ini->seclist = (SC *)malloc(sizeof(SC));
                pIndex = ini->seclist;
            }
            else
            {
                pIndex->next = (SC *)malloc(sizeof(SC));
                pIndex = pIndex->next;
            }
            pIndex->sname = strdup(key + 1);
            // printf("读取节点:\t%s\n",pIndex->sname);
            pIndex->kvlist = NULL;
            pKV = NULL;
            continue;
        }
        len -= (key - (char *)buf);

        // key-value section
        if (strchr(key, '=') == NULL)
            continue;
        // 获取value开始位置
        for (i = 0; i < len; ++i)
        {
            if (key[i] == '\t' || key[i] == ' ' || key[i] == '\n')
            {
                key[i] = '\0'; // 对于key,不允许有空白字符，所以有空白的位置表示结束了
            }
            if (key[i] == '=')
            {                  // 找到'='号，开始排除空白字符
                key[i] = '\0'; // 这代表key最远的结束

                /*
                                for(j=i+1;j<len;++j){//找value的起点
                                    if(key[j] != '\t' && key[j] != ' ' )
                                    {
                                        value = &key[j];
                                        break;
                                    }
                                }
                */
                ++i;
                while (key[i] == '\t' || key[i] == ' ')
                {
                    ++i;
                }

                value = &key[i];
                i = 0;
                while (value[i++])
                {
                    if (value[i] == '\n' || value[i] == '\t')
                    {
                        value[i] = '\0';
                        break;
                    }
                }
                break;
                // 这里没有考虑没有value的情况，也没用考虑没有'='的情况
            }
        }
        // 成功获取了key和value的开始位置
        if (pIndex->kvlist == NULL)
        { // 当前节点第一次添加
            pIndex->kvlist = (K_V *)malloc(sizeof(K_V));
            pKV = pIndex->kvlist;
        }
        else
        {
            pKV->next = (K_V *)malloc(sizeof(K_V));
            pKV = pKV->next;
        }
        pKV->key = strdup(key);
        pKV->value = strdup(value);
        pKV->next = NULL;

        // printf("key:[%s]\t=\t[%s]\n",pKV->key,pKV->value);
    }
    fclose(fp);
    // puts("读取结束");
    return ini;
}

// 创建一个空的ini配置对象
INI *createINI()
{
    return NULL;
}

// 释放ini对象
// 返回释放的键值对数
int freeINI(INI *ini)
{
    SC *pSC = NULL;
    K_V *pKV = NULL;
    int ret = 0;
    if (ini == NULL)
    {
        return 0;
    }
    // 开始释放
    while (ini->seclist != NULL)
    {
        pSC = ini->seclist;
        ini->seclist = pSC->next;
        // 释放节点管理的键值对链表
        while (pSC->kvlist != NULL)
        {
            pKV = pSC->kvlist; // 取出链表头
            pSC->kvlist = pKV->next;
            free(pKV->key);   // 释放key
            free(pKV->value); // 释放value
            free(pKV);        // 释放链表节点
            ret += 1;
        } // end pKV
        free(pSC->sname);
        free(pSC); // 释放节点
    } // end pSC
    free(ini->filename);
    free(ini); // 释放INI(注意：调用此函数后应将ini置为NULL)
    return ret;
}

// 打印输出
void showINI(INI *ini)
{
    SC *pSC = NULL;
    K_V *pKV = NULL;
    if (ini == NULL)
    {
        puts("ini == NULL");
        return;
    }
    printf(";--------- filename = %s--------------\n", ini->filename == NULL ? "not this file" : ini->filename);
    pSC = ini->seclist;
    while (pSC != NULL)
    {
        printf("[%s]\n", (pSC->sname == NULL ? "Not Sections" : pSC->sname));
        pKV = pSC->kvlist;
        while (pKV != NULL)
        {
            printf("%s\t=\t%s\n", (pKV->key == NULL ? "NULL" : pKV->key), (pKV->value == NULL ? "NULL" : pKV->value));
            pKV = pKV->next;
        } // end pKV
        pSC = pSC->next;
    } // end pSC
    puts(";-----------------------------------------------");
}

boolean GetIniKeyBool(INI *ini, const char *Sections, const char *key)
{
    char key2[128];
    sprintf(key2, "#%s", key);
    return (strcasecmp(GetIniKeyString(ini, Sections, key, ""), "true") == 0 ||
            strcasecmp(GetIniKeyString(ini, Sections, key2, ""), "true") == 0);
}

int GetIniKeyInt(INI *ini, const char *Sections, const char *key, const int defineValue)
{
    char *ptr = GetIniKeyString(ini, Sections, key, "");
    if (ptr == "")
        return defineValue;
    return atoi(ptr);
}

// 从INI对象中获取值
// 如果找到了节点或者键，返回对应的值，否则返回defineValue
char *GetIniKeyString(INI *ini, const char *Sections, const char *key, char *defineValue)
{
    SC *pSC = NULL;
    K_V *pKV = NULL;
    if (ini == NULL)
        return defineValue;
    // 查找节点
    pSC = ini->seclist;
    while (pSC != NULL)
    {
        if (strcmp(pSC->sname, Sections) == 0)
        {
            break; // 找到了节点
        }
        pSC = pSC->next;
    }
    if (pSC == NULL)
        return defineValue; // 没有找到节点
    // 查找key
    pKV = pSC->kvlist;
    while (pKV != NULL)
    {
        if (strcmp(pKV->key, key) == 0)
        {
            return pKV->value; // 找到了
        }
        pKV = pKV->next;
    }
    // 没有找到
    return defineValue;
}

// 向INI对象中添加一个记录
// 若是节点不存在或者出错，返回0，否则返回1
int addKVtoINI(INI *ini, const char *Sections, const char *key, const char *value)
{
    SC *pSC = NULL;
    K_V *pKV = NULL;

    if (ini == NULL)
        return 0;
    // 查找节点
    pSC = ini->seclist;
    while (pSC != NULL)
    {
        if (strcmp(pSC->sname, Sections) == 0)
        {
            break; // 找到了节点
        }
        pSC = pSC->next;
    }
    if (pSC == NULL)
    {
        return 0;
    } // 节点不存在
    // 想节点管理链表中添加一个键值对
    // 直接添加为链表头了
    pKV = (K_V *)malloc(sizeof(K_V));
    pKV->key = strdup(key);
    pKV->value = strdup(value);
    pKV->next = pSC->kvlist; // 作为头节点插入
    pSC->kvlist = pKV;
    return 1;
}

// 从INI对象中删除一个记录
// 成功返回1，失败返回0(包括节点或者键不存在)
int delKVfromINI(INI *ini, const char *Sections, const char *key)
{
    SC *pSC = NULL;
    K_V *pKV = NULL;
    K_V *pKV2 = NULL;

    if (ini == NULL)
        return 0;
    // 查找节点
    pSC = ini->seclist;
    while (pSC != NULL)
    {
        if (strcmp(pSC->sname, Sections) == 0)
        {
            break; // 找到了节点
        }
        pSC = pSC->next;
    }
    if (pSC == NULL)
    {
        return 0;
    } // 节点不存在
    // 查找key
    pKV = pSC->kvlist;
    while (pKV != NULL)
    {
        if (strcmp(pKV->key, key) == 0)
        {
            break; // 找到了
        }
        pKV2 = pKV; // 便于找到的时候，执行pKV前一个
        pKV = pKV->next;
    }
    if (pKV == NULL)
    {
        return 0;
    } // 没有找到key
    if (pKV2 == NULL)
    { // 第一个节点就是
        pSC->kvlist = pKV->next;
    }
    else
    { // if-else中都是将带删除key-value移出链表
        pKV2->next = pKV->next;
    }
    free(pKV->key);
    free(pKV->value);
    free(pKV); // 释放
    return 1;
}

// 向INI对象中添加一个节点
// 添加成功返回1(包括节点已经存在)，否则返回0
int addSCtoINI(INI *ini, const char *Sections)
{
    SC *pSC = NULL;
    SC *pSC2 = NULL;
    if (ini == NULL)
        return 0;
    // 查找节点
    pSC = ini->seclist;
    while (pSC != NULL)
    {
        if (strcmp(pSC->sname, Sections) == 0)
        {
            break; // 找到了节点
        }
        pSC2 = pSC; // 用于找不到的时候指向链表的尾部节点
        pSC = pSC->next;
    }
    if (pSC != NULL)
    {
        return 1;
    } // 节点已经存在
    if (pSC2 == NULL)
    { // 当前ini还没有任何节点
        ini->seclist = (SC *)malloc(sizeof(SC));
        pSC = ini->seclist;
    }
    else
    {
        // pSC2目前指向 seclist 尾节点
        pSC2->next = (SC *)malloc(sizeof(SC));
        pSC = pSC2->next;
    }
    pSC->sname = strdup(Sections);
    pSC->kvlist = NULL;
    pSC->next = NULL; // 此处必须有，不然链表成环(与删除节点一起用会出问题)

    return 1;
}

// 从INI对象中删除一个节点
// 成功返回1失败返回0
int delSCfromINI(INI *ini, const char *Sections)
{
    SC *pSC = NULL;
    SC *pSC2 = NULL;
    K_V *pKV = NULL;

    if (ini == NULL)
        return 0;
    // 查找节点
    pSC = ini->seclist;
    while (pSC != NULL)
    {
        if (strcmp(pSC->sname, Sections) == 0)
        {
            break; // 找到了节点
        }
        pSC2 = pSC; // 用于找到的时候，保存pSC的前一个
        pSC = pSC->next;
    }
    if (pSC == NULL)
    {
        return 0;
    } // 节点不存在

    if (pSC2 == NULL)
    { // 第一节点就是要删除的
        ini->seclist = pSC->next;
    }
    else
    {
        pSC2->next = pSC->next;
    }
    // 释放节点*pSC
    // 释放节点管理的键值对链表
    while (pSC->kvlist != NULL)
    {
        pKV = pSC->kvlist; // 取链表头
        pSC->kvlist = pKV->next;
        free(pKV->key);   // 释放key
        free(pKV->value); // 释放value
        free(pKV);        // 释放链表节点
    } // end pKV
    free(pSC->sname);
    free(pSC); // 释放节点

    return 1;
}

// 将INI对象保存到指定文件
int saveINItoFile(INI *ini, const char *filename)
{
    SC *pSC = NULL;
    K_V *pKV = NULL;
    int ret = 0;     // 计数
    FILE *fp = NULL; // 文件指针
    if (ini == NULL)
    {
        puts("ini == NULL");
        return;
    }
    fp = fopen(filename, "w");
    if (fp == NULL)
        return -1;

    // printf(";--------- filename = %s--------------\n",
    //	ini->filename == NULL?"not this file":ini->filename);
    pSC = ini->seclist;
    while (pSC != NULL)
    {
        fprintf(fp, "[%s]\n", (pSC->sname == NULL ? "Not Sections" : pSC->sname));
        pKV = pSC->kvlist;
        while (pKV != NULL)
        {
            fprintf(fp, "%s\t=\t%s\n", (pKV->key == NULL ? "NULL" : pKV->key), (pKV->value == NULL ? "NULL" : pKV->value));
            ++ret;
            pKV = pKV->next;
        } // end pKV
        pSC = pSC->next;
    } // end pSC
    return ret;
}

// 将INI对象保存到来源文件
int saveINI(INI *ini)
{
    // 先不考虑来源文件里面原有的内容了
    return saveINItoFile(ini, ini->filename);
}
