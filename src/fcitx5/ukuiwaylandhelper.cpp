#include "ukuiwaylandhelper.h"

#include <algorithm>

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
        FREEWB_DEBUG("UkuiWaylandHelper: failed to connect to wayland display");
        return;
    }

    registry_ = display_->get_registry();
    registry_.on_global() = [this](uint32_t name, std::string interface, uint32_t version)
    { handleRegistryGlobal(name, interface, version); };
    registry_.on_global_remove() = [this](uint32_t name) { handleRegistryGlobalRemove(name); };

    display_->roundtrip();

    running_ = true;
    event_thread_ = std::thread([this]() { dispatchLoop(); });
}

void UkuiWaylandHelper::disconnectFromWayland()
{
    running_ = false;

    if (display_)
    {
        display_->flush();
    }

    if (event_thread_.joinable())
    {
        event_thread_.join();
    }

    windows_.clear();
    outputs_.clear();
    has_window_management_ = false;
    window_management_ = wayland::ukui_window_management_t{};
    registry_ = wayland::registry_t{};
    display_.reset();
}

void UkuiWaylandHelper::dispatchLoop()
{
    while (running_ && display_)
    {
        if (display_->dispatch() < 0)
        {
            break;
        }
        display_->flush();
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

        display_->roundtrip();
    }
    else if (interface == wayland::kde_output_device_v2_t::interface_name)
    {
        const uint32_t bind_version = std::min(version, 2u);
        wayland::kde_output_device_v2_t iface;
        OutputInfo output;
        output.device = wayland::kde_output_device_v2_t(registry_.bind(name, iface, bind_version));
        output.registry_name = name;
        setupOutputListeners(output);
        outputs_.push_back(std::move(output));

        display_->roundtrip();
    }
}

void UkuiWaylandHelper::handleRegistryGlobalRemove(uint32_t name)
{
    outputs_.erase(std::remove_if(outputs_.begin(), outputs_.end(),
                                  [name](const OutputInfo &output) { return output.registry_name == name; }),
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
    setupWindowListeners(info);
    windows_.push_back(std::move(info));

    display_->roundtrip();
}

void UkuiWaylandHelper::setupOutputListeners(OutputInfo &output)
{
    output.device.on_geometry() = [&output](int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t,
                                            std::string, std::string, int32_t)
    {
        output.x = x;
        output.y = y;
        output.width = physical_width;
        output.height = physical_height;
    };

    output.device.on_scale() = [&output](double factor) { output.scale = factor; };

    output.device.on_uuid() = [&output](std::string uuid) { output.uuid = std::move(uuid); };
}

void UkuiWaylandHelper::setupWindowListeners(WindowInfo &info)
{
    info.window.on_state_changed() = [this, window = info.window](uint32_t flags)
    {
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

const std::array<int32_t, 2> &UkuiWaylandHelper::focusWindowPosition() const
{
    return focus_window_position_;
}

double UkuiWaylandHelper::maxScreenScaleFactor() const
{
    double max_scale = 1.0;
    for (const auto &output : outputs_)
    {
        if (output.width < 0 || output.height < 0 || output.scale <= 0.0)
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
