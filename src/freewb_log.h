#ifndef FREEWB_LOG_H
#define FREEWB_LOG_H
#include <stdio.h>
#define FREEWB_UI_FLAG_FILE "/tmp/freewb-log-ui.txt"
//ㄧ
#ifdef __cplusplus
        extern "C" {
#endif
        void FreewbLog(char* pstr,...);
#ifdef __cplusplus
        }
#endif

#endif
