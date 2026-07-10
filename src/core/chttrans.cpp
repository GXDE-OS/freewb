#include "chttrans.h"

#include "config.h"
#include "log.h"
#include "settings.h"

namespace freewb
{

void Chttrans::loadPair(const std::string &s2tProfile, const std::string &t2sProfile)
{
    const std::string s2tPath = s2tProfile.empty() ? std::string(FREEWB_INSTALL_PKGDATADIR "/s2t/s2t.json") : s2tProfile;
    try
    {
        s2t_ = std::make_unique<opencc::SimpleConverter>(s2tPath);
    }
    catch (const std::exception &)
    {
        FREEWB_ERROR("Failed to load s2t converter: {}", s2tPath);
        s2t_.reset();
    }
}

Chttrans::Chttrans()
{
    loadPair(std::string(), std::string());
    loadSettings();
}

void Chttrans::loadSettings()
{
    available_ = settings::instance().get_simpTradFlg();
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

} // namespace freewb
