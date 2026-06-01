#ifndef UKUI_WAYLAND_HELPER_H
#define UKUI_WAYLAND_HELPER_H

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <wayland-client.hpp>

#include "kde-output-device-v2.h"
#include "ukui-window-management.h"

namespace freewb
{

struct OutputInfo
{
    wayland::kde_output_device_v2_t device;
    std::string uuid;
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
    double scale = 1.0;
    uint32_t registry_name = 0;
};

struct WindowInfo
{
    wayland::ukui_window_t window;
    std::string uuid;
    int32_t x = -1;
    int32_t y = -1;
    bool is_active = false;
};

class UkuiWaylandHelper
{
public:
    UkuiWaylandHelper();
    ~UkuiWaylandHelper();

    const std::array<int32_t, 2> &focusWindowPosition() const;
    double maxScreenScaleFactor() const;

private:
    void connectToWayland();
    void disconnectFromWayland();
    void dispatchLoop();
    void handleRegistryGlobal(uint32_t name, const std::string &interface, uint32_t version);
    void handleRegistryGlobalRemove(uint32_t name);
    void handleWindowCreated(const std::string &uuid);
    void setupOutputListeners(OutputInfo &output);
    void setupWindowListeners(WindowInfo &info);
    WindowInfo *findWindow(const wayland::ukui_window_t &window);

private:
    std::unique_ptr<wayland::display_t> display_;
    wayland::registry_t registry_;
    wayland::ukui_window_management_t window_management_;
    bool has_window_management_ = false;

    std::vector<WindowInfo> windows_;
    std::vector<OutputInfo> outputs_;

    std::array<int32_t, 2> focus_window_position_{0, 0};

    std::thread event_thread_;
    std::atomic<bool> running_{false};
};

} // namespace freewb

#endif
