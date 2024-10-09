#ifndef _FREE_KEYBORD_H
#define _FREE_KEYBORD_H
#include "fcitx/fcitx.h"
#include "fcitx-config/fcitx-config.h"
#include "stdio.h"
#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct _FREE_KEYBOARD
    {
        FcitxKeySym code;
        char value[10];

    } FREE_KEYBOARD;

    // 虚拟键盘工作模式
    typedef enum
    {
        VKM_INPUT_PC = 0,        // ＰＣ键盘输入模式
        VKM_INPUT_GREEK,         // 希腊字母
        VKM_INPUT_RUSSIAN,       // 俄文字母
        VKM_INPUT_PHONETIC,      // 注音符号
        VKM_INPUT_PINYIN,        // 汉语拼音
        VKM_INPUT_JAPAN_FLAT,    // 日文平假名
        VKM_INPUT_JAPAN_PIECE,   // 日文片假名
        VKM_INPUT_PUNCTUATION,   // 标点符号
        VKM_INPUT_DIGITAL_ORDER, // 数字序号
        VKM_INPUT_MATH,          // 数学符号
        VKM_INPUT_UNIT,          // 单位符号
        VKM_INPUT_TABS,          // 制表符
        VKM_INPUT_SPECIAL,       // 特殊符号

        VKM_INPUT_USER_CHAR, // 用户自定义字符输入模式
        VKM_INPUT_USER_MARK, // 用户自定义标点输入模式

        VKM_NUM
    } vrtual_keyboard_mode_t;

    void setVKboard(char *str, vrtual_keyboard_mode_t mode);
    char *matchVkBoard(unsigned char sym, int modeIndex);

#ifdef __cplusplus
    extern "C"
}
#endif

#endif
