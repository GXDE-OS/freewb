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
    if (settings::instance().get_WbzxEngine())
    {
        wbzxEngine_ = std::unique_ptr<WbzxEngine>(new WbzxEngine());
        const char *wbzxName = wbzxEngine_->name();
        engines_.emplace_back(wbzxName, std::move(wbzxEngine_));
    }

    if (settings::instance().get_PyEngine())
    {
        pyEngine_ = std::unique_ptr<PyEngine>(new PyEngine());
        const char *pyName = pyEngine_->name();
        engines_.emplace_back(pyName, std::move(pyEngine_));
    }

    if (settings::instance().get_EnEngine())
    {
        enEngine_ = std::unique_ptr<En>(new En());
        const char *enName = enEngine_->name();
        engines_.emplace_back(enName, std::move(enEngine_));
    }

    if (settings::instance().get_WbpyEngine() && settings::instance().get_WbzxEngine() && settings::instance().get_PyEngine())
    {
        wbpyEngine_ = std::unique_ptr<Wbpy>(new Wbpy(wbzxEngine_.get(), pyEngine_.get()));
        const char *wbpyName = wbpyEngine_->name();
        engines_.emplace_back(wbpyName, std::move(wbpyEngine_));
    }
}

void EngineManager::loadDefaultEngines()
{
    static constexpr int kInputModeWbzx = 0;
    static constexpr int kInputModeWbpy = 1;
    static constexpr int kInputModePinyin = 2;
    static constexpr int kInputModeEn = 3;

    const char *name = nullptr;
    switch (settings::instance().get_inputMode())
    {
    case kInputModeWbzx:
        name = "engine:wbzx";
        break;
    case kInputModeWbpy:
        name = "engine:wbpy";
        break;
    case kInputModePinyin:
        name = "engine:py";
        break;
    case kInputModeEn:
        name = "engine:en";
        break;
    default:
        break;
    }

    currentEngine_ = findEngineByName(name);
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
    if (currentEngine_ == nullptr)
    {
        return false;
    }
    auto *engine = dynamic_cast<IFreewbEngine *>(currentEngine_);
    if (engine == nullptr)
    {
        return false;
    }

    if (!Key::isKeyaz(keysym, state) && !Key::isKeyAZ(keysym, state))
    {
        return false;
    }

    const char *key = Key::keySymToString(keysym);
    if (key == nullptr)
    {
        return false;
    }
    candidateList_->setPreeditText(candidateList_->preeditText() + key);
    if (static_cast<int>(candidateList_->preeditText().length()) > engine->inputCodeLength())
    {
        candidateList_->clear();
        engine->reset();
        return true;
    }

    engine->putKey(candidateList_->preeditText().c_str());
    candidateList_->setCandidateTexts(currentEngine_->getResult().texts);
    return true;
}

void EngineManager::reset()
{
    candidateList_->clear();
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