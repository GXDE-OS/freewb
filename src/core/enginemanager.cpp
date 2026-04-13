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
    initEngines();
}

EngineManager::~EngineManager() = default;

void EngineManager::initEngines()
{
    wbzxEngine_ = std::make_unique<WbzxEngine>();
    pyEngine_ = std::make_unique<PyEngine>();

    engines_.emplace_back(wbzxEngine_->name(), std::move(wbzxEngine_));
    engines_.emplace_back(pyEngine_->name(), std::move(pyEngine_));

    if (settings::instance().get_inputMode() == 0)
    {
        currentEngine_ = engines_[0].second.get();
    }
    else if (settings::instance().get_inputMode() == 1)
    {
        currentEngine_ = engines_[1].second.get();
    }
    else
    {
        currentEngine_ = nullptr;
    }
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

    if (!Key::isKeyaz(keysym, state) || Key::isModifierKeySym(keysym))
    {
        return false;
    }

    const char *key = Key::keySymToString(keysym);
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
    for (const auto &engine : engines_)
    {
        if (engine.first != engineName)
        {
            continue;
        }
        currentEngine_ = engine.second.get();
        break;
    }
}

} // namespace freewb