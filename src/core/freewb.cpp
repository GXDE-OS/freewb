#include "freewb.h"

#include <cstring>
#include <utility>

#include "key.h"
#include "settings.h"

namespace freewb
{

Freewb::Freewb(ipc::IDBus *dbusProxy, CommitCallback commitCallback) : log_("/tmp/freewb-engine.log"), dbusProxy_(dbusProxy)
{
    punc_ = new Punc();
    chttrans_ = new Chttrans();
    candidateList_ = new CandidateList(chttrans_);
    committer_ = new Committer(std::move(commitCallback), this);
    engineManager_ = new EngineManager(candidateList_, committer_);
    userPhrase_ = new UserPhrase(
        dbusProxy_,
        [this](const std::string &phrase) { return engineManager_->calculateWubiPhraseCode(phrase); },
        [this](int charCount) { return committer_->committedText(static_cast<std::size_t>(charCount)); });
    connectDBusCallback();
}

Freewb::~Freewb()
{
    if (engineManager_ != nullptr)
    {
        delete engineManager_;
        engineManager_ = nullptr;
    }
    if (committer_ != nullptr)
    {
        delete committer_;
        committer_ = nullptr;
    }
    if (candidateList_ != nullptr)
    {
        delete candidateList_;
        candidateList_ = nullptr;
    }
    if (chttrans_ != nullptr)
    {
        delete chttrans_;
        chttrans_ = nullptr;
    }
    if (punc_ != nullptr)
    {
        delete punc_;
        punc_ = nullptr;
    }
    if (userPhrase_ != nullptr)
    {
        delete userPhrase_;
        userPhrase_ = nullptr;
    }
}

void Freewb::activate()
{
    dbusProxy_->callPanelShowToolbar();
}

void Freewb::deactivate()
{
    userPhrase_->reset();
    engineManager_->reset();
    candidateList_->clear();
    dbusProxy_->callPanelHideToolbar();
    dbusProxy_->callPanelUpdatePreeditText({.text = "", .caret = 0, .show = false});
    dbusProxy_->callPanelUpdateCandidate(
        {.fullCodes = {}, .texts = {}, .prompts = {}, .hasPrev = false, .hasNext = false, .cursor = -1, .layout = Horizontal});
}

EngineManager *Freewb::engineManager() const
{
    return engineManager_;
}

CandidateList *Freewb::candidateList() const
{
    return candidateList_;
}

Punc *Freewb::punc() const
{
    return punc_;
}

ipc::IDBus *Freewb::dbusProxy() const
{
    return dbusProxy_;
}

bool Freewb::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    FREEWB_DEBUG("keysym: {}, state: {}", static_cast<int>(keysym), static_cast<int>(state));
    bool processed = false;
    processed = userPhrase_->processKey(keysym, state);
    if (processed)
    {
        return true;
    }

    processed = handleSingleShortcutKey(keysym, state);
    if (processed)
    {
        return true;
    }

    processed = handleGlobalShortcutKey(keysym, state);
    if (processed)
    {
        return true;
    }

    processed = engineManager_->processKey(keysym, state);
    if (processed)
    {
        return true;
    }

    processed = committer_->processKey(keysym, state);
    if (processed)
    {
        return true;
    }

    return false;
}

void Freewb::reset()
{
    userPhrase_->reset();
    engineManager_->reset();
    candidateList_->clear();
}

void Freewb::reloadConfig()
{
    settings::instance().reload();

    const bool userWordFlg = settings::instance().get_userWordFlg();

    committer_->loadSettings();
    candidateList_->loadSettings();

    if (userWordFlg)
    {
        engineManager_->reloadDictionaries();
        dbusProxy_->callUsrWordLoadOkMethod();
    }

    updateCandidateAndPreeditToUI();
}

bool Freewb::handleGlobalShortcutKey(FreewbKeySym keysym, FreewbKeyState state)
{
    {
        const char *keyString = Key::readKeyString(settings::instance().get_backFindCode().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            dbusProxy_->callDictQueryMethod(committer_->lastCommitString());
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_markAutoPair().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            // make mark auto pair
            punc_->changeAvailable();
            return true;
        }
    }
    {

        const char *keyString = Key::readKeyString(settings::instance().get_onlineAddWord().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            const char *engine = engineManager_->currentEngineName();
            if (engine != nullptr && std::strcmp(engine, "engine:py") == 0)
            {
                return false;
            }
            // 在线造词，使用历史上屏的文本
            userPhrase_->enterAddPhraseState(false);
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_onlineAddWord().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        const FreewbKeyState wantState = static_cast<FreewbKeyState>(FreewbKeyState_Ctrl | FreewbKeyState_Alt);
        if (keysym == keySym && state == wantState)
        {
            const char *engine = engineManager_->currentEngineName();
            if (engine != nullptr && std::strcmp(engine, "engine:py") == 0)
            {
                return false;
            }
            // 在线造词，使用剪贴板的文本
            userPhrase_->enterAddPhraseState(true);
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_onlineDelWord().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            const char *engine = engineManager_->currentEngineName();
            if (engine != nullptr && std::strcmp(engine, "engine:py") == 0)
            {
                return false;
            }
            // 在线删词
            const std::string &delText = committer_->lastCommitString();
            userPhrase_->enterDeletePhraseState(delText);
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_quickDelScreenItem().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            // 暂时不实现
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_setupOption().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            dbusProxy_->callOpenUiSettingMethod();
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchCharSet().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            dbusProxy_->callPanelSwitchCharSetMethod();
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchChttrans().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            chttrans_->changeAvailable();
            dbusProxy_->callPanelSwitchChttransMethod();
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchInputMode().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            engineManager_->nextEngine();
            const char *nextEngine = engineManager_->currentEngineName();
            dbusProxy_->callPanelSwitchInputModeMethod(nextEngine);
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchLexicon().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            dbusProxy_->callSwitchTableMethod();
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchSkin().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            dbusProxy_->callSwitchSkinMethod();
            return true;
        }
    }

    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchVKb().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            dbusProxy_->callSwitchVirtualKeyboardModeMethod(0);
            return true;
        }
    }

    return false;
}

bool Freewb::handleSingleShortcutKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (state != FreewbKeyState_None)
    {
        return false;
    }

    if (keysym == FreewbKey_Escape)
    {
        userPhrase_->reset();
        candidateList_->clear();
        engineManager_->reset();
        return true;
    }

    const FreewbKeySym prevPageKey = Key::keySymFromUniqueName(settings::instance().get_prevPageKey().c_str());
    if (keysym == prevPageKey)
    {
        if (candidateList_->size() == 0)
        {
            return false;
        }
        candidateList_->prev();
        return true;
    }

    const FreewbKeySym nextPageKey = Key::keySymFromUniqueName(settings::instance().get_nextPageKey().c_str());
    if (keysym == nextPageKey)
    {
        if (candidateList_->size() == 0)
        {
            return false;
        }
        candidateList_->next();
        return true;
    }

    if (keysym == FreewbKey_BackSpace)
    {
        if (candidateList_->preeditText().empty())
        {
            return false;
        }

        candidateList_->popPreeditText();
        engineManager_->refreshEngineResult();
        return true;
    }

    return false;
}

void Freewb::connectDBusCallback()
{
    dbusProxy_->bindDBusSignalCallback(
        [this](const char *member, int index)
        {
            if (std::strcmp(member, "SelectCandidate") == 0)
            {
                this->committer_->selectCandidate(static_cast<int>(index));
            }
            else if (std::strcmp(member, "LookupTablePageUp") == 0)
            {
                this->candidateList_->prev();
                this->updateCandidateAndPreeditToUI();
            }
            else if (std::strcmp(member, "LookupTablePageDown") == 0)
            {
                this->candidateList_->next();
                this->updateCandidateAndPreeditToUI();
            }
            else if (std::strcmp(member, "ReloadConfig") == 0)
            {
                this->reloadConfig();
            }
            else if (std::strcmp(member, "RequestNextInputMode") == 0)
            {
                this->engineManager_->nextEngine();
                const std::string nextEngine = this->engineManager_->currentEngineName();
                this->dbusProxy_->callPanelSwitchInputModeMethod(nextEngine.c_str());
            }
            else if (std::strcmp(member, "SwitchPunctuation") == 0)
            {
                this->punc_->changeAvailable();
            }
            else if (std::strcmp(member, "SwitchChttrans") == 0)
            {
                this->chttrans_->changeAvailable();
            }
        });
}

void Freewb::updateCandidateAndPreeditToUI()
{
    dbusProxy_->callPanelUpdatePreeditText({.text = candidateList_->preeditText(),
                                             .caret = candidateList_->cursor(),
                                             .show = !candidateList_->preeditText().empty()});
    dbusProxy_->callPanelUpdatePreeditCaret(candidateList_->cursor());
    dbusProxy_->callPanelUpdateCandidate({.fullCodes = {},
                                      .texts = candidateList_->candidateTexts(),
                                      .prompts = candidateList_->candidatePrompts(),
                                      .hasPrev = candidateList_->hasPrev(),
                                      .hasNext = candidateList_->hasNext(),
                                      .cursor = -1,
                                      .layout = Horizontal});
}

} // namespace freewb