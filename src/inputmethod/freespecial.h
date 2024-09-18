#ifndef _FREESPECIAL_H_
#define _FREESPECIAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcitx-utils/utils.h>
#include <fcitx/keys.h>

enum PuncKeyNumber{
    Key_Angle_bracket,
    Key_Point_number,
    Key_quotation_marks,
    Key_big_parantheses,
    Key_Bracket,
    Key_Parentheses
};
char* matchTime(char* str);
int isPuncKey(FcitxKeySym sym);
void matchPunc(char *str,int index,boolean puncState);
void switchTempKey(char *str,boolean puncState);

char* matchNumber(char* str);
char* matchNumberSim(char* str);
char* matchMoney(char* str);
char* matchtimeManul(char* str,int flg);//0-HZ   1-number   2-yyyy-mm-rr

#endif
