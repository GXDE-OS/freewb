#include "punc.h"

#include "log.h"
#include "settings.h"

namespace freewb
{
const char *Punc::name() const
{
    return "core:punc";
}

bool Punc::available() const
{
    return true;
}

void Punc::changeAvailable()
{
    available_ = !available_;
}

const std::pair<const char *, const char *> Punc::autoPair(const char *key) const
{
    if (!settings::instance().get_smartMark())
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