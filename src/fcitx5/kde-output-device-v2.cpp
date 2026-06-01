#include <kde-output-device-v2.h>

using namespace wayland;
using namespace wayland::detail;

const wl_interface *kde_output_device_v2_interface_geometry_event[8] = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

const wl_interface *kde_output_device_v2_interface_current_mode_event[1] = {
    &kde_output_device_mode_v2_interface,
};

const wl_interface *kde_output_device_v2_interface_mode_event[1] = {
    &kde_output_device_mode_v2_interface,
};

const wl_interface *kde_output_device_v2_interface_done_event[0] = {};

const wl_interface *kde_output_device_v2_interface_scale_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_edid_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_enabled_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_uuid_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_serial_number_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_eisa_id_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_capabilities_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_overscan_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_vrr_policy_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_rgb_range_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_name_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_high_dynamic_range_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_sdr_brightness_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_wide_color_gamut_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_auto_rotate_policy_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_icc_profile_path_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_brightness_metadata_event[3] = {
    nullptr,
    nullptr,
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_brightness_overrides_event[3] = {
    nullptr,
    nullptr,
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_sdr_gamut_wideness_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_color_profile_source_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_v2_interface_brightness_event[1] = {
    nullptr,
};

const wl_message kde_output_device_v2_interface_requests[0] = {};

const wl_message kde_output_device_v2_interface_events[25] = {
    {
        "geometry",
        "iiiiissi",
        kde_output_device_v2_interface_geometry_event,
    },
    {
        "current_mode",
        "o",
        kde_output_device_v2_interface_current_mode_event,
    },
    {
        "mode",
        "n",
        kde_output_device_v2_interface_mode_event,
    },
    {
        "done",
        "",
        kde_output_device_v2_interface_done_event,
    },
    {
        "scale",
        "f",
        kde_output_device_v2_interface_scale_event,
    },
    {
        "edid",
        "s",
        kde_output_device_v2_interface_edid_event,
    },
    {
        "enabled",
        "i",
        kde_output_device_v2_interface_enabled_event,
    },
    {
        "uuid",
        "s",
        kde_output_device_v2_interface_uuid_event,
    },
    {
        "serial_number",
        "s",
        kde_output_device_v2_interface_serial_number_event,
    },
    {
        "eisa_id",
        "s",
        kde_output_device_v2_interface_eisa_id_event,
    },
    {
        "capabilities",
        "u",
        kde_output_device_v2_interface_capabilities_event,
    },
    {
        "overscan",
        "u",
        kde_output_device_v2_interface_overscan_event,
    },
    {
        "vrr_policy",
        "u",
        kde_output_device_v2_interface_vrr_policy_event,
    },
    {
        "rgb_range",
        "u",
        kde_output_device_v2_interface_rgb_range_event,
    },
    {
        "name",
        "2s",
        kde_output_device_v2_interface_name_event,
    },
    {
        "high_dynamic_range",
        "3u",
        kde_output_device_v2_interface_high_dynamic_range_event,
    },
    {
        "sdr_brightness",
        "3u",
        kde_output_device_v2_interface_sdr_brightness_event,
    },
    {
        "wide_color_gamut",
        "3u",
        kde_output_device_v2_interface_wide_color_gamut_event,
    },
    {
        "auto_rotate_policy",
        "4u",
        kde_output_device_v2_interface_auto_rotate_policy_event,
    },
    {
        "icc_profile_path",
        "5s",
        kde_output_device_v2_interface_icc_profile_path_event,
    },
    {
        "brightness_metadata",
        "6uuu",
        kde_output_device_v2_interface_brightness_metadata_event,
    },
    {
        "brightness_overrides",
        "6iii",
        kde_output_device_v2_interface_brightness_overrides_event,
    },
    {
        "sdr_gamut_wideness",
        "6u",
        kde_output_device_v2_interface_sdr_gamut_wideness_event,
    },
    {
        "color_profile_source",
        "7u",
        kde_output_device_v2_interface_color_profile_source_event,
    },
    {
        "brightness",
        "8u",
        kde_output_device_v2_interface_brightness_event,
    },
};

const wl_interface wayland::detail::kde_output_device_v2_interface = {
    "kde_output_device_v2", 8, 0, kde_output_device_v2_interface_requests, 25, kde_output_device_v2_interface_events,
};

const wl_interface *kde_output_device_mode_v2_interface_size_event[2] = {
    nullptr,
    nullptr,
};

const wl_interface *kde_output_device_mode_v2_interface_refresh_event[1] = {
    nullptr,
};

const wl_interface *kde_output_device_mode_v2_interface_preferred_event[0] = {};

const wl_interface *kde_output_device_mode_v2_interface_removed_event[0] = {};

const wl_message kde_output_device_mode_v2_interface_requests[0] = {};

const wl_message kde_output_device_mode_v2_interface_events[4] = {
    {
        "size",
        "ii",
        kde_output_device_mode_v2_interface_size_event,
    },
    {
        "refresh",
        "i",
        kde_output_device_mode_v2_interface_refresh_event,
    },
    {
        "preferred",
        "",
        kde_output_device_mode_v2_interface_preferred_event,
    },
    {
        "removed",
        "",
        kde_output_device_mode_v2_interface_removed_event,
    },
};

const wl_interface wayland::detail::kde_output_device_mode_v2_interface = {
    "kde_output_device_mode_v2",
    1,
    0,
    kde_output_device_mode_v2_interface_requests,
    4,
    kde_output_device_mode_v2_interface_events,
};

kde_output_device_v2_t::kde_output_device_v2_t(const proxy_t &p) : proxy_t(p)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
    }
    set_interface(&kde_output_device_v2_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return kde_output_device_v2_t(p); });
}

kde_output_device_v2_t::kde_output_device_v2_t()
{
    set_interface(&kde_output_device_v2_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return kde_output_device_v2_t(p); });
}

kde_output_device_v2_t::kde_output_device_v2_t(kde_output_device_v2 *p, wrapper_type t)
    : proxy_t(reinterpret_cast<wl_proxy *>(p), t)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
    }
    set_interface(&kde_output_device_v2_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return kde_output_device_v2_t(p); });
}

kde_output_device_v2_t::kde_output_device_v2_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/)
    : proxy_t(wrapped_proxy, construct_proxy_wrapper_tag())
{
    set_interface(&kde_output_device_v2_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return kde_output_device_v2_t(p); });
}

kde_output_device_v2_t kde_output_device_v2_t::proxy_create_wrapper()
{
    return {*this, construct_proxy_wrapper_tag()};
}

const std::string kde_output_device_v2_t::interface_name = "kde_output_device_v2";

kde_output_device_v2_t::operator kde_output_device_v2 *() const
{
    return reinterpret_cast<kde_output_device_v2 *>(c_ptr());
}

std::function<void(int32_t, int32_t, int32_t, int32_t, int32_t, std::string, std::string, int32_t)> &
kde_output_device_v2_t::on_geometry()
{
    return std::static_pointer_cast<events_t>(get_events())->geometry;
}

std::function<void(kde_output_device_mode_v2_t)> &kde_output_device_v2_t::on_current_mode()
{
    return std::static_pointer_cast<events_t>(get_events())->current_mode;
}

std::function<void(kde_output_device_mode_v2_t)> &kde_output_device_v2_t::on_mode()
{
    return std::static_pointer_cast<events_t>(get_events())->mode;
}

std::function<void()> &kde_output_device_v2_t::on_done()
{
    return std::static_pointer_cast<events_t>(get_events())->done;
}

std::function<void(double)> &kde_output_device_v2_t::on_scale()
{
    return std::static_pointer_cast<events_t>(get_events())->scale;
}

std::function<void(std::string)> &kde_output_device_v2_t::on_edid()
{
    return std::static_pointer_cast<events_t>(get_events())->edid;
}

std::function<void(int32_t)> &kde_output_device_v2_t::on_enabled()
{
    return std::static_pointer_cast<events_t>(get_events())->enabled;
}

std::function<void(std::string)> &kde_output_device_v2_t::on_uuid()
{
    return std::static_pointer_cast<events_t>(get_events())->uuid;
}

std::function<void(std::string)> &kde_output_device_v2_t::on_serial_number()
{
    return std::static_pointer_cast<events_t>(get_events())->serial_number;
}

std::function<void(std::string)> &kde_output_device_v2_t::on_eisa_id()
{
    return std::static_pointer_cast<events_t>(get_events())->eisa_id;
}

std::function<void(kde_output_device_v2_capability)> &kde_output_device_v2_t::on_capabilities()
{
    return std::static_pointer_cast<events_t>(get_events())->capabilities;
}

std::function<void(uint32_t)> &kde_output_device_v2_t::on_overscan()
{
    return std::static_pointer_cast<events_t>(get_events())->overscan;
}

std::function<void(kde_output_device_v2_vrr_policy)> &kde_output_device_v2_t::on_vrr_policy()
{
    return std::static_pointer_cast<events_t>(get_events())->vrr_policy;
}

std::function<void(kde_output_device_v2_rgb_range)> &kde_output_device_v2_t::on_rgb_range()
{
    return std::static_pointer_cast<events_t>(get_events())->rgb_range;
}

std::function<void(std::string)> &kde_output_device_v2_t::on_name()
{
    return std::static_pointer_cast<events_t>(get_events())->name;
}

std::function<void(uint32_t)> &kde_output_device_v2_t::on_high_dynamic_range()
{
    return std::static_pointer_cast<events_t>(get_events())->high_dynamic_range;
}

std::function<void(uint32_t)> &kde_output_device_v2_t::on_sdr_brightness()
{
    return std::static_pointer_cast<events_t>(get_events())->sdr_brightness;
}

std::function<void(uint32_t)> &kde_output_device_v2_t::on_wide_color_gamut()
{
    return std::static_pointer_cast<events_t>(get_events())->wide_color_gamut;
}

std::function<void(kde_output_device_v2_auto_rotate_policy)> &kde_output_device_v2_t::on_auto_rotate_policy()
{
    return std::static_pointer_cast<events_t>(get_events())->auto_rotate_policy;
}

std::function<void(std::string)> &kde_output_device_v2_t::on_icc_profile_path()
{
    return std::static_pointer_cast<events_t>(get_events())->icc_profile_path;
}

std::function<void(uint32_t, uint32_t, uint32_t)> &kde_output_device_v2_t::on_brightness_metadata()
{
    return std::static_pointer_cast<events_t>(get_events())->brightness_metadata;
}

std::function<void(int32_t, int32_t, int32_t)> &kde_output_device_v2_t::on_brightness_overrides()
{
    return std::static_pointer_cast<events_t>(get_events())->brightness_overrides;
}

std::function<void(uint32_t)> &kde_output_device_v2_t::on_sdr_gamut_wideness()
{
    return std::static_pointer_cast<events_t>(get_events())->sdr_gamut_wideness;
}

std::function<void(kde_output_device_v2_color_profile_source)> &kde_output_device_v2_t::on_color_profile_source()
{
    return std::static_pointer_cast<events_t>(get_events())->color_profile_source;
}

std::function<void(uint32_t)> &kde_output_device_v2_t::on_brightness()
{
    return std::static_pointer_cast<events_t>(get_events())->brightness;
}

int kde_output_device_v2_t::dispatcher(uint32_t opcode, const std::vector<any> &args,
                                       const std::shared_ptr<detail::events_base_t> &e)
{
    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch (opcode)
    {
    case 0:
        if (events->geometry)
            events->geometry(args[0].get<int32_t>(), args[1].get<int32_t>(), args[2].get<int32_t>(), args[3].get<int32_t>(),
                             args[4].get<int32_t>(), args[5].get<std::string>(), args[6].get<std::string>(),
                             args[7].get<int32_t>());
        break;
    case 1:
        if (events->current_mode)
            events->current_mode(kde_output_device_mode_v2_t(args[0].get<proxy_t>()));
        break;
    case 2:
        if (events->mode)
            events->mode(kde_output_device_mode_v2_t(args[0].get<proxy_t>()));
        break;
    case 3:
        if (events->done)
            events->done();
        break;
    case 4:
        if (events->scale)
            events->scale(args[0].get<double>());
        break;
    case 5:
        if (events->edid)
            events->edid(args[0].get<std::string>());
        break;
    case 6:
        if (events->enabled)
            events->enabled(args[0].get<int32_t>());
        break;
    case 7:
        if (events->uuid)
            events->uuid(args[0].get<std::string>());
        break;
    case 8:
        if (events->serial_number)
            events->serial_number(args[0].get<std::string>());
        break;
    case 9:
        if (events->eisa_id)
            events->eisa_id(args[0].get<std::string>());
        break;
    case 10:
        if (events->capabilities)
            events->capabilities(kde_output_device_v2_capability(args[0].get<uint32_t>()));
        break;
    case 11:
        if (events->overscan)
            events->overscan(args[0].get<uint32_t>());
        break;
    case 12:
        if (events->vrr_policy)
            events->vrr_policy(kde_output_device_v2_vrr_policy(args[0].get<uint32_t>()));
        break;
    case 13:
        if (events->rgb_range)
            events->rgb_range(kde_output_device_v2_rgb_range(args[0].get<uint32_t>()));
        break;
    case 14:
        if (events->name)
            events->name(args[0].get<std::string>());
        break;
    case 15:
        if (events->high_dynamic_range)
            events->high_dynamic_range(args[0].get<uint32_t>());
        break;
    case 16:
        if (events->sdr_brightness)
            events->sdr_brightness(args[0].get<uint32_t>());
        break;
    case 17:
        if (events->wide_color_gamut)
            events->wide_color_gamut(args[0].get<uint32_t>());
        break;
    case 18:
        if (events->auto_rotate_policy)
            events->auto_rotate_policy(kde_output_device_v2_auto_rotate_policy(args[0].get<uint32_t>()));
        break;
    case 19:
        if (events->icc_profile_path)
            events->icc_profile_path(args[0].get<std::string>());
        break;
    case 20:
        if (events->brightness_metadata)
            events->brightness_metadata(args[0].get<uint32_t>(), args[1].get<uint32_t>(), args[2].get<uint32_t>());
        break;
    case 21:
        if (events->brightness_overrides)
            events->brightness_overrides(args[0].get<int32_t>(), args[1].get<int32_t>(), args[2].get<int32_t>());
        break;
    case 22:
        if (events->sdr_gamut_wideness)
            events->sdr_gamut_wideness(args[0].get<uint32_t>());
        break;
    case 23:
        if (events->color_profile_source)
            events->color_profile_source(kde_output_device_v2_color_profile_source(args[0].get<uint32_t>()));
        break;
    case 24:
        if (events->brightness)
            events->brightness(args[0].get<uint32_t>());
        break;
    }
    return 0;
}

const bitfield<7, 2> kde_output_device_v2_capability::overscan{0x1};
const bitfield<7, 2> kde_output_device_v2_capability::vrr{0x2};
const bitfield<7, 2> kde_output_device_v2_capability::rgb_range{0x4};
const bitfield<7, 2> kde_output_device_v2_capability::high_dynamic_range{0x8};
const bitfield<7, 2> kde_output_device_v2_capability::wide_color_gamut{0x10};
const bitfield<7, 2> kde_output_device_v2_capability::auto_rotate{0x20};
const bitfield<7, 2> kde_output_device_v2_capability::icc_profile{0x40};

kde_output_device_mode_v2_t::kde_output_device_mode_v2_t(const proxy_t &p) : proxy_t(p)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
    }
    set_interface(&kde_output_device_mode_v2_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return kde_output_device_mode_v2_t(p); });
}

kde_output_device_mode_v2_t::kde_output_device_mode_v2_t()
{
    set_interface(&kde_output_device_mode_v2_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return kde_output_device_mode_v2_t(p); });
}

kde_output_device_mode_v2_t::kde_output_device_mode_v2_t(kde_output_device_mode_v2 *p, wrapper_type t)
    : proxy_t(reinterpret_cast<wl_proxy *>(p), t)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
    }
    set_interface(&kde_output_device_mode_v2_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return kde_output_device_mode_v2_t(p); });
}

kde_output_device_mode_v2_t::kde_output_device_mode_v2_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/)
    : proxy_t(wrapped_proxy, construct_proxy_wrapper_tag())
{
    set_interface(&kde_output_device_mode_v2_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return kde_output_device_mode_v2_t(p); });
}

kde_output_device_mode_v2_t kde_output_device_mode_v2_t::proxy_create_wrapper()
{
    return {*this, construct_proxy_wrapper_tag()};
}

const std::string kde_output_device_mode_v2_t::interface_name = "kde_output_device_mode_v2";

kde_output_device_mode_v2_t::operator kde_output_device_mode_v2 *() const
{
    return reinterpret_cast<kde_output_device_mode_v2 *>(c_ptr());
}

std::function<void(int32_t, int32_t)> &kde_output_device_mode_v2_t::on_size()
{
    return std::static_pointer_cast<events_t>(get_events())->size;
}

std::function<void(int32_t)> &kde_output_device_mode_v2_t::on_refresh()
{
    return std::static_pointer_cast<events_t>(get_events())->refresh;
}

std::function<void()> &kde_output_device_mode_v2_t::on_preferred()
{
    return std::static_pointer_cast<events_t>(get_events())->preferred;
}

std::function<void()> &kde_output_device_mode_v2_t::on_removed()
{
    return std::static_pointer_cast<events_t>(get_events())->removed;
}

int kde_output_device_mode_v2_t::dispatcher(uint32_t opcode, const std::vector<any> &args,
                                            const std::shared_ptr<detail::events_base_t> &e)
{
    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch (opcode)
    {
    case 0:
        if (events->size)
            events->size(args[0].get<int32_t>(), args[1].get<int32_t>());
        break;
    case 1:
        if (events->refresh)
            events->refresh(args[0].get<int32_t>());
        break;
    case 2:
        if (events->preferred)
            events->preferred();
        break;
    case 3:
        if (events->removed)
            events->removed();
        break;
    }
    return 0;
}
