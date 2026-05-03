#include "enginemanager.h"

#include <cstring>

#include "ifreewb.h"
#include "key.h"
#include "log.h"
#include "settings.h"

namespace freewb
{

EngineManager::EngineManager(CandidateList *candidateList) : candidateList_(candidateList)
{
    initAllEngines();
    loadDefaultEngines();
}

EngineManager::~EngineManager() = default;

void EngineManager::initAllEngines()
{
    // en engine is always available
    enEngine_ = std::unique_ptr<En>(new En());
    const char *enName = enEngine_->name();
    engines_.emplace_back(enName, std::move(enEngine_));

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
}

void EngineManager::loadDefaultEngines()
{
    const std::string &name = settings::instance().get_inputMode();
    currentEngine_ = findEngineByName(name.c_str());
    if (currentEngine_ == nullptr && !engines_.empty())
    {
        currentEngine_ = engines_[0].second.get();
    }
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

void EngineManager::nextEngine()
{
    if (engines_.empty() || currentEngine_ == nullptr)
    {
        return;
    }

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

const std::string &EngineManager::currentEngineName() const
{
    if (currentEngine_ == nullptr)
    {
        return "";
    }
    const auto *engine = dynamic_cast<const IFreewb *>(currentEngine_);
    if (engine == nullptr)
    {
        return "";
    }
    return engine->name();
}

bool EngineManager::processKey(FreewbKeySym keysym, FreewbKeyState state)
{
    auto *engine = dynamic_cast<IFreewbEngine *>(currentEngine_);
    if (engine == nullptr)
    {
        return false;
    }

    const char *key = Key::keySymToName(keysym);
    if (key == nullptr)
    {
        return false;
    }

    if (!engine->shouldProcessKey(key))
    {
        return false;
    }

    candidateList_->setPreeditText(candidateList_->preeditText() + key);
    engine->putKey(candidateList_->preeditText().c_str());
    candidateList_->setCandidateTexts(currentEngine_->getResult().texts);

    if (static_cast<int>(candidateList_->preeditText().length()) >= engine->inputCodeLength())
    {
        candidateList_->clear();
        engine->reset();
        return true;
    }
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
    candidateList_->setCandidateTexts(currentEngine_->getResult().texts);
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
    }
}

} // namespace freewb