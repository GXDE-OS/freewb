#ifndef _FREEWB_CONVERSION_TOOL_
#define _FREEWB_CONVERSION_TOOL_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef __cplusplus
extern "C"{
#endif
int txt2mb(char* txtPath, char* mbPath, int* HZcount);
int mb2txt(char* txtPath, char* mbPath, int* HZcount);



#ifdef __cplusplus
}
#endif
#endif
