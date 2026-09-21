#include "statemanager.h"

#include "candidatelist.h"
#include "chttrans.h"
#include "committer.h"
#include "enginemanager.h"
#include "freewb.h"
#include "idbus.h"
#include "key.h"
#include "log.h"
#include "punc.h"
#include "settings.h"
#include "special.h"

namespace freewb
{

/** 无参 IDBus：X("dos.", callOpenConfDirMethod) */
#define QUICK_COMMAND_CALL_DBUS_METHOD(X)                                                                                        \
    X("dos.", callOpenConfDirMethod)                                                                                             \
    X("tt.", callSwitchRecodeProofMethod)                                                                                        \
    X("hh.", callSwitchToolbarHideFlgMethod)                                                                                     \
    X("oo.", callOpenUiSettingMethod)                                                                                            \
    X("pp.", callOpenProfessionalSettingMethod)                                                                                  \
    X("vv.", callShowVersionInfoMethod)                                                                                          \
    X("qq.", callModQuickTableMethod)                                                                                            \
    X("uu.", callModUserTableMethod)                                                                                             \
    X("uw.", callModWubiTableMethod)                                                                                             \
    X("up.", callModPinyinTableMethod)

/** 需传参 IDBus 方法的快捷命令：X("aa.", handleAddPhrase) */
#define QUICK_COMMAND_HANDLE_CUSTOM(X)                                                                                           \
    X("aa.", handleAddPhrase)                                                                                                    \
    X("dd.", handleDeletePhrase)                                                                                                 \
    X("ff.", handleDictQuery)                                                                                                    \
    X("jj.", handleToggleChttrans)                                                                                               \
    X("mm.", handleToggleCharSet)                                                                                                \
    X("kk.", handleSwitchVirtualKeyboard)

TempEnglishState::TempEnglishState(StateManager *manager, Freewb *freewb) : manager_(manager), freewb_(freewb)
{
    registerQuickCommands();
}

const char *TempEnglishState::name() const
{
    return "state:tempEnglish";
}

bool TempEnglishState::available() const
{
    return available_;
}

void TempEnglishState::changeAvailable()
{
    available_ = !available_;
}

bool TempEnglishState::handleAddPhrase()
{
    if (manager_ == nullptr)
    {
        return false;
    }
    const bool entered = manager_->enterAddPhraseState(false);
    if (!entered)
    {
        FREEWB_WARN("[{}] add phrase failed: insufficient commit history", name());
        return false;
    }
    if (freewb_ == nullptr)
    {
        return true;
    }
    CandidateList *const candidates = freewb_->candidateList();
    if (candidates == nullptr)
    {
        return true;
    }
    candidates->clear();
    return true;
}

bool TempEnglishState::handleDeletePhrase()
{
    if (manager_ == nullptr || freewb_ == nullptr || freewb_->committer() == nullptr)
    {
        return false;
    }
    const bool entered =
        manager_->enterDeletePhraseState(freewb_->committer()->lastCommitString(), freewb_->committer()->lastCommitCode());
    if (!entered)
    {
        FREEWB_WARN("[{}] delete phrase failed: empty commit text", name());
        return false;
    }
    if (CandidateList *const candidates = freewb_->candidateList(); candidates != nullptr)
    {
        candidates->clear();
    }
    return true;
}

bool TempEnglishState::handleDictQuery()
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr || freewb_->committer() == nullptr)
    {
        return false;
    }
    freewb_->dbusProxy()->callDictQueryMethod(freewb_->committer()->lastCommitString());
    return true;
}

bool TempEnglishState::handleToggleChttrans()
{
    if (freewb_ == nullptr)
    {
        return false;
    }
    if (freewb_->chttrans() != nullptr)
    {
        freewb_->chttrans()->changeAvailable();
    }
    if (freewb_->dbusProxy() != nullptr)
    {
        freewb_->dbusProxy()->callPanelSwitchChttransMethod();
    }
    return true;
}

bool TempEnglishState::handleToggleCharSet()
{
    if (freewb_ == nullptr || freewb_->dbusProxy() == nullptr)
    {
        return false;
    }
    freewb_->engineManager()->toggleCharset();
    freewb_->dbusProxy()->callPanelSwitchCharSetMethod();
    return true;
}

bool TempEnglishState::handleSwitchVirtualKeyboard()
{
    if (freewb_ != nullptr && freewb_->dbusProxy() != nullptr)
    {
        freewb_->dbusProxy()->callSwitchVirtualKeyboardModeMethod(0);
    }
    return true;
}

void TempEnglishState::registerQuickCommands()
{
#define QUICK_COMMAND_REGISTER_DBUS_METHOD(PATTERN, METHOD)                                                                      \
    quickCommands_[PATTERN] = [freewb = freewb_, method = &ipc::IDBus::METHOD]()                                                 \
    {                                                                                                                            \
        if (freewb != nullptr && freewb->dbusProxy() != nullptr)                                                                 \
        {                                                                                                                        \
            (freewb->dbusProxy()->*method)();                                                                                    \
        }                                                                                                                        \
        return true;                                                                                                             \
    };
    QUICK_COMMAND_CALL_DBUS_METHOD(QUICK_COMMAND_REGISTER_DBUS_METHOD)
#undef QUICK_COMMAND_REGISTER_DBUS_METHOD

#define QUICK_COMMAND_REGISTER_CUSTOM(PATTERN, HANDLER) quickCommands_[PATTERN] = [this]() { return HANDLER(); };
    QUICK_COMMAND_HANDLE_CUSTOM(QUICK_COMMAND_REGISTER_CUSTOM)
#undef QUICK_COMMAND_REGISTER_CUSTOM
}

bool TempEnglishState::dispatchQuickCommand(const std::string &commandBodyWithoutPrefix)
{
    if (manager_ == nullptr)
    {
        return false;
    }
    const auto it = quickCommands_.find(commandBodyWithoutPrefix);
    if (it == quickCommands_.end())
    {
        return false;
    }
    FREEWB_DEBUG("[{}] quick command: {}", name(), commandBodyWithoutPrefix);
    if (!it->second())
    {
        FREEWB_WARN("[{}] quick command failed: {}", name(), commandBodyWithoutPrefix);
        return false;
    }
    if (manager_->isTempEnglish())
    {
        manager_->reset();
    }
    return true;
}

std::vector<std::string> TempEnglishState::uninterestedEngines() const
{
    return {"engine:en"};
}

bool TempEnglishState::begin(const std::string &commandPrefix)
{
    if (commandPrefix.empty() || freewb_ == nullptr)
    {
        return false;
    }
    CandidateList *candidates = freewb_->candidateList();
    if (candidates == nullptr)
    {
        return false;
    }
    // 已有编码或候选时不进入临时英文
    if (!candidates->preeditText().empty() || candidates->totalCandidateCount() > 0)
    {
        return false;
    }
    candidates->clear();
    candidates->setLead(commandPrefix);
    secondRecodeKey_ = Key::keySymFromUniqueName(settings::instance().get_secondRecodeKey().c_str());
    thirdRecodeKey_ = Key::keySymFromUniqueName(settings::instance().get_thirdRecodeKey().c_str());
    return true;
}

void TempEnglishState::cancel()
{
    freewb_->candidateList()->clear();
}

void TempEnglishState::refreshQuickFormatCandidates()
{
    CandidateList *const candidates = freewb_->candidateList();
    Special *const special = freewb_->special();
    const std::string &code = candidates->inputCode();
    if (code.empty() || special == nullptr || !special->available())
    {
        candidates->setCandidates({}, {});
        return;
    }

    const std::vector<std::string> texts = special->formatSpecialValues(code);
    if (texts.empty())
    {
        candidates->setCandidates({}, {});
        return;
    }

    std::vector<std::string> prompts(texts.size());
    candidates->setCandidates(texts, prompts);
}

bool TempEnglishState::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    CandidateList *const candidates = freewb_->candidateList();

    if (Key::isModifierKeySym(keysym))
    {
        return false;
    }
    if (!Key::hasNoModifier(state))
    {
        // Ctrl/Alt 等：清空退出，键交给后续组合键/应用
        manager_->reset();
        return false;
    }

    if (keysym == FreewbKey_BackSpace)
    {
        if (candidates->inputCode().empty())
        {
            manager_->reset();
        }
        else
        {
            candidates->popInputCode();
            refreshQuickFormatCandidates();
        }
        return true;
    }

    // 回车、有候选的空格、够选的二三选：清 lead 回 idle，交给 Committer
    const int size = candidates->size();
    if (keysym == FreewbKey_Return || (keysym == FreewbKey_space && size > 0) ||
        (keysym == secondRecodeKey_ && size > 1) || (keysym == thirdRecodeKey_ && size > 2))
    {
        candidates->setLead({});
        manager_->enterIdleState();
        return false;
    }

    // 可打印 ASCII（含无候选空格、不够选的二三选键、数字等）：追加
    if (keysym >= FreewbKey_space && keysym <= FreewbKey_asciitilde)
    {
        const char ch = static_cast<char>(keysym);
        const std::string code = candidates->inputCode() + ch;
        if (keysym == FreewbKey_period && dispatchQuickCommand(code))
        {
            return true;
        }
        candidates->setInputCode(code);
        refreshQuickFormatCandidates();
        return true;
    }

    return false;
}

} // namespace freewb
