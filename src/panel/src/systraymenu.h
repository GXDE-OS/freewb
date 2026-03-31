/*****************************************************************************************
** @作者：lcj
**
** @说明：
**      SysTrayMenu是一个从QSystemTrayIcon类继承而来的UI类，起设计目的为替输入法在系统任务栏提供一个
** 　　　托盘菜单．
****************************************************************************************/

#ifndef SYSTRAYMENU_H
#define SYSTRAYMENU_H

class SysTrayMenu
{
public:
    SysTrayMenu() = default;
    ~SysTrayMenu() = default;

public:
    static bool s_externImFlg;
    static bool is_extern_im();
    static void set_extern_im(bool flg);
};

#endif
