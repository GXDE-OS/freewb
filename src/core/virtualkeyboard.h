#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include "ifreewb.h"
#include "keysym.h"
#include "vklayouts.h"

namespace freewb
{

class Freewb;

/** 软键盘布局拦截：物理键按当前 vkMode 上屏，PC/关闭不拦截。 */
class VirtualKeyboard final : public IFreewb
{
public:
    explicit VirtualKeyboard(Freewb *freewb);
    ~VirtualKeyboard() override = default;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    void loadSettings();
    bool processKey(FreewbKeySym keysym, FreewbKeyState state);

private:
    Freewb *freewb_ = nullptr;
    VirtualKeyboardMode mode_ = VK_MODE_CLOSED;
    VkLayout customChar_{};
};

} // namespace freewb

#endif /* VIRTUALKEYBOARD_H */
