#ifndef _FREEWB_H_
#define _FREEWB_H_

namespace freewb
{
class IFreewb
{
public:
    virtual ~IFreewb() = default;
    virtual const char *name() = 0;
    virtual bool available() = 0;
    virtual void changeAvailable() = 0;
};
} // namespace freewb
#endif