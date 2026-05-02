#include "utils.h"

#include <vector>

namespace freewb
{
const std::string userFreewbPath()
{
    const char *home = std::getenv("HOME");
    if (!home || !*home)
    {
        return {};
    }
    return std::string(home) + "/.local/freewb";
}
} // namespace freewb