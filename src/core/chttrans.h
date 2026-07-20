#ifndef CHTTRANS_H
#define CHTTRANS_H

#include <memory>
#include <string>

#include "ifreewb.h"
#include "opencc/opencc.h"

namespace freewb
{

class Freewb;

class Chttrans final : public IFreewb
{
public:
    explicit Chttrans(Freewb *freewb = nullptr);
    ~Chttrans() override = default;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    void simpToTrad(std::string &text) const;

private:
    void loadPair(const std::string &s2tProfile, const std::string &t2sProfile);
    void notifyToolbarProperty() const;

private:
    Freewb *freewb_ = nullptr;
    std::unique_ptr<opencc::SimpleConverter> s2t_;
    bool available_ = true;
};

} // namespace freewb

#endif /* CHTTRANS_H */
