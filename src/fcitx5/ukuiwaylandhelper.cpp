#include "ukuiwaylandhelper.h"

#include <poll.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <system_error>

#include <sys/eventfd.h>

#include "log.h"

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
    try
    {
        display_ = std::make_unique<wayland::display_t>();
    }
    catch (...)
    {
        FREEWB_ERROR("UkuiWaylandHelper: failed to connect to wayland display");
        return;
    }

    registry_ = display_->get_registry();
    registry_.on_global() = [this](uint32_t name, std::string interface, uint32_t version)
    { handleRegistryGlobal(name, interface, version); };
    registry_.on_global_remove() = [this](uint32_t name) { handleRegistryGlobalRemove(name); };

    display_->roundtrip();

    wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_fd_ < 0)
    {
        FREEWB_ERROR("UkuiWaylandHelper: failed to create eventfd, errno={}", errno);
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
        windows_.clear();
        outputs_.clear();
    }
    has_window_management_ = false;
    window_management_ = wayland::ukui_window_management_t{};
    registry_ = wayland::registry_t{};
    display_.reset();
}

void UkuiWaylandHelper::dispatchLoop()
{
    while (running_.load(std::memory_order_acquire) && display_)
    {
        try
        {
            display_->flush();

            auto readIntent = display_->obtain_read_intent();

            struct pollfd pollFds[2];
            pollFds[0].fd = display_->get_fd();
            pollFds[0].events = POLLIN;
            pollFds[0].revents = 0;
            pollFds[1].fd = wakeup_fd_;
            pollFds[1].events = POLLIN;
            pollFds[1].revents = 0;

            const int pollRet = poll(pollFds, 2, -1);

            if (!running_)
            {
                readIntent.cancel();
                break;
            }

            if (pollFds[1].revents & POLLIN)
            {
                uint64_t counter = 0;
                while (read(wakeup_fd_, &counter, sizeof(counter)) > 0)
                {
                }
                readIntent.cancel();
                continue;
            }

            if (pollRet > 0 && (pollFds[0].revents & POLLIN))
            {
                readIntent.read();
                while (running_)
                {
                    if (display_->dispatch_pending() <= 0)
                    {
                        break;
                    }
                }
            }
            else
            {
                readIntent.cancel();
                if (pollRet < 0 && errno != EINTR)
                {
                    break;
                }
            }
        }
        catch (const std::system_error &e)
        {
            FREEWB_ERROR("UkuiWaylandHelper dispatchLoop: system_error code={} what={}", e.code().value(), e.what());
            break;
        }
    }
}

void UkuiWaylandHelper::handleRegistryGlobal(uint32_t name, const std::string &interface, uint32_t version)
{
    if (interface == wayland::ukui_window_management_t::interface_name)
    {
        const uint32_t bind_version = std::min(version, 1u);
        wayland::ukui_window_management_t iface;
        window_management_ = wayland::ukui_window_management_t(registry_.bind(name, iface, bind_version));
        has_window_management_ = true;

        window_management_.on_window_created() = [this](std::string uuid) { handleWindowCreated(uuid); };
    }
    else if (interface == wayland::kde_output_device_v2_t::interface_name)
    {
        const uint32_t bind_version = std::min(version, 2u);
        wayland::kde_output_device_v2_t iface;
        wayland::kde_output_device_v2_t device(registry_.bind(name, iface, bind_version));

        std::lock_guard lock(state_mutex_);
        outputs_.emplace_back();
        OutputInfo &output = outputs_.back();
        output.device = std::move(device);
        output.registry_name = name;
        setupOutputListeners(output);
    }
}

void UkuiWaylandHelper::handleRegistryGlobalRemove(uint32_t name)
{
    std::lock_guard lock(state_mutex_);
    outputs_.erase(std::remove_if(outputs_.begin(), outputs_.end(),
                                  [name](OutputInfo &output)
                                  {
                                      if (output.registry_name != name)
                                      {
                                          return false;
                                      }
                                      output.modes.clear();
                                      output.device = wayland::kde_output_device_v2_t{};
                                      return true;
                                  }),
                   outputs_.end());
}

void UkuiWaylandHelper::handleWindowCreated(const std::string &uuid)
{
    if (!has_window_management_)
    {
        return;
    }

    WindowInfo info;
    info.uuid = uuid;
    info.window = wayland::ukui_window_t(window_management_.create_window(uuid));
    std::lock_guard lock(state_mutex_);
    windows_.push_back(std::move(info));
    setupWindowListeners(windows_.back());
}

void UkuiWaylandHelper::setupOutputListeners(OutputInfo &output)
{
    const uint32_t registry_name = output.registry_name;

    output.device.on_mode() = [this, registry_name](wayland::kde_output_device_mode_v2_t mode)
    {
        wayland::kde_output_device_mode_v2_t stored_mode = std::move(mode);
        auto *mode_handle = static_cast<kde_output_device_mode_v2 *>(stored_mode);

        stored_mode.on_removed() = [this, registry_name, mode_handle]()
        {
            std::lock_guard lock(state_mutex_);
            OutputInfo *current = findOutput(registry_name);
            if (current == nullptr)
            {
                return;
            }

            current->modes.erase(std::remove_if(current->modes.begin(), current->modes.end(),
                                                [mode_handle](const wayland::kde_output_device_mode_v2_t &stored)
                                                { return static_cast<kde_output_device_mode_v2 *>(stored) == mode_handle; }),
                                 current->modes.end());
        };

        std::lock_guard lock(state_mutex_);
        OutputInfo *entry = findOutput(registry_name);
        if (entry == nullptr)
        {
            return;
        }
        entry->modes.emplace_back(std::move(stored_mode));
    };

    output.device.on_scale() = [this, registry_name](double factor)
    {
        std::lock_guard lock(state_mutex_);
        OutputInfo *entry = findOutput(registry_name);
        if (entry == nullptr)
        {
            return;
        }
        entry->scale = factor;
    };

    output.device.on_enabled() = [this, registry_name](int32_t enabled)
    {
        std::lock_guard lock(state_mutex_);
        OutputInfo *entry = findOutput(registry_name);
        if (entry == nullptr)
        {
            return;
        }
        entry->enabled = enabled != 0;
    };
}

void UkuiWaylandHelper::setupWindowListeners(WindowInfo &info)
{
    info.window.on_state_changed() = [this, window = info.window](uint32_t flags)
    {
        std::lock_guard lock(state_mutex_);
        WindowInfo *entry = findWindow(window);
        if (entry == nullptr)
        {
            return;
        }

        entry->is_active = (flags & static_cast<uint32_t>(wayland::ukui_window_state::active)) != 0;
        if (entry->is_active)
        {
            focus_window_position_ = {entry->x, entry->y};
        }
    };

    info.window.on_geometry() = [this, window = info.window](int32_t x, int32_t y, uint32_t, uint32_t)
    {
        std::lock_guard lock(state_mutex_);
        WindowInfo *entry = findWindow(window);
        if (entry == nullptr)
        {
            return;
        }

        entry->x = x;
        entry->y = y;
        if (entry->is_active)
        {
            focus_window_position_ = {x, y};
        }
    };

    info.window.on_unmapped() = [this, window = info.window]()
    {
        std::lock_guard lock(state_mutex_);
        windows_.erase(std::remove_if(windows_.begin(), windows_.end(),
                                      [&window](const WindowInfo &entry)
                                      { return static_cast<ukui_window *>(entry.window) == static_cast<ukui_window *>(window); }),
                       windows_.end());
    };
}

WindowInfo *UkuiWaylandHelper::findWindow(const wayland::ukui_window_t &window)
{
    for (auto &entry : windows_)
    {
        if (static_cast<ukui_window *>(entry.window) == static_cast<ukui_window *>(window))
        {
            return &entry;
        }
    }
    return nullptr;
}

OutputInfo *UkuiWaylandHelper::findOutput(uint32_t registry_name)
{
    for (auto &output : outputs_)
    {
        if (output.registry_name == registry_name)
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
    double max_scale = 1.0;
    for (const auto &output : outputs_)
    {
        if (!output.enabled || output.scale <= 0.0)
        {
            continue;
        }
        if (output.scale > max_scale)
        {
            max_scale = output.scale;
        }
    }
    return max_scale;
}

} // namespace freewb
