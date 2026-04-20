#include "en.h"

#include "log.h"
#include "settings.h"

namespace freewb
{
const char *En::name() const
{
    return "engine:en";
}

bool En::available() const
{
    return available_;
}

void En::changeAvailable()
{
    available_ = !available_;
}

void En::putKey(const char *strCode)
{
    FREEWB_DEBUG("En::putKey: %s", strCode);
    return;
}

const CandidatePayload &En::getResult() const
{
    return CandidatePayload{};
}

void En::reset()
{
    return;
}

int En::inputCodeLength() const
{
    return 128;
}
} // namespace freewb
