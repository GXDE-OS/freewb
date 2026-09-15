#include "statemanager.h"

namespace freewb
{

bool IdleState::processKey(FreewbKeySym /*keysym*/, FreewbKeyState /*state*/)
{
    return false;
}

void IdleState::cancel()
{
}

const char *IdleState::name() const
{
    return "state:idle";
}

bool IdleState::available() const
{
    return true;
}

void IdleState::changeAvailable()
{
    return;
}

std::vector<std::string> IdleState::uninterestedEngines() const
{
    return {};
}

} // namespace freewb
