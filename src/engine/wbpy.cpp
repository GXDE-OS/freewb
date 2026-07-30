#include "wbpy.h"

#include <algorithm>
#include <unordered_set>

namespace freewb
{

Wbpy::Wbpy(WbzxEngine *wbzxEngine, PyEngine *pyEngine) : wbzxEngine_(wbzxEngine), pyEngine_(pyEngine)
{
}

Wbpy::~Wbpy()
{
}

const char *Wbpy::name() const
{
    return "engine:wbpy";
}

bool Wbpy::available() const
{
    return available_;
}

void Wbpy::changeAvailable()
{
    available_ = !available_;
}

void Wbpy::putKey(const char *strCode)
{
    result_.clearRows();

    if (strCode == nullptr)
    {
        return;
    }

    if (wbzxEngine_ == nullptr || pyEngine_ == nullptr)
    {
        return;
    }

    wbzxEngine_->putKey(strCode);
    pyEngine_->putKey(strCode);

    const CandidatePayload &wb = wbzxEngine_->getResult();
    const CandidatePayload &py = pyEngine_->getResult();

    result_.texts.reserve(wb.texts.size() + py.texts.size());
    result_.fullCodes.reserve(wb.texts.size() + py.texts.size());
    result_.prompts.reserve(wb.texts.size() + py.texts.size());

    std::unordered_set<std::string> seen;
    seen.reserve(wb.texts.size() + py.texts.size());

    for (std::size_t i = 0; i < wb.texts.size(); ++i)
    {
        if (wb.texts[i].empty())
        {
            continue;
        }
        result_.texts.push_back(wb.texts[i]);
        result_.fullCodes.push_back(i < wb.fullCodes.size() ? wb.fullCodes[i] : std::string{});
        result_.prompts.push_back(i < wb.prompts.size() ? wb.prompts[i] : std::string{});
        seen.insert(wb.texts[i]);
    }
    for (std::size_t i = 0; i < py.texts.size(); ++i)
    {
        const std::string &t = py.texts[i];
        if (t.empty() || seen.count(t) != 0U)
        {
            continue;
        }
        seen.insert(t);
        result_.texts.push_back(t);
        result_.fullCodes.push_back(i < py.fullCodes.size() ? py.fullCodes[i] : std::string{});
        result_.prompts.push_back(i < py.prompts.size() ? py.prompts[i] : std::string{});
    }
}

const CandidatePayload &Wbpy::getResult() const
{
    return result_;
}

void Wbpy::reset()
{
    wbzxEngine_->reset();
    pyEngine_->reset();
    result_.clearRows();
}

bool Wbpy::shouldProcessKey(const char *key) const
{
    return wbzxEngine_->shouldProcessKey(key) || pyEngine_->shouldProcessKey(key);
}

bool Wbpy::isExactDictionaryKey(const std::string &preedit) const
{
    if (wbzxEngine_ == nullptr || pyEngine_ == nullptr)
    {
        return false;
    }
    return wbzxEngine_->isExactDictionaryKey(preedit) || pyEngine_->isExactDictionaryKey(preedit);
}

} // namespace freewb
