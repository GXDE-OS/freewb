#ifndef PUNC_H
#define PUNC_H

#include <string>

#include "ifreewb.h"

namespace freewb
{

static const struct _PuncPair
{
    const char *left;
    const char *right;
} PuncPairList[] = {
    {"[", "]"}, {"{", "}"}, {"【", "】"}, {"（", "）"}, {"《", "》"}, {"<", ">"}, {"(", ")"},
};

class Punc : public IFreewb
{
public:
    Punc();
    ~Punc() = default;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    const std::pair<const char *, const char *> autoPair(const char *key) const;

private:
    bool available_ = true;
};
} // namespace freewb

#endif // _PUNC_H_