#include "chttrans.h"
#include "settings.h"
#include "log.h"

namespace freewb
{

void Chttrans::loadPair(const std::string &s2tProfile, const std::string &t2sProfile)
{
    const std::string s2tPath =
        s2tProfile.empty() ? std::string(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD) : s2tProfile;
    try
    {
        s2t_ = std::make_unique<opencc::SimpleConverter>(s2tPath);
    }
    catch (const std::exception &)
    {
        FREEWB_ERROR("Failed to load s2t converter: {}", s2tPath);
        s2t_.reset();
    }

    const std::string t2sPath =
        t2sProfile.empty() ? std::string(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP) : t2sProfile;
    try
    {
        t2s_ = std::make_unique<opencc::SimpleConverter>(t2sPath);
    }
    catch (const std::exception &)
    {
        FREEWB_ERROR("Failed to load t2s converter: {}", t2sPath);
        t2s_.reset();
    }
}

Chttrans::Chttrans()
{
    loadPair(std::string(), std::string());
}

const char *Chttrans::name() const
{
    return "core:chttrans";
}

bool Chttrans::available() const
{
    return available_;
}

void Chttrans::changeAvailable()
{
    available_ = !available_;
}

void Chttrans::simpToTrad(std::string &text) const
{
    if (!available_ || !s2t_)
    {
        FREEWB_WARN("available_ is false or s2t_ is nullptr");
        return;
    }
    if (!settings::instance().get_simpTradFlg())
    {
        return;
    }

    try
    {
        text = s2t_->Convert(text);
    }
    catch (const std::exception &)
    {
        FREEWB_ERROR("Failed to convert text to trad: {}", text);
    }
}

void Chttrans::tradToSimp(std::string &text) const
{
    if (!available_ || !t2s_)
    {
        return;
    }

    try
    {
        text = t2s_->Convert(text);
    }
    catch (const std::exception &)
    {
        FREEWB_ERROR("Failed to convert text to simp: {}", text);
    }
}

} // namespace freewb
