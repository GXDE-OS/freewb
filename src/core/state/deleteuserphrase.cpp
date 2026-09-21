#include "statemanager.h"

#include "enginemanager.h"
#include "freewb.h"
#include "idbus.h"
#include "key.h"
#include "log.h"

namespace freewb
{

DeleteUserPhraseState::DeleteUserPhraseState(StateManager *manager, Freewb *freewb) : manager_(manager), freewb_(freewb)
{
}

const char *DeleteUserPhraseState::name() const
{
    return "state:deletePhrase";
}

bool DeleteUserPhraseState::available() const
{
    return available_;
}

void DeleteUserPhraseState::changeAvailable()
{
    available_ = !available_;
}

bool DeleteUserPhraseState::begin(const std::string &wordText, const std::string &wordCode)
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return false;
    }

    wordText_ = filterNonHanziContent(wordText);
    if (wordText_.empty())
    {
        FREEWB_WARN("[{}] begin: no hanzi in commit text", name());
        return false;
    }
    wordCode_ = wordCode;
    freewb_->dbusProxy()->callDeleteUsrParseMethod(0, wordCode_, wordText_);
    return true;
}

void DeleteUserPhraseState::cancel()
{
    if (freewb_ != nullptr && freewb_->dbusProxy() != nullptr)
    {
        freewb_->dbusProxy()->callDeleteUsrParseMethod(2, "", "");
    }
}

bool DeleteUserPhraseState::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return true;
    }

    if (keysym == FreewbKey_Return && Key::hasNoModifier(state))
    {
        if (freewb_->engineManager() != nullptr)
        {
            (void)freewb_->engineManager()->deleteUserWord(wordCode_, wordText_);
        }
        freewb_->dbusProxy()->callDeleteUsrParseMethod(2, "", "");
        if (manager_ != nullptr)
        {
            manager_->enterIdleState();
        }
        return true;
    }
    return true;
}

std::vector<std::string> DeleteUserPhraseState::uninterestedEngines() const
{
    return {"engine:py", "engine:en"};
}

} // namespace freewb
