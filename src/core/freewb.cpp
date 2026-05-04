#include "freewb.h"

#include <utility>

#include "key.h"
#include "settings.h"

namespace freewb
{

Freewb::Freewb(void *sd_event_handle, CommitCallback commitCallback) : log_("/tmp/freewb-engine.log")
{
    sdbusProxy_ = new ipc::SDBusProxy(sd_event_handle);
    punc_ = new Punc();
    candidateList_ = new CandidateList();
    engineManager_ = new EngineManager(candidateList_);
    committer_ = new Committer(std::move(commitCallback), candidateList_, engineManager_, punc_);
}

Freewb::~Freewb()
{
    if (sdbusProxy_ != nullptr)
    {
        delete sdbusProxy_;
        sdbusProxy_ = nullptr;
    }
    if (engineManager_ != nullptr)
    {
        delete engineManager_;
        engineManager_ = nullptr;
    }
    if (candidateList_ != nullptr)
    {
        delete candidateList_;
        candidateList_ = nullptr;
    }
    if (punc_ != nullptr)
    {
        delete punc_;
        punc_ = nullptr;
    }
}

void Freewb::activate()
{
    sdbusProxy_->emitShowToolbar();
}

void Freewb::deactivate()
{
    engineManager_->reset();
    candidateList_->clear();
    sdbusProxy_->emitHideToolbar();
    sdbusProxy_->emitUpdatePreeditText({.text = "", .caret = 0, .show = false});
    sdbusProxy_->emitUpdateCandidate({.labels = {}, .texts = {}, .attrs = {}, .hasPrev = false, .hasNext = false, .cursor = -1, .layout = Horizontal});
}

ipc::SDBusProxy *Freewb::sdbusProxy() const
{
    return sdbusProxy_;
}

Punc *Freewb::punc() const
{
    return punc_;
}

bool Freewb::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    FREEWB_DEBUG("keysym: {}, state: {}", static_cast<int>(keysym), static_cast<int>(state));
    bool processed = false;
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
    engineManager_->reset();
    candidateList_->clear();
}

bool Freewb::handleGlobalShortcutKey(FreewbKeySym keysym, FreewbKeyState state)
{
    {
        const char *keyString = Key::readKeyString(settings::instance().get_backFindCode().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            sdbusProxy_->callDictQueryMethod(committer_->lastCommitString());
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_markAutoPair().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            //make mark auto pair
            punc_->changeAvailable();
            return true;
        }

    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_onlineAddWord().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            sdbusProxy_->callAddUsrParseMethod(0, "", "");
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_onlineDelWord().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            sdbusProxy_->callDeleteUsrParseMethod(0, "", "");
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
            sdbusProxy_->callOpenUiSettingMethod();
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchCharSet().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            sdbusProxy_->callSwitchCharSetMethod();
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchChttrans().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            sdbusProxy_->callSwitchChttransMethod();
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
            sdbusProxy_->callSwitchInputModeMethod(nextEngine);
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchLexicon().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            sdbusProxy_->callSwitchTableMethod();
            return true;
        }
    }
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchSkin().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            sdbusProxy_->callSwitchSkinMethod();
            return true;
        }
    }
    
    {
        const char *keyString = Key::readKeyString(settings::instance().get_switchVKb().c_str());
        const FreewbKeySym keySym = Key::keySymFromUniqueName(keyString);
        if (keysym == keySym && state == FreewbKeyState_Ctrl)
        {
            sdbusProxy_->callSwitchVirtualKeyboardModeMethod(0);
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

void Freewb::updateCandidateAndPreeditToUI()
{
    sdbusProxy_->emitUpdatePreeditText({.text = candidateList_->preeditText(), .caret = candidateList_->cursor(), .show = !candidateList_->preeditText().empty()});
    sdbusProxy_->emitUpdatePreeditCaret(candidateList_->cursor());
    sdbusProxy_->emitUpdateCandidate({.labels = {}, .texts = candidateList_->candidateTexts(), .attrs = {}, .hasPrev = candidateList_->hasPrev(), .hasNext = candidateList_->hasNext(), .cursor = -1, .layout = Horizontal});
}

} // namespace freewb