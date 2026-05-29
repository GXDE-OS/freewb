#ifndef CHTTRANS_H
#define CHTTRANS_H

#include <memory>
#include <string>

#include "ifreewb.h"
#include "opencc.h"

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

private:
    void loadPair(const std::string &s2tProfile, const std::string &t2sProfile);

private:
    std::unique_ptr<opencc::SimpleConverter> s2t_;
    bool available_ = true;
};

} // namespace freewb

#endif /* CHTTRANS_H */
