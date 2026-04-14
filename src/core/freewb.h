#ifndef _FREEWB_H_
#define _FREEWB_H_

#include <memory>

namespace freewb
{

class Freewb
{
public:
    explicit Freewb(void *sd_event_handle = nullptr);
    ~Freewb();
    void activate();
    void deactivate();
};
} // namespace freewb
#endif