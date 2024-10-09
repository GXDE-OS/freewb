#ifndef __C_INI_OP_H__
#define __C_INI_OP_H__
#include <fcitx-utils/utils.h>
/*******************************************************

  INI{Sections}	//INI文件
		|
		---->{s0,s1,s2,s3...sn}	//节点数组
			   |
			   ---->{{key1:value1},	//键值对
					 {key2:value2},
					 ...
					 {keyN:valueN}
					 }
树形关系
*******************************************************/

// 键值对
typedef struct KeyValue
{
	char *key;			   // 键
	char *value;		   // 值
	struct KeyValue *next; // 下一个键值对
} K_V;

// 节点
typedef struct SECTIONS
{
	char *sname;		   // 节点名
	K_V *kvlist;		   // 节点下的键值对链表
	struct SECTIONS *next; // 下一个节点
} SC;

typedef struct INIObject
{
	char *filename; // 对应的文件
	SC *seclist;	// 节点链表指针
} INI;

// 从文件读取一个ini配置
INI *fileToIni(const char *filename);
// 创建一个空的ini配置对象
INI *createINI();
// 释放ini对象
int freeINI(INI *ini);

// 打印输出
void showINI(INI *ini);

// 从INI对象中获取值
boolean GetIniKeyBool(INI *ini, const char *Sections, const char *key);
int GetIniKeyInt(INI *ini, const char *Sections, const char *key, const int defineValue);
char *GetIniKeyString(INI *ini, const char *Sections, const char *key, char *defineValue);
// 向INI对象中添加一个记录
int addKVtoINI(INI *ini, const char *Sections, const char *key, const char *value);
// 从INI对象中删除一个记录
int delKVfromINI(INI *ini, const char *Sections, const char *key);
// 向INI对象中添加一个节点
int addSCtoINI(INI *ini, const char *Sections);
// 从INI对象中删除一个节点
int delSCfromINI(INI *ini, const char *Sections);

// 将INI对象保存到指定文件
int saveINItoFile(INI *ini, const char *filename);
// 将INI对象保存到来源文件
int saveINI(INI *ini);

#endif //!__C_INI_OP_H__
