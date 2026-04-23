#ifndef CHTTRANS_H
#define CHTTRANS_H

#include "ifreewb.h"

#include <opencc.h>
#include <memory>
#include <string>

namespace freewb
{

class Chttrans final : public IFreewb
{
public:
    Chttrans();
    ~Chttrans() override = default;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    void simpToTrad(std::string &text) const;
    void tradToSimp(std::string &text) const;

private:
    void loadPair(const std::string &s2tProfile, const std::string &t2sProfile);

private:
    std::unique_ptr<opencc::SimpleConverter> s2t_;
    std::unique_ptr<opencc::SimpleConverter> t2s_;
    bool available_ = true;
};

} // namespace freewb

#endif /* CHTTRANS_H */
