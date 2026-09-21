#include "virtualkeyboard.h"

#include "committer.h"
#include "freewb.h"
#include "key.h"
#include "settings.h"

namespace freewb
{

VirtualKeyboard::VirtualKeyboard(Freewb *freewb) : freewb_(freewb)
{
    loadSettings();
}

const char *VirtualKeyboard::name() const
{
    return "core:virtualkeyboard";
}

bool VirtualKeyboard::available() const
{
    return mode_ != VK_MODE_CLOSED && mode_ != VK_MODE_PC;
}

void VirtualKeyboard::changeAvailable()
{
}

void VirtualKeyboard::loadSettings()
{
    mode_ = static_cast<VirtualKeyboardMode>(settings::instance().get_vkMode());
    customChar_ = VkLayouts::parse(settings::instance().get_CoustomChar());
}

bool VirtualKeyboard::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (freewb_ == nullptr || !available())
    {
        return false;
    }

    const FreewbKeySym sym = Key::normalizedKeySymbol(keysym, state);
    if (sym == FreewbKey_None || sym < FreewbKey_space || sym > static_cast<FreewbKeySym>(0x007e))
    {
        return false;
    }

    const unsigned char ascii = static_cast<unsigned char>(sym);
    const std::string text = (mode_ == VK_MODE_USER_CHAR)
                                 ? VkLayouts::convertToCustomSymbol(customChar_, ascii)
                                 : VkLayouts::convertToVkText(mode_, ascii);
    if (text.empty())
    {
        return false;
    }

    if (freewb_->committer() != nullptr)
    {
        freewb_->committer()->commit(text);
    }
    return true;
}

} // namespace freewb
