/**********************************************************************
** @作者：lcj
**
** @说明：
**      该文件定义了一些其它源文件所用到的通用宏
*********************************************************************/

#ifndef COMMDEFINE_H
#define COMMDEFINE_H

#include "config.h"

//软件安装路径后缀(在ＨＯＭＥ下面)
#define INSTALL_DIR QString(qgetenv("HOME")) + "/.local/freewb"

enum CpuType
{
    CT_X86,
    CT_ARM
};

enum DesktopType
{
    DT_UBUNTU,//ubuntu
    DT_MATE,//kylin
    DT_UKUI,//uKylin
    DT_DEEPIN//deepin
};



extern CpuType g_cpuType;
extern DesktopType g_desktopType;



#endif
