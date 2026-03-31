#include "systraymenu.h"

bool SysTrayMenu::s_externImFlg = true;

bool SysTrayMenu::is_extern_im()
{
    return s_externImFlg;
}

void SysTrayMenu::set_extern_im(bool flg)
{
    s_externImFlg = flg;
}
