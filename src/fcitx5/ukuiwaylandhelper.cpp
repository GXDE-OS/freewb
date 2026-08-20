#include "ukuiwaylandhelper.h"

#include <poll.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

#include <sys/eventfd.h>

#include "kde-output-device-v2-client-protocol.h"
#include "log.h"
#include "ukui-window-management-client-protocol.h"

namespace freewb
{

UkuiWaylandHelper::UkuiWaylandHelper()
{
    connectToWayland();
}

UkuiWaylandHelper::~UkuiWaylandHelper()
{
    disconnectFromWayland();
}

void UkuiWaylandHelper::connectToWayland()
{
    display_ = wl_display_connect(nullptr);
    if (display_ == nullptr)
    {
        FREEWB_ERROR("UkuiWaylandHelper: failed to connect to wayland display");
        return;
    }

    registry_ = wl_display_get_registry(display_);
    if (registry_ == nullptr)
    {
        FREEWB_ERROR("UkuiWaylandHelper: failed to get wl_registry");
        wl_display_disconnect(display_);
        display_ = nullptr;
        return;
    }

    static const struct wl_registry_listener registryListener = {
        .global = registryGlobal,
        .global_remove = registryGlobalRemove,
    };
    wl_registry_add_listener(registry_, &registryListener, this);
    wl_display_roundtrip(display_);

    wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_fd_ < 0)
    {
        FREEWB_ERROR("UkuiWaylandHelper: failed to create eventfd, errno={}", errno);
        disconnectFromWayland();
        return;
    }

    running_ = true;
    event_thread_ = std::thread([this]() { dispatchLoop(); });
}

void UkuiWaylandHelper::disconnectFromWayland()
{
    running_ = false;
    if (wakeup_fd_ >= 0)
    {
        const uint64_t one = 1;
        (void)write(wakeup_fd_, &one, sizeof(one));
    }

    if (event_thread_.joinable())
    {
        event_thread_.join();
    }

    if (wakeup_fd_ >= 0)
    {
        close(wakeup_fd_);
        wakeup_fd_ = -1;
    }

    {
        std::lock_guard lock(state_mutex_);
        for (auto &window : windows_)
        {
            destroyWindow(window);
        }
        windows_.clear();

        for (auto &output : outputs_)
        {
            destroyOutput(output);
        }
        outputs_.clear();
    }

    if (window_management_ != nullptr)
    {
        ukui_window_management_destroy(window_management_);
        window_management_ = nullptr;
    }
    if (registry_ != nullptr)
    {
        wl_registry_destroy(registry_);
        registry_ = nullptr;
    }
    if (display_ != nullptr)
    {
        wl_display_disconnect(display_);
        display_ = nullptr;
    }
}

void UkuiWaylandHelper::dispatchLoop()
{
    while (running_.load(std::memory_order_acquire) && display_ != nullptr)
    {
        while (wl_display_prepare_read(display_) != 0)
        {
            if (wl_display_dispatch_pending(display_) < 0)
            {
                FREEWB_ERROR("UkuiWaylandHelper: wl_display_dispatch_pending failed, errno={}", errno);
                return;
            }
        }

        if (wl_display_flush(display_) < 0 && errno != EAGAIN)
        {
            wl_display_cancel_read(display_);
            FREEWB_ERROR("UkuiWaylandHelper: wl_display_flush failed, errno={}", errno);
            return;
        }

        struct pollfd pollFds[2];
        pollFds[0].fd = wl_display_get_fd(display_);
        pollFds[0].events = POLLIN;
        pollFds[0].revents = 0;
        pollFds[1].fd = wakeup_fd_;
        pollFds[1].events = POLLIN;
        pollFds[1].revents = 0;

        const int pollRet = poll(pollFds, 2, -1);
        if (!running_)
        {
            wl_display_cancel_read(display_);
            break;
        }

        if (pollFds[1].revents & POLLIN)
        {
            uint64_t counter = 0;
            while (read(wakeup_fd_, &counter, sizeof(counter)) > 0)
            {
            }
            wl_display_cancel_read(display_);
            continue;
        }

        if (pollRet < 0)
        {
            wl_display_cancel_read(display_);
            if (errno == EINTR)
            {
                continue;
            }
            FREEWB_ERROR("UkuiWaylandHelper: poll failed, errno={}", errno);
            break;
        }

        if (pollFds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
        {
            wl_display_cancel_read(display_);
            FREEWB_ERROR("UkuiWaylandHelper: wayland display fd error, revents={}", pollFds[0].revents);
            break;
        }

        if (pollFds[0].revents & POLLIN)
        {
            if (wl_display_read_events(display_) < 0)
            {
                FREEWB_ERROR("UkuiWaylandHelper: wl_display_read_events failed, errno={}", errno);
                break;
            }
            if (wl_display_dispatch_pending(display_) < 0)
            {
                FREEWB_ERROR("UkuiWaylandHelper: wl_display_dispatch_pending failed, errno={}", errno);
                break;
            }
        }
        else
        {
            wl_display_cancel_read(display_);
        }
    }
}

void UkuiWaylandHelper::registryGlobal(void *data, struct wl_registry * /*registry*/, uint32_t name, const char *interface,
                                       uint32_t version)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    self->handleRegistryGlobal(name, interface, version);
}

void UkuiWaylandHelper::registryGlobalRemove(void *data, struct wl_registry * /*registry*/, uint32_t name)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    self->handleRegistryGlobalRemove(name);
}

void UkuiWaylandHelper::handleRegistryGlobal(uint32_t name, const char *interface, uint32_t version)
{
    if (interface == nullptr || registry_ == nullptr)
    {
        return;
    }

    if (std::strcmp(interface, ukui_window_management_interface.name) == 0)
    {
        const uint32_t bindVersion = std::min(version, 1u);
        window_management_ = static_cast<struct ukui_window_management *>(
            wl_registry_bind(registry_, name, &ukui_window_management_interface, bindVersion));
        if (window_management_ == nullptr)
        {
            FREEWB_ERROR("UkuiWaylandHelper: failed to bind ukui_window_management");
            return;
        }

        static const struct ukui_window_management_listener listener = {
            .show_desktop_changed = windowManagementShowDesktopChanged,
            .stacking_order_changed = windowManagementStackingOrderChanged,
            .window_created = windowManagementWindowCreated,
        };
        ukui_window_management_add_listener(window_management_, &listener, this);
    }
    else if (std::strcmp(interface, kde_output_device_v2_interface.name) == 0)
    {
        const uint32_t bindVersion = std::min(version, 2u);
        auto *device = static_cast<struct kde_output_device_v2 *>(
            wl_registry_bind(registry_, name, &kde_output_device_v2_interface, bindVersion));
        if (device == nullptr)
        {
            FREEWB_ERROR("UkuiWaylandHelper: failed to bind kde_output_device_v2");
            return;
        }

        std::lock_guard lock(state_mutex_);
        outputs_.emplace_back();
        OutputInfo &output = outputs_.back();
        output.device = device;
        output.registry_name = name;
        setupOutputListeners(output);
    }
}

void UkuiWaylandHelper::handleRegistryGlobalRemove(uint32_t name)
{
    std::lock_guard lock(state_mutex_);
    outputs_.erase(std::remove_if(outputs_.begin(), outputs_.end(),
                                  [this, name](OutputInfo &output)
                                  {
                                      if (output.registry_name != name)
                                      {
                                          return false;
                                      }
                                      destroyOutput(output);
                                      return true;
                                  }),
                   outputs_.end());
}

void UkuiWaylandHelper::windowManagementShowDesktopChanged(void * /*data*/, struct ukui_window_management * /*management*/,
                                                           uint32_t /*state*/)
{
}

void UkuiWaylandHelper::windowManagementStackingOrderChanged(void * /*data*/, struct ukui_window_management * /*management*/,
                                                             const char * /*uuids*/)
{
}

void UkuiWaylandHelper::windowManagementWindowCreated(void *data, struct ukui_window_management * /*management*/,
                                                      const char *uuid)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    self->handleWindowCreated(uuid);
}

void UkuiWaylandHelper::handleWindowCreated(const char *uuid)
{
    if (window_management_ == nullptr || uuid == nullptr)
    {
        return;
    }

    struct ukui_window *window = ukui_window_management_create_window(window_management_, uuid);
    if (window == nullptr)
    {
        FREEWB_ERROR("UkuiWaylandHelper: create_window failed for uuid={}", uuid);
        return;
    }

    WindowInfo info;
    info.uuid = uuid;
    info.window = window;

    std::lock_guard lock(state_mutex_);
    windows_.push_back(std::move(info));
    setupWindowListeners(windows_.back());
}

void UkuiWaylandHelper::setupOutputListeners(OutputInfo &output)
{
    static const struct kde_output_device_v2_listener listener = {
        .geometry = [](void *, struct kde_output_device_v2 *, int32_t, int32_t, int32_t, int32_t, int32_t, const char *,
                       const char *, int32_t) {},
        .current_mode = [](void *, struct kde_output_device_v2 *, struct kde_output_device_mode_v2 *) {},
        .mode = outputMode,
        .done = [](void *, struct kde_output_device_v2 *) {},
        .scale = outputScale,
        .edid = [](void *, struct kde_output_device_v2 *, const char *) {},
        .enabled = outputEnabled,
        .uuid = [](void *, struct kde_output_device_v2 *, const char *) {},
        .serial_number = [](void *, struct kde_output_device_v2 *, const char *) {},
        .eisa_id = [](void *, struct kde_output_device_v2 *, const char *) {},
        .capabilities = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .overscan = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .vrr_policy = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .rgb_range = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .name = [](void *, struct kde_output_device_v2 *, const char *) {},
        .high_dynamic_range = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .sdr_brightness = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .wide_color_gamut = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .auto_rotate_policy = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .icc_profile_path = [](void *, struct kde_output_device_v2 *, const char *) {},
        .brightness_metadata = [](void *, struct kde_output_device_v2 *, uint32_t, uint32_t, uint32_t) {},
        .brightness_overrides = [](void *, struct kde_output_device_v2 *, int32_t, int32_t, int32_t) {},
        .sdr_gamut_wideness = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .color_profile_source = [](void *, struct kde_output_device_v2 *, uint32_t) {},
        .brightness = [](void *, struct kde_output_device_v2 *, uint32_t) {},
    };
    kde_output_device_v2_add_listener(output.device, &listener, this);
}

void UkuiWaylandHelper::setupWindowListeners(WindowInfo &info)
{
    static const struct ukui_window_listener listener = {
        .title_changed = [](void *, struct ukui_window *, const char *) {},
        .app_id_changed = [](void *, struct ukui_window *, const char *) {},
        .state_changed = windowStateChanged,
        .themed_icon_name_changed = [](void *, struct ukui_window *, const char *) {},
        .unmapped = windowUnmapped,
        .initial_state = [](void *, struct ukui_window *) {},
        .parent_window = [](void *, struct ukui_window *, struct ukui_window *) {},
        .geometry = windowGeometry,
        .icon_changed = [](void *, struct ukui_window *) {},
        .pid_changed = [](void *, struct ukui_window *, uint32_t) {},
        .virtual_desktop_entered = [](void *, struct ukui_window *, const char *) {},
        .virtual_desktop_left = [](void *, struct ukui_window *, const char *) {},
        .application_menu = [](void *, struct ukui_window *, const char *, const char *) {},
        .activity_entered = [](void *, struct ukui_window *, const char *) {},
        .activity_left = [](void *, struct ukui_window *, const char *) {},
        .resource_name_changed = [](void *, struct ukui_window *, const char *) {},
    };
    ukui_window_add_listener(info.window, &listener, this);
}

void UkuiWaylandHelper::outputMode(void *data, struct kde_output_device_v2 *device, struct kde_output_device_mode_v2 *mode)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    if (mode == nullptr)
    {
        return;
    }

    static const struct kde_output_device_mode_v2_listener modeListener = {
        .size = [](void *, struct kde_output_device_mode_v2 *, int32_t, int32_t) {},
        .refresh = [](void *, struct kde_output_device_mode_v2 *, int32_t) {},
        .preferred = [](void *, struct kde_output_device_mode_v2 *) {},
        .removed = modeRemoved,
    };
    kde_output_device_mode_v2_add_listener(mode, &modeListener, self);

    std::lock_guard lock(self->state_mutex_);
    OutputInfo *entry = self->findOutputByDevice(device);
    if (entry == nullptr)
    {
        kde_output_device_mode_v2_destroy(mode);
        return;
    }
    entry->modes.push_back(mode);
}

void UkuiWaylandHelper::outputScale(void *data, struct kde_output_device_v2 *device, wl_fixed_t factor)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    std::lock_guard lock(self->state_mutex_);
    OutputInfo *entry = self->findOutputByDevice(device);
    if (entry == nullptr)
    {
        return;
    }
    entry->scale = wl_fixed_to_double(factor);
}

void UkuiWaylandHelper::outputEnabled(void *data, struct kde_output_device_v2 *device, int32_t enabled)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    std::lock_guard lock(self->state_mutex_);
    OutputInfo *entry = self->findOutputByDevice(device);
    if (entry == nullptr)
    {
        return;
    }
    entry->enabled = enabled != 0;
}

void UkuiWaylandHelper::modeRemoved(void *data, struct kde_output_device_mode_v2 *mode)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    std::lock_guard lock(self->state_mutex_);
    for (auto &output : self->outputs_)
    {
        auto it = std::find(output.modes.begin(), output.modes.end(), mode);
        if (it == output.modes.end())
        {
            continue;
        }
        output.modes.erase(it);
        kde_output_device_mode_v2_destroy(mode);
        return;
    }
}

void UkuiWaylandHelper::windowStateChanged(void *data, struct ukui_window *window, uint32_t flags)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    std::lock_guard lock(self->state_mutex_);
    WindowInfo *entry = self->findWindow(window);
    if (entry == nullptr)
    {
        return;
    }

    entry->is_active = (flags & UKUI_WINDOW_STATE_ACTIVE) != 0;
    if (entry->is_active)
    {
        self->focus_window_position_ = {entry->x, entry->y};
    }
}

void UkuiWaylandHelper::windowGeometry(void *data, struct ukui_window *window, int32_t x, int32_t y, uint32_t /*width*/,
                                       uint32_t /*height*/)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    std::lock_guard lock(self->state_mutex_);
    WindowInfo *entry = self->findWindow(window);
    if (entry == nullptr)
    {
        return;
    }

    entry->x = x;
    entry->y = y;
    if (entry->is_active)
    {
        self->focus_window_position_ = {x, y};
    }
}

void UkuiWaylandHelper::windowUnmapped(void *data, struct ukui_window *window)
{
    auto *self = static_cast<UkuiWaylandHelper *>(data);
    std::lock_guard lock(self->state_mutex_);
    self->windows_.erase(std::remove_if(self->windows_.begin(), self->windows_.end(),
                                        [self, window](WindowInfo &entry)
                                        {
                                            if (entry.window != window)
                                            {
                                                return false;
                                            }
                                            self->destroyWindow(entry);
                                            return true;
                                        }),
                         self->windows_.end());
}

void UkuiWaylandHelper::destroyOutput(OutputInfo &output)
{
    for (auto *mode : output.modes)
    {
        if (mode != nullptr)
        {
            kde_output_device_mode_v2_destroy(mode);
        }
    }
    output.modes.clear();

    if (output.device != nullptr)
    {
        kde_output_device_v2_destroy(output.device);
        output.device = nullptr;
    }
}

void UkuiWaylandHelper::destroyWindow(WindowInfo &info)
{
    if (info.window != nullptr)
    {
        ukui_window_destroy(info.window);
        info.window = nullptr;
    }
}

WindowInfo *UkuiWaylandHelper::findWindow(struct ukui_window *window)
{
    for (auto &entry : windows_)
    {
        if (entry.window == window)
        {
            return &entry;
        }
    }
    return nullptr;
}

OutputInfo *UkuiWaylandHelper::findOutputByDevice(struct kde_output_device_v2 *device)
{
    for (auto &output : outputs_)
    {
        if (output.device == device)
        {
            return &output;
        }
    }
    return nullptr;
}

std::array<int32_t, 2> UkuiWaylandHelper::focusWindowPosition() const
{
    std::lock_guard lock(state_mutex_);
    return focus_window_position_;
}

double UkuiWaylandHelper::maxScreenScaleFactor() const
{
    std::lock_guard lock(state_mutex_);
    double maxScale = 1.0;
    for (const auto &output : outputs_)
    {
        if (!output.enabled || output.scale <= 0.0)
        {
            continue;
        }
        if (output.scale > maxScale)
        {
            maxScale = output.scale;
        }
    }
    return maxScale;
}

} // namespace freewb
