#ifndef WBZX_H
#define WBZX_H

#include <string>

#include "common.h"
#include "engine.h"
#include "ifreewb.h"
#include "userdict.h"

namespace freewb
{

class WbzxEngine : public IFreewbEngine, public IFreewb
{
public:
    WbzxEngine();
    ~WbzxEngine();

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
    UserDict userDict_;
    std::string inputCodes_;
    bool available_ = true;
    CandidatePayload result_;
};
} // namespace freewb

#endif
