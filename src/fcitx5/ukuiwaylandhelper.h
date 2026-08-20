#ifndef UKUI_WAYLAND_HELPER_H
#define UKUI_WAYLAND_HELPER_H

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <wayland-client.h>

struct ukui_window_management;
struct ukui_window;
struct kde_output_device_v2;
struct kde_output_device_mode_v2;

namespace freewb
{

struct OutputInfo
{
    struct kde_output_device_v2 *device = nullptr;
    std::vector<struct kde_output_device_mode_v2 *> modes;
    double scale = 1.0;
    bool enabled = true;
    uint32_t registry_name = 0;
};

struct WindowInfo
{
    struct ukui_window *window = nullptr;
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

    std::array<int32_t, 2> focusWindowPosition() const;
    double maxScreenScaleFactor() const;

private:
    void connectToWayland();
    void disconnectFromWayland();
    void dispatchLoop();
    void handleRegistryGlobal(uint32_t name, const char *interface, uint32_t version);
    void handleRegistryGlobalRemove(uint32_t name);
    void handleWindowCreated(const char *uuid);
    void setupOutputListeners(OutputInfo &output);
    void setupWindowListeners(WindowInfo &info);
    void destroyOutput(OutputInfo &output);
    void destroyWindow(WindowInfo &info);
    WindowInfo *findWindow(struct ukui_window *window);
    OutputInfo *findOutputByDevice(struct kde_output_device_v2 *device);

    // wl_registry
    static void registryGlobal(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    static void registryGlobalRemove(void *data, struct wl_registry *registry, uint32_t name);

    // ukui_window_management
    static void windowManagementShowDesktopChanged(void *data, struct ukui_window_management *management, uint32_t state);
    static void windowManagementStackingOrderChanged(void *data, struct ukui_window_management *management, const char *uuids);
    static void windowManagementWindowCreated(void *data, struct ukui_window_management *management, const char *uuid);

    // ukui_window（仅实现用到的事件，其余用空回调填充 listener）
    static void windowStateChanged(void *data, struct ukui_window *window, uint32_t flags);
    static void windowUnmapped(void *data, struct ukui_window *window);
    static void windowGeometry(void *data, struct ukui_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height);

    // kde_output_device_v2
    static void outputMode(void *data, struct kde_output_device_v2 *device, struct kde_output_device_mode_v2 *mode);
    static void outputScale(void *data, struct kde_output_device_v2 *device, wl_fixed_t factor);
    static void outputEnabled(void *data, struct kde_output_device_v2 *device, int32_t enabled);

    // kde_output_device_mode_v2
    static void modeRemoved(void *data, struct kde_output_device_mode_v2 *mode);

private:
    struct wl_display *display_ = nullptr;
    struct wl_registry *registry_ = nullptr;
    struct ukui_window_management *window_management_ = nullptr;

    std::vector<WindowInfo> windows_;
    std::vector<OutputInfo> outputs_;

    std::array<int32_t, 2> focus_window_position_{0, 0};
    mutable std::mutex state_mutex_;

    std::thread event_thread_;
    std::atomic<bool> running_{false};
    int wakeup_fd_ = -1;
};

} // namespace freewb

#endif
