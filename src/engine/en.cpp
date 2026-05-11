#include "en.h"

#include "log.h"

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
    static const CandidatePayload kEmpty{};
    return kEmpty;
}

void En::reset()
{
    return;
}

int En::inputCodeLength() const
{
    return 128;
}

bool En::shouldProcessKey(const char *key) const
{
    return false;
}

bool En::isExactDictionaryKey(const std::string &preedit) const
{
    (void)preedit;
    return false;
}

bool En::isPreeditOverflow(const char *key, const std::string &pre, const std::string &full) const
{
    (void)key;
    (void)pre;
    (void)full;
    return false;
}
} // namespace freewb
