#ifndef PY_H
#define PY_H

#include <string>

#include "common.h"
#include "engine.h"
#include "ifreewb.h"

namespace freewb
{

class PyEngine : public IFreewbEngine, public IFreewb
{
public:
    PyEngine();
    ~PyEngine();

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    void putKey(const char *strCode) override;
    const CandidatePayload &getResult() const override;
    void reset() override;

    int inputCodeLength() const override;
    bool shouldProcessKey(const char *key) const override;

private:
    void clearMbLoadState();
    void loadDictionary();

private:
    MbDictionaryTable mbTable_;
    std::string inputCodes_;
    bool available_ = true;
    CandidatePayload result_;
};

} // namespace freewb

#endif
