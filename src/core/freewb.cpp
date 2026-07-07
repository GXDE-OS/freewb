#include "freewb.h"

#include <cstring>
#include <utility>
#include <vector>

#include "charwidth.h"
#include "chttrans.h"
#include "key.h"
#include "settings.h"
#include "special.h"
#include "statemanager.h"

namespace freewb
{

Freewb::Freewb(ipc::IDBus *dbusProxy, CommitCallback commitCallback) : log_("/tmp/freewb-engine.log"), dbusProxy_(dbusProxy)
{
    charWidth_ = new CharWidth();
    punc_ = new Punc(this);
    special_ = new Special();
    chttrans_ = new Chttrans();
    candidateList_ = new CandidateList(this);
    committer_ = new Committer(std::move(commitCallback), this);
    engineManager_ = new EngineManager(candidateList_, committer_);
    stateManager_ = new StateManager(this);
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
    if (charWidth_ != nullptr)
    {
        delete charWidth_;
        charWidth_ = nullptr;
    }
    if (special_ != nullptr)
    {
        delete special_;
        special_ = nullptr;
    }
    if (stateManager_ != nullptr)
    {
        delete stateManager_;
        stateManager_ = nullptr;
    }
}

void Freewb::activate()
{
    dbusProxy_->callPanelShowToolbar();
}

void Freewb::deactivate()
{
    reset();
    dbusProxy_->callPanelHideToolbar();
    dbusProxy_->callPanelUpdatePreeditText({.text = "", .caret = 0});
    dbusProxy_->callPanelUpdateCandidate(
        {.fullCodes = {}, .texts = {}, .prompts = {}, .hasPrev = false, .hasNext = false, .cursor = -1});
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

CharWidth *Freewb::charWidth() const
{
    return charWidth_;
}

ipc::IDBus *Freewb::dbusProxy() const
{
    return dbusProxy_;
}

Committer *Freewb::committer() const
{
    return committer_;
}

Chttrans *Freewb::chttrans() const
{
    return chttrans_;
}

Special *Freewb::special() const
{
    return special_;
}

bool Freewb::processKeyPress(FreewbKeySym keysym, FreewbKeyState state)
{
    FREEWB_DEBUG("keysym: {}, state: {}", static_cast<int>(keysym), static_cast<int>(state));

    bool processed = false;
    processed = stateManager_->processKey(keysym, state);
    if (processed)
    {
        return true;
    }

    processed = handleSingleKey(keysym, state);
    if (processed)
    {
        return true;
    }

    processed = handleComboKey(keysym, state);
    if (processed)
    {
        return true;
    }

    processed = engineManager_->processKey(keysym, state);
    if (processed)
    {
        return true;
    }

    processed = handleDirectSymbolKey(keysym, state);
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

bool Freewb::processKeyRelease(FreewbKeySym keysym, FreewbKeyState state)
{
    (void)state;

    const FreewbKeyState switchMod =
        Key::modifierStateFromKeySym(Key::keySymFromUniqueName(settings::instance().get_cnEnSwitch().c_str()));
    if (switchMod == FreewbKeyState_None || Key::modifierStateFromKeySym(keysym) != switchMod || !cnEnSwitchKeyPending_)
    {
        cnEnSwitchKeyPending_ = false;
        return false;
    }

    cnEnSwitchKeyPending_ = false;

    engineManager_->toggleEnglishEngine();
    const char *engineName = engineManager_->currentEngineName();
    if (engineName != nullptr)
    {
        dbusProxy_->callPanelSwitchInputModeMethod(engineName);
    }
    return true;
}

void Freewb::reset()
{
    cnEnSwitchKeyPending_ = false;
    stateManager_->reset();
    engineManager_->reset();
    candidateList_->clear();
    punc_->reset();
}

void Freewb::reloadConfig()
{
    settings::instance().reload();

    committer_->loadSettings();
    candidateList_->loadSettings();
    charWidth_->loadSettings();
    punc_->loadSettings();
    chttrans_->loadSettings();

    updateCandidateAndPreeditToUI();
}

bool Freewb::handleComboKey(FreewbKeySym keysym, FreewbKeyState state)
{
    if (settings::instance().get_disableAllShortcutKey())
    {
        return false;
    }

    if (!settings::instance().get_disableFullHalfSwitch() && keysym == FreewbKey_space && state == FreewbKeyState_Shift)
    {
        charWidth_->changeAvailable();
        dbusProxy_->callPanelSwitchCharWidthMethod();
        return true;
    }

    if (keysym == FreewbKey_period && state == FreewbKeyState_Ctrl)
    {
        const char *engineName = engineManager_->currentEngineName();
        if (engineName != nullptr && std::strcmp(engineName, "engine:en") == 0)
        {
            return true;
        }
        punc_->changeAvailable();
        dbusProxy_->callPanelSwitchPunctuationModeMethod();
        return true;
    }

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
        const char *keyString = Key::readKeyString(settings::instance().get_backFindCode().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        const FreewbKeyState wantState = static_cast<FreewbKeyState>(FreewbKeyState_Ctrl | FreewbKeyState_Alt);
        if (keysym == keySym && state == wantState)
        {
            // 查询剪贴板文本
            const std::string clipText = dbusProxy_->callGetClipboardMethod();
            if (!clipText.empty())
            {
                dbusProxy_->callDictQueryMethod(clipText);
            }
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_markAutoPair().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            punc_->toggleAutoPair();
            dbusProxy_->callSwitchMarkAutoPairsFlgMethod();
            return true;
        }
    }
    {

        const char *keyString = Key::readKeyString(settings::instance().get_onlineAddWord().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            // 在线造词，使用历史上屏的文本
            return stateManager_->enterAddPhraseState(false);
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_onlineAddWord().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        const FreewbKeyState wantState = static_cast<FreewbKeyState>(FreewbKeyState_Ctrl | FreewbKeyState_Alt);
        if (keysym == keySym && state == wantState)
        {
            // 在线造词，使用剪贴板的文本
            return stateManager_->enterAddPhraseState(true);
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_onlineDelWord().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            // 在线删词
            const std::string &delText = committer_->lastCommitString();
            const std::string &delCode = committer_->lastCommitCode();
            return stateManager_->enterDeletePhraseState(delText, delCode);
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
            engineManager_->toggleCharset();
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
        const char *keyString = Key::readKeyString(settings::instance().get_switchVKb().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            dbusProxy_->callSwitchVirtualKeyboardModeMethod(0);
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_showHideToolbar().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            dbusProxy_->callSwitchToolbarHideFlgMethod();
            return true;
        }
    }

    return false;
}

bool Freewb::handleSingleKey(FreewbKeySym keysym, FreewbKeyState state)
{
    const FreewbKeyState switchMod =
        Key::modifierStateFromKeySym(Key::keySymFromUniqueName(settings::instance().get_cnEnSwitch().c_str()));
    if (switchMod != FreewbKeyState_None)
    {
        const FreewbKeyState keyMod = Key::modifierStateFromKeySym(keysym);
        if (!Key::isModifierKeySym(keysym))
        {
            cnEnSwitchKeyPending_ = false;
        }
        else if (keyMod == switchMod && state == FreewbKeyState_None)
        {
            cnEnSwitchKeyPending_ = true;
        }
    }

    if (state != FreewbKeyState_None)
    {
        return false;
    }

    if (keysym == FreewbKey_Escape)
    {
        reset();
        return false;
    }

    const FreewbKeySym prevPageKey = Key::keySymFromUniqueName(settings::instance().get_prevPageKey().c_str());
    if (keysym == prevPageKey)
    {
        if (!candidateList_->hasPrev())
        {
            return false;
        }
        candidateList_->prev();
        return true;
    }

    const FreewbKeySym nextPageKey = Key::keySymFromUniqueName(settings::instance().get_nextPageKey().c_str());
    if (keysym == nextPageKey)
    {
        if (!candidateList_->hasNext())
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
            committer_->handleCommittedBackspace();
            return false;
        }

        candidateList_->popPreeditText();
        engineManager_->refreshEngineResult();
        return true;
    }

    {
        const char *keyString = Key::readKeyString(settings::instance().get_tempEnglish().c_str());
        const FreewbKeySym tempEnglishKey = Key::keySymFromUniqueName(keyString);
        if (keysym == tempEnglishKey)
        {
            const char *const commandPrefix = Key::keySymToName(keysym);
            return stateManager_->enterTempEnglishState(commandPrefix != nullptr ? std::string(commandPrefix) : std::string());
        }
    }

    return false;
}

bool Freewb::handleDirectSymbolKey(FreewbKeySym keysym, FreewbKeyState state)
{
    CandidateList *candidates = candidateList_;
    if (candidates->size() != 0 || !candidates->preeditText().empty())
    {
        return false;
    }

    if (Key::isKey09(keysym, state))
    {
        committer_->commit(std::string(1, static_cast<char>(keysym)));
        return true;
    }

    const FreewbKeySym sym = Key::normalizedKeySymbol(keysym, state);
    if (sym == FreewbKey_None)
    {
        return false;
    }

    const PuncPushResult result = punc_->convert(keysym, state);
    if (result.empty())
    {
        return false;
    }

    committer_->commit(result.joined());
    return true;
}

void Freewb::connectDBusCallback()
{
    dbusProxy_->bindDBusSignalCallback(
        [this](const PanelSignalEvent &evt)
        {
            if (evt.member == nullptr)
            {
                return;
            }
            if (std::strcmp(evt.member, "CommitUserWordAdd") == 0)
            {
                (void)this->engineManager_->addUserWord(evt.str0, evt.str1);
            }
            else if (std::strcmp(evt.member, "SelectCandidate") == 0)
            {
                this->committer_->selectCandidate(evt.index);
            }
            else if (std::strcmp(evt.member, "LookupTablePageUp") == 0)
            {
                this->candidateList_->prev();
                this->updateCandidateAndPreeditToUI();
            }
            else if (std::strcmp(evt.member, "LookupTablePageDown") == 0)
            {
                this->candidateList_->next();
                this->updateCandidateAndPreeditToUI();
            }
            else if (std::strcmp(evt.member, "ReloadConfig") == 0)
            {
                this->reloadConfig();
            }
            else if (std::strcmp(evt.member, "ReloadDictionaries") == 0)
            {
                this->engineManager_->reloadDictionaries(evt.index);
            }
            else if (std::strcmp(evt.member, "RequestNextInputMode") == 0)
            {
                this->engineManager_->nextEngine();
                const std::string nextEngine = this->engineManager_->currentEngineName();
                this->dbusProxy_->callPanelSwitchInputModeMethod(nextEngine.c_str());
            }
            else if (std::strcmp(evt.member, "SwitchPunctuation") == 0)
            {
                this->punc_->changeAvailable();
            }
            else if (std::strcmp(evt.member, "SwitchFullWidth") == 0)
            {
                this->charWidth_->changeAvailable();
            }
            else if (std::strcmp(evt.member, "SwitchCharSetMode") == 0)
            {
                this->engineManager_->toggleCharset();
            }
            else if (std::strcmp(evt.member, "SwitchChttrans") == 0)
            {
                this->chttrans_->changeAvailable();
            }
        });
}

void Freewb::updateCandidateAndPreeditToUI()
{
    // 用户造词/删词状态，不更新候选窗UI
    // 由状态机更新造词/删词的相关信息到UI
    if (stateManager_->isUserPhraseState())
    {
        return;
    }
    dbusProxy_->callPanelUpdatePreeditText({.text = candidateList_->preeditText(), .caret = candidateList_->cursor()});
    dbusProxy_->callPanelUpdatePreeditCaret(candidateList_->cursor());
    dbusProxy_->callPanelUpdateCandidate({.fullCodes = {},
                                          .texts = candidateList_->candidateTexts(),
                                          .prompts = candidateList_->candidatePrompts(),
                                          .hasPrev = candidateList_->hasPrev(),
                                          .hasNext = candidateList_->hasNext(),
                                          .cursor = -1});
}

} // namespace freewb