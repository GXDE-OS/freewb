#include "enginemanager.h"

#include <cstring>
#include <string>

#include "ifreewb.h"
#include "key.h"
#include "log.h"
#include "settings.h"
#include "wbpy.h"

namespace freewb
{

EngineManager::EngineManager(CandidateList *candidateList, Committer *committer)
    : candidateList_(candidateList), committer_(committer)
{
    initAllEngines();
    loadDefaultEngines();
}

EngineManager::~EngineManager() = default;

void EngineManager::initAllEngines()
{
    // en engine is always available,but not added to engines vector.
    enEngine_ = std::unique_ptr<En>(new En());

    // wbzx engine is available if config is true
    if (settings::instance().get_WbzxEngine())
    {
        wbzxEngine_ = std::unique_ptr<WbzxEngine>(new WbzxEngine());
        const char *wbzxName = wbzxEngine_->name();
        engines_.emplace_back(wbzxName, std::move(wbzxEngine_));
    }

    // py engine is available if config is true
    if (settings::instance().get_PyEngine())
    {
        pyEngine_ = std::unique_ptr<PyEngine>(new PyEngine());
        const char *pyName = pyEngine_->name();
        engines_.emplace_back(pyName, std::move(pyEngine_));
    }

    // wbpy engine is available if config is true
    if (settings::instance().get_WbpyEngine() && settings::instance().get_WbzxEngine() && settings::instance().get_PyEngine())
    {
        auto *wbzx = dynamic_cast<WbzxEngine *>(findEngineByName("engine:wbzx"));
        auto *py = dynamic_cast<PyEngine *>(findEngineByName("engine:py"));
        if (wbzx == nullptr || py == nullptr)
        {
            return;
        }

        wbpyEngine_ = std::unique_ptr<Wbpy>(new Wbpy(wbzx, py));
        const char *wbpyName = wbpyEngine_->name();
        engines_.emplace_back(wbpyName, std::move(wbpyEngine_));
    }

    // 五笔字型引擎供拼音单字 [xxxx] 反查
    auto *wbzxForHint = dynamic_cast<WbzxEngine *>(findEngineByName("engine:wbzx"));
    auto *pyForHint = dynamic_cast<PyEngine *>(findEngineByName("engine:py"));
    if (wbzxForHint != nullptr && pyForHint != nullptr)
    {
        pyForHint->setWubiPrimaryCodeLookupCallback([wbzxForHint](const std::string &hz)
                                                    { return wbzxForHint->primaryWubiCodeForSingleHanziUtf8(hz); });
    }

    FREEWB_DEBUG("engines size: {}", engines_.size());
}

void EngineManager::loadDefaultEngines()
{
    const std::string &name = settings::instance().get_inputMode();
    currentEngine_ = findEngineByName(name.c_str());
    if (currentEngine_ == nullptr && !engines_.empty())
    {
        currentEngine_ = engines_[0].second.get();
    }

    FREEWB_DEBUG("current engine: {}", currentEngineName());
}

IFreewbEngine *EngineManager::findEngineByName(const char *name) const
{
    if (name == nullptr)
    {
        return nullptr;
    }
    for (const auto &entry : engines_)
    {
        if (std::strcmp(entry.first, name) == 0)
        {
            return entry.second.get();
        }
    }
    return nullptr;
}

void EngineManager::restoreLastEngine()
{
    IFreewbEngine *targetEngine = lastEngine_;
    if (targetEngine == nullptr)
    {
        if (engines_.empty())
        {
            return;
        }
        targetEngine = engines_[0].second.get();
        const auto *fallbackName = dynamic_cast<const IFreewb *>(targetEngine);
        FREEWB_WARN("restoreLastEngine: lastEngine is null, fallback to {}",
                    fallbackName != nullptr ? fallbackName->name() : "null");
    }

    if (currentEngine_ != nullptr)
    {
        currentEngine_->reset();
    }
    if (candidateList_ != nullptr)
    {
        candidateList_->clear();
    }

    currentEngine_ = targetEngine;
    lastEngine_ = nullptr;
}

void EngineManager::toggleEnglishEngine()
{
    if (candidateList_ == nullptr || enEngine_ == nullptr)
    {
        return;
    }

    if (currentEngine_ == enEngine_.get())
    {
        restoreLastEngine();
        return;
    }

    lastEngine_ = currentEngine_;
    if (currentEngine_ != nullptr)
    {
        currentEngine_->reset();
    }
    candidateList_->clear();
    currentEngine_ = enEngine_.get();
}

void EngineManager::nextEngine()
{
    if (currentEngine_ == enEngine_.get())
    {
        restoreLastEngine();
        return;
    }

    if (engines_.empty() || currentEngine_ == nullptr)
    {
        return;
    }

    lastEngine_ = nullptr;
    currentEngine_->reset();
    candidateList_->clear();

    const auto *asFreewbUi = dynamic_cast<const IFreewb *>(currentEngine_);
    if (asFreewbUi == nullptr)
    {
        return;
    }
    const char *curName = asFreewbUi->name();
    for (size_t i = 0; i < engines_.size(); ++i)
    {
        if (std::strcmp(engines_[i].first, curName) == 0)
        {
            const size_t next = (i + 1U) % engines_.size();
            currentEngine_ = engines_[next].second.get();
            return;
        }
    }
    currentEngine_ = engines_[0].second.get();
}

const char *EngineManager::currentEngineName() const
{
    const auto *engine = dynamic_cast<const IFreewb *>(currentEngine_);
    if (engine == nullptr)
    {
        FREEWB_DEBUG("current engine is nullptr");
        return nullptr;
    }
    return engine->name();
}

void EngineManager::commitPreeditOverflow(const std::string &prefix)
{
    candidateList_->setPreeditText(prefix);
    refreshEngineResult();

    if (committer_ == nullptr || candidateList_->size() == 0)
    {
        reset();
        candidateList_->clear();
    }
    else
    {
        committer_->commit(candidateList_->selectCandidateText(0), candidateList_->selectCandidateFullCode(0));
    }
}

void EngineManager::tryExactDictionarySingleCandidateCommit()
{
    if (committer_ == nullptr || candidateList_->size() == 0)
    {
        return;
    }

    const std::string &pre = candidateList_->preeditText();
    if (candidateList_->totalCandidateCount() != 1 || !isCurrentPreeditExactDictionaryKey(pre))
    {
        return;
    }

    committer_->commit(candidateList_->selectCandidateText(0), candidateList_->selectCandidateFullCode(0));
}

bool EngineManager::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    auto *engine = dynamic_cast<IFreewbEngine *>(currentEngine_);
    if (engine == nullptr)
    {
        return false;
    }

    const char *key = Key::keySymToName(keysym);
    if (key == nullptr || key[0] == '\0')
    {
        return false;
    }

    if (!engine->shouldProcessKey(key))
    {
        return false;
    }

    if (candidateList_ == nullptr)
    {
        return false;
    }

    const std::string pre = candidateList_->preeditText();
    const std::string full = pre + key;

    if (!pre.empty() && engine->isPreeditOverflow(full))
    {
        commitPreeditOverflow(pre);
        candidateList_->setPreeditText(key);
    }
    else
    {
        candidateList_->setPreeditText(full);
    }

    refreshEngineResult();

    tryExactDictionarySingleCandidateCommit();

    return true;
}

void EngineManager::refreshEngineResult()
{
    auto *engine = dynamic_cast<IFreewbEngine *>(currentEngine_);
    if (engine == nullptr)
    {
        return;
    }

    if (candidateList_->preeditText().empty())
    {
        engine->reset();
        return;
    }

    engine->putKey(candidateList_->preeditText().c_str());

    CandidatePayload payload = engine->getResult();
    candidateList_->setCandidates(std::move(payload.texts), std::move(payload.prompts), std::move(payload.fullCodes));
}

void EngineManager::reset()
{
    if (currentEngine_ == nullptr)
    {
        return;
    }
    auto *engine = dynamic_cast<IFreewbEngine *>(currentEngine_);
    if (engine == nullptr)
    {
        return;
    }
    engine->reset();
}

bool EngineManager::isCurrentPreeditExactDictionaryKey(const std::string &preedit) const
{
    const auto *engine = dynamic_cast<const IFreewbEngine *>(currentEngine_);
    if (engine == nullptr)
    {
        return false;
    }
    return engine->isExactDictionaryKey(preedit);
}

std::string EngineManager::calculateWubiPhraseCode(const std::string &phrase) const
{
    auto *wbzx = dynamic_cast<WbzxEngine *>(findEngineByName("engine:wbzx"));
    if (wbzx == nullptr)
    {
        return {};
    }
    return wbzx->calculateWubiPhraseCode(phrase);
}

bool EngineManager::addUserWord(const std::string &code, const std::string &text)
{
    auto *wbzx = dynamic_cast<WbzxEngine *>(findEngineByName("engine:wbzx"));
    if (wbzx == nullptr)
    {
        FREEWB_ERROR("EngineManager::addUserWord: wbzx engine unavailable");
        return false;
    }
    return wbzx->addUserWord(code, text);
}

void EngineManager::addAutoPhrase(const std::string &committedText, const std::string &code)
{
    const char *const engineName = currentEngineName();
    if (engineName == nullptr || (std::strcmp(engineName, "engine:wbzx") != 0 && std::strcmp(engineName, "engine:wbpy") != 0))
    {
        return;
    }

    auto *wbzx = dynamic_cast<WbzxEngine *>(findEngineByName("engine:wbzx"));
    if (wbzx == nullptr)
    {
        FREEWB_DEBUG("EngineManager::addAutoPhrase: wbzx engine unavailable");
        return;
    }

    FREEWB_DEBUG("EngineManager::addAutoPhrase engine={} text={} code={}", engineName, committedText, code);

    wbzx->addAutoPhrase(committedText, code);
}

bool EngineManager::deleteUserWord(const std::string &code, const std::string &text)
{
    auto *wbzx = dynamic_cast<WbzxEngine *>(findEngineByName("engine:wbzx"));
    if (wbzx == nullptr)
    {
        FREEWB_ERROR("EngineManager::deleteUserWord: wbzx engine unavailable");
        return false;
    }
    return wbzx->deleteUserWord(code, text);
}

void EngineManager::changeEngine(const std::string &engineName)
{
    if (engineName.empty())
    {
        return;
    }
    IFreewbEngine *engine = findEngineByName(engineName.c_str());
    if (engine != nullptr)
    {
        currentEngine_ = engine;
        lastEngine_ = nullptr;
    }
}

void EngineManager::reloadDictionaries(int mask)
{
    if (mask == freewb::DictReloadNone)
    {
        return;
    }

    FREEWB_DEBUG("reloadDictionaries mask={:#x}", mask);

    if ((mask & freewb::DictReloadMainTables) != 0)
    {
        settings::instance().reload();
    }

    auto *wbzx = dynamic_cast<WbzxEngine *>(findEngineByName("engine:wbzx"));
    auto *py = dynamic_cast<PyEngine *>(findEngineByName("engine:py"));

    if ((mask & freewb::DictReloadPinyinTable) != 0 && py != nullptr)
    {
        py->reloadMainDictionary();
    }
    if ((mask & freewb::DictReloadWubiTable) != 0 && wbzx != nullptr)
    {
        wbzx->reloadMainDictionary();
    }
    if ((mask & freewb::DictReloadUserWord) != 0 && wbzx != nullptr)
    {
        wbzx->reloadUserDictionary();
    }
    if ((mask & freewb::DictReloadAutoPhrase) != 0 && wbzx != nullptr)
    {
        wbzx->reloadAutoPhraseDictionary();
    }
}

void EngineManager::toggleCharset()
{
    auto *wbzx = dynamic_cast<WbzxEngine *>(findEngineByName("engine:wbzx"));
    if (wbzx != nullptr)
    {
        wbzx->toggleCharset();
    }
}

int EngineManager::charSet() const
{
    auto *wbzx = dynamic_cast<WbzxEngine *>(findEngineByName("engine:wbzx"));
    if (wbzx == nullptr)
    {
        return 0;
    }
    return wbzx->charSet();
}

} // namespace freewb
