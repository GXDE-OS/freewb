#include "punc.h"

#include "log.h"
#include "settings.h"

namespace freewb
{

Punc::Punc()
{
    available_ = settings::instance().get_smartMark();
}

const char *Punc::name() const
{
    return "core:punc";
}

bool Punc::available() const
{
    return available_;
}

void Punc::changeAvailable()
{
    available_ = !available_;
    settings::instance().set_smartMark(available_);
}

const std::pair<const char *, const char *> Punc::autoPair(const char *key) const
{
    if (!available_)
    {
        return std::make_pair(nullptr, nullptr);
    }
    for (int i = 0; i < sizeof(PuncPairList) / sizeof(PuncPairList[0]); i++)
    {
        if (strcmp(key, PuncPairList[i].left) == 0 || strcmp(key, PuncPairList[i].right) == 0)
        {
            return std::make_pair(PuncPairList[i].left, PuncPairList[i].right);
        }
    }
    return std::make_pair(nullptr, nullptr);
}
} // namespace freewb