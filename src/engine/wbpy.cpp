#include "wbpy.h"

#include <algorithm>

namespace freewb
{

Wbpy::Wbpy(WbzxEngine *wbzxEngine, PyEngine *pyEngine) : wbzxEngine_(wbzxEngine), pyEngine_(pyEngine) {}

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
    result_.labels.clear();
    result_.attrs.clear();
    result_.texts.clear();

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
    for (const std::string &t : wb.texts)
    {
        if (!t.empty())
        {
            result_.texts.push_back(t);
        }
    }
    for (const std::string &t : py.texts)
    {
        if (t.empty())
        {
            continue;
        }
        bool duplicate = false;
        for (const std::string &existing : result_.texts)
        {
            if (existing == t)
            {
                duplicate = true;
                break;
            }
        }
        if (!duplicate)
        {
            result_.texts.push_back(t);
        }
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
    result_ = CandidatePayload{};
}

int Wbpy::inputCodeLength() const
{
    return std::min(wbzxEngine_->inputCodeLength(), pyEngine_->inputCodeLength());
}

bool Wbpy::shouldProcessKey(const char *key) const
{
    return wbzxEngine_->shouldProcessKey(key) || pyEngine_->shouldProcessKey(key);
}

} // namespace freewb
