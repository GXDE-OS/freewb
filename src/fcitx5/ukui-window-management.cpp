#include <ukui-window-management.h>

using namespace wayland;
using namespace wayland::detail;

const wl_interface *ukui_window_management_interface_show_desktop_request[1] = {
    nullptr,
};

const wl_interface *ukui_window_management_interface_create_window_request[2] = {
    &ukui_window_interface,
    nullptr,
};

const wl_interface *ukui_window_management_interface_show_desktop_changed_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_management_interface_stacking_order_changed_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_management_interface_window_created_event[1] = {
    nullptr,
};

const wl_message ukui_window_management_interface_requests[2] = {
    {
        "show_desktop",
        "u",
        ukui_window_management_interface_show_desktop_request,
    },
    {
        "create_window",
        "ns",
        ukui_window_management_interface_create_window_request,
    },
};

const wl_message ukui_window_management_interface_events[3] = {
    {
        "show_desktop_changed",
        "u",
        ukui_window_management_interface_show_desktop_changed_event,
    },
    {
        "stacking_order_changed",
        "s",
        ukui_window_management_interface_stacking_order_changed_event,
    },
    {
        "window_created",
        "s",
        ukui_window_management_interface_window_created_event,
    },
};

const wl_interface wayland::detail::ukui_window_management_interface = {
    "ukui_window_management", 1, 2, ukui_window_management_interface_requests, 3, ukui_window_management_interface_events,
};

const wl_interface *ukui_window_interface_set_state_request[2] = {
    nullptr,
    nullptr,
};

const wl_interface *ukui_window_interface_set_startup_geometry_request[5] = {
    &surface_interface, nullptr, nullptr, nullptr, nullptr,
};

const wl_interface *ukui_window_interface_set_minimized_geometry_request[5] = {
    &surface_interface, nullptr, nullptr, nullptr, nullptr,
};

const wl_interface *ukui_window_interface_unset_minimized_geometry_request[1] = {
    &surface_interface,
};

const wl_interface *ukui_window_interface_close_request[0] = {};

const wl_interface *ukui_window_interface_request_move_request[0] = {};

const wl_interface *ukui_window_interface_request_resize_request[0] = {};

const wl_interface *ukui_window_interface_destroy_request[0] = {};

const wl_interface *ukui_window_interface_get_icon_request[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_request_enter_virtual_desktop_request[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_request_enter_new_virtual_desktop_request[0] = {};

const wl_interface *ukui_window_interface_request_leave_virtual_desktop_request[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_request_enter_activity_request[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_request_leave_activity_request[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_send_to_output_request[1] = {
    &output_interface,
};

const wl_interface *ukui_window_interface_highlight_request[0] = {};

const wl_interface *ukui_window_interface_unset_highlight_request[0] = {};

const wl_interface *ukui_window_interface_title_changed_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_app_id_changed_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_state_changed_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_themed_icon_name_changed_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_unmapped_event[0] = {};

const wl_interface *ukui_window_interface_initial_state_event[0] = {};

const wl_interface *ukui_window_interface_parent_window_event[1] = {
    &ukui_window_interface,
};

const wl_interface *ukui_window_interface_geometry_event[4] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

const wl_interface *ukui_window_interface_icon_changed_event[0] = {};

const wl_interface *ukui_window_interface_pid_changed_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_virtual_desktop_entered_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_virtual_desktop_left_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_application_menu_event[2] = {
    nullptr,
    nullptr,
};

const wl_interface *ukui_window_interface_activity_entered_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_activity_left_event[1] = {
    nullptr,
};

const wl_interface *ukui_window_interface_resource_name_changed_event[1] = {
    nullptr,
};

const wl_message ukui_window_interface_requests[17] = {
    {
        "set_state",
        "uu",
        ukui_window_interface_set_state_request,
    },
    {
        "set_startup_geometry",
        "ouuuu",
        ukui_window_interface_set_startup_geometry_request,
    },
    {
        "set_minimized_geometry",
        "ouuuu",
        ukui_window_interface_set_minimized_geometry_request,
    },
    {
        "unset_minimized_geometry",
        "o",
        ukui_window_interface_unset_minimized_geometry_request,
    },
    {
        "close",
        "",
        ukui_window_interface_close_request,
    },
    {
        "request_move",
        "",
        ukui_window_interface_request_move_request,
    },
    {
        "request_resize",
        "",
        ukui_window_interface_request_resize_request,
    },
    {
        "destroy",
        "",
        ukui_window_interface_destroy_request,
    },
    {
        "get_icon",
        "h",
        ukui_window_interface_get_icon_request,
    },
    {
        "request_enter_virtual_desktop",
        "s",
        ukui_window_interface_request_enter_virtual_desktop_request,
    },
    {
        "request_enter_new_virtual_desktop",
        "",
        ukui_window_interface_request_enter_new_virtual_desktop_request,
    },
    {
        "request_leave_virtual_desktop",
        "s",
        ukui_window_interface_request_leave_virtual_desktop_request,
    },
    {
        "request_enter_activity",
        "s",
        ukui_window_interface_request_enter_activity_request,
    },
    {
        "request_leave_activity",
        "s",
        ukui_window_interface_request_leave_activity_request,
    },
    {
        "send_to_output",
        "o",
        ukui_window_interface_send_to_output_request,
    },
    {
        "highlight",
        "",
        ukui_window_interface_highlight_request,
    },
    {
        "unset_highlight",
        "",
        ukui_window_interface_unset_highlight_request,
    },
};

const wl_message ukui_window_interface_events[16] = {
    {
        "title_changed",
        "s",
        ukui_window_interface_title_changed_event,
    },
    {
        "app_id_changed",
        "s",
        ukui_window_interface_app_id_changed_event,
    },
    {
        "state_changed",
        "u",
        ukui_window_interface_state_changed_event,
    },
    {
        "themed_icon_name_changed",
        "s",
        ukui_window_interface_themed_icon_name_changed_event,
    },
    {
        "unmapped",
        "",
        ukui_window_interface_unmapped_event,
    },
    {
        "initial_state",
        "",
        ukui_window_interface_initial_state_event,
    },
    {
        "parent_window",
        "?o",
        ukui_window_interface_parent_window_event,
    },
    {
        "geometry",
        "iiuu",
        ukui_window_interface_geometry_event,
    },
    {
        "icon_changed",
        "",
        ukui_window_interface_icon_changed_event,
    },
    {
        "pid_changed",
        "u",
        ukui_window_interface_pid_changed_event,
    },
    {
        "virtual_desktop_entered",
        "s",
        ukui_window_interface_virtual_desktop_entered_event,
    },
    {
        "virtual_desktop_left",
        "s",
        ukui_window_interface_virtual_desktop_left_event,
    },
    {
        "application_menu",
        "ss",
        ukui_window_interface_application_menu_event,
    },
    {
        "activity_entered",
        "s",
        ukui_window_interface_activity_entered_event,
    },
    {
        "activity_left",
        "s",
        ukui_window_interface_activity_left_event,
    },
    {
        "resource_name_changed",
        "s",
        ukui_window_interface_resource_name_changed_event,
    },
};

const wl_interface wayland::detail::ukui_window_interface = {
    "ukui_window", 1, 17, ukui_window_interface_requests, 16, ukui_window_interface_events,
};

const wl_interface *org_ukui_activation_feedback_interface_destroy_request[0] = {};

const wl_interface *org_ukui_activation_feedback_interface_activation_event[1] = {
    &org_ukui_activation_interface,
};

const wl_message org_ukui_activation_feedback_interface_requests[1] = {
    {
        "destroy",
        "",
        org_ukui_activation_feedback_interface_destroy_request,
    },
};

const wl_message org_ukui_activation_feedback_interface_events[1] = {
    {
        "activation",
        "n",
        org_ukui_activation_feedback_interface_activation_event,
    },
};

const wl_interface wayland::detail::org_ukui_activation_feedback_interface = {
    "org_ukui_activation_feedback",
    1,
    1,
    org_ukui_activation_feedback_interface_requests,
    1,
    org_ukui_activation_feedback_interface_events,
};

const wl_interface *org_ukui_activation_interface_destroy_request[0] = {};

const wl_interface *org_ukui_activation_interface_app_id_event[1] = {
    nullptr,
};

const wl_interface *org_ukui_activation_interface_finished_event[0] = {};

const wl_message org_ukui_activation_interface_requests[1] = {
    {
        "destroy",
        "",
        org_ukui_activation_interface_destroy_request,
    },
};

const wl_message org_ukui_activation_interface_events[2] = {
    {
        "app_id",
        "s",
        org_ukui_activation_interface_app_id_event,
    },
    {
        "finished",
        "",
        org_ukui_activation_interface_finished_event,
    },
};

const wl_interface wayland::detail::org_ukui_activation_interface = {
    "org_ukui_activation", 1, 1, org_ukui_activation_interface_requests, 2, org_ukui_activation_interface_events,
};

ukui_window_management_t::ukui_window_management_t(const proxy_t &p) : proxy_t(p)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
    }
    set_interface(&ukui_window_management_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return ukui_window_management_t(p); });
}

ukui_window_management_t::ukui_window_management_t()
{
    set_interface(&ukui_window_management_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return ukui_window_management_t(p); });
}

ukui_window_management_t::ukui_window_management_t(ukui_window_management *p, wrapper_type t)
    : proxy_t(reinterpret_cast<wl_proxy *>(p), t)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
    }
    set_interface(&ukui_window_management_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return ukui_window_management_t(p); });
}

ukui_window_management_t::ukui_window_management_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/)
    : proxy_t(wrapped_proxy, construct_proxy_wrapper_tag())
{
    set_interface(&ukui_window_management_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return ukui_window_management_t(p); });
}

ukui_window_management_t ukui_window_management_t::proxy_create_wrapper()
{
    return {*this, construct_proxy_wrapper_tag()};
}

const std::string ukui_window_management_t::interface_name = "ukui_window_management";

ukui_window_management_t::operator ukui_window_management *() const
{
    return reinterpret_cast<ukui_window_management *>(c_ptr());
}

void ukui_window_management_t::show_desktop(uint32_t state)
{
    marshal(0U, state);
}

ukui_window_t ukui_window_management_t::create_window(std::string const &internal_window_uuid)
{
    proxy_t p = marshal_constructor(1U, &ukui_window_interface, nullptr, internal_window_uuid);
    return ukui_window_t(p);
}

std::function<void(uint32_t)> &ukui_window_management_t::on_show_desktop_changed()
{
    return std::static_pointer_cast<events_t>(get_events())->show_desktop_changed;
}

std::function<void(std::string)> &ukui_window_management_t::on_stacking_order_changed()
{
    return std::static_pointer_cast<events_t>(get_events())->stacking_order_changed;
}

std::function<void(std::string)> &ukui_window_management_t::on_window_created()
{
    return std::static_pointer_cast<events_t>(get_events())->window_created;
}

int ukui_window_management_t::dispatcher(uint32_t opcode, const std::vector<any> &args,
                                         const std::shared_ptr<detail::events_base_t> &e)
{
    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch (opcode)
    {
    case 0:
        if (events->show_desktop_changed)
            events->show_desktop_changed(args[0].get<uint32_t>());
        break;
    case 1:
        if (events->stacking_order_changed)
            events->stacking_order_changed(args[0].get<std::string>());
        break;
    case 2:
        if (events->window_created)
            events->window_created(args[0].get<std::string>());
        break;
    }
    return 0;
}

ukui_window_t::ukui_window_t(const proxy_t &p) : proxy_t(p)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
        set_destroy_opcode(7U);
    }
    set_interface(&ukui_window_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return ukui_window_t(p); });
}

ukui_window_t::ukui_window_t()
{
    set_interface(&ukui_window_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return ukui_window_t(p); });
}

ukui_window_t::ukui_window_t(ukui_window *p, wrapper_type t) : proxy_t(reinterpret_cast<wl_proxy *>(p), t)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
        set_destroy_opcode(7U);
    }
    set_interface(&ukui_window_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return ukui_window_t(p); });
}

ukui_window_t::ukui_window_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/)
    : proxy_t(wrapped_proxy, construct_proxy_wrapper_tag())
{
    set_interface(&ukui_window_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return ukui_window_t(p); });
}

ukui_window_t ukui_window_t::proxy_create_wrapper()
{
    return {*this, construct_proxy_wrapper_tag()};
}

const std::string ukui_window_t::interface_name = "ukui_window";

ukui_window_t::operator ukui_window *() const
{
    return reinterpret_cast<ukui_window *>(c_ptr());
}

void ukui_window_t::set_state(uint32_t flags, uint32_t state)
{
    marshal(0U, flags, state);
}

void ukui_window_t::set_startup_geometry(surface_t const &entry, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    marshal(1U, entry.proxy_has_object() ? reinterpret_cast<wl_object *>(entry.c_ptr()) : nullptr, x, y, width, height);
}

void ukui_window_t::set_minimized_geometry(surface_t const &panel, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    marshal(2U, panel.proxy_has_object() ? reinterpret_cast<wl_object *>(panel.c_ptr()) : nullptr, x, y, width, height);
}

void ukui_window_t::unset_minimized_geometry(surface_t const &panel)
{
    marshal(3U, panel.proxy_has_object() ? reinterpret_cast<wl_object *>(panel.c_ptr()) : nullptr);
}

void ukui_window_t::close()
{
    marshal(4U);
}

void ukui_window_t::request_move()
{
    marshal(5U);
}

void ukui_window_t::request_resize()
{
    marshal(6U);
}

void ukui_window_t::get_icon(int fd)
{
    marshal(8U, argument_t::fd(fd));
}

void ukui_window_t::request_enter_virtual_desktop(std::string const &id)
{
    marshal(9U, id);
}

void ukui_window_t::request_enter_new_virtual_desktop()
{
    marshal(10U);
}

void ukui_window_t::request_leave_virtual_desktop(std::string const &id)
{
    marshal(11U, id);
}

void ukui_window_t::request_enter_activity(std::string const &id)
{
    marshal(12U, id);
}

void ukui_window_t::request_leave_activity(std::string const &id)
{
    marshal(13U, id);
}

void ukui_window_t::send_to_output(output_t const &output)
{
    marshal(14U, output.proxy_has_object() ? reinterpret_cast<wl_object *>(output.c_ptr()) : nullptr);
}

void ukui_window_t::highlight()
{
    marshal(15U);
}

void ukui_window_t::unset_highlight()
{
    marshal(16U);
}

std::function<void(std::string)> &ukui_window_t::on_title_changed()
{
    return std::static_pointer_cast<events_t>(get_events())->title_changed;
}

std::function<void(std::string)> &ukui_window_t::on_app_id_changed()
{
    return std::static_pointer_cast<events_t>(get_events())->app_id_changed;
}

std::function<void(uint32_t)> &ukui_window_t::on_state_changed()
{
    return std::static_pointer_cast<events_t>(get_events())->state_changed;
}

std::function<void(std::string)> &ukui_window_t::on_themed_icon_name_changed()
{
    return std::static_pointer_cast<events_t>(get_events())->themed_icon_name_changed;
}

std::function<void()> &ukui_window_t::on_unmapped()
{
    return std::static_pointer_cast<events_t>(get_events())->unmapped;
}

std::function<void()> &ukui_window_t::on_initial_state()
{
    return std::static_pointer_cast<events_t>(get_events())->initial_state;
}

std::function<void(ukui_window_t)> &ukui_window_t::on_parent_window()
{
    return std::static_pointer_cast<events_t>(get_events())->parent_window;
}

std::function<void(int32_t, int32_t, uint32_t, uint32_t)> &ukui_window_t::on_geometry()
{
    return std::static_pointer_cast<events_t>(get_events())->geometry;
}

std::function<void()> &ukui_window_t::on_icon_changed()
{
    return std::static_pointer_cast<events_t>(get_events())->icon_changed;
}

std::function<void(uint32_t)> &ukui_window_t::on_pid_changed()
{
    return std::static_pointer_cast<events_t>(get_events())->pid_changed;
}

std::function<void(std::string)> &ukui_window_t::on_virtual_desktop_entered()
{
    return std::static_pointer_cast<events_t>(get_events())->virtual_desktop_entered;
}

std::function<void(std::string)> &ukui_window_t::on_virtual_desktop_left()
{
    return std::static_pointer_cast<events_t>(get_events())->virtual_desktop_left;
}

std::function<void(std::string, std::string)> &ukui_window_t::on_application_menu()
{
    return std::static_pointer_cast<events_t>(get_events())->application_menu;
}

std::function<void(std::string)> &ukui_window_t::on_activity_entered()
{
    return std::static_pointer_cast<events_t>(get_events())->activity_entered;
}

std::function<void(std::string)> &ukui_window_t::on_activity_left()
{
    return std::static_pointer_cast<events_t>(get_events())->activity_left;
}

std::function<void(std::string)> &ukui_window_t::on_resource_name_changed()
{
    return std::static_pointer_cast<events_t>(get_events())->resource_name_changed;
}

int ukui_window_t::dispatcher(uint32_t opcode, const std::vector<any> &args, const std::shared_ptr<detail::events_base_t> &e)
{
    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch (opcode)
    {
    case 0:
        if (events->title_changed)
            events->title_changed(args[0].get<std::string>());
        break;
    case 1:
        if (events->app_id_changed)
            events->app_id_changed(args[0].get<std::string>());
        break;
    case 2:
        if (events->state_changed)
            events->state_changed(args[0].get<uint32_t>());
        break;
    case 3:
        if (events->themed_icon_name_changed)
            events->themed_icon_name_changed(args[0].get<std::string>());
        break;
    case 4:
        if (events->unmapped)
            events->unmapped();
        break;
    case 5:
        if (events->initial_state)
            events->initial_state();
        break;
    case 6:
        if (events->parent_window)
            events->parent_window(ukui_window_t(args[0].get<proxy_t>()));
        break;
    case 7:
        if (events->geometry)
            events->geometry(args[0].get<int32_t>(), args[1].get<int32_t>(), args[2].get<uint32_t>(), args[3].get<uint32_t>());
        break;
    case 8:
        if (events->icon_changed)
            events->icon_changed();
        break;
    case 9:
        if (events->pid_changed)
            events->pid_changed(args[0].get<uint32_t>());
        break;
    case 10:
        if (events->virtual_desktop_entered)
            events->virtual_desktop_entered(args[0].get<std::string>());
        break;
    case 11:
        if (events->virtual_desktop_left)
            events->virtual_desktop_left(args[0].get<std::string>());
        break;
    case 12:
        if (events->application_menu)
            events->application_menu(args[0].get<std::string>(), args[1].get<std::string>());
        break;
    case 13:
        if (events->activity_entered)
            events->activity_entered(args[0].get<std::string>());
        break;
    case 14:
        if (events->activity_left)
            events->activity_left(args[0].get<std::string>());
        break;
    case 15:
        if (events->resource_name_changed)
            events->resource_name_changed(args[0].get<std::string>());
        break;
    }
    return 0;
}

org_ukui_activation_feedback_t::org_ukui_activation_feedback_t(const proxy_t &p) : proxy_t(p)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
        set_destroy_opcode(0U);
    }
    set_interface(&org_ukui_activation_feedback_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return org_ukui_activation_feedback_t(p); });
}

org_ukui_activation_feedback_t::org_ukui_activation_feedback_t()
{
    set_interface(&org_ukui_activation_feedback_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return org_ukui_activation_feedback_t(p); });
}

org_ukui_activation_feedback_t::org_ukui_activation_feedback_t(org_ukui_activation_feedback *p, wrapper_type t)
    : proxy_t(reinterpret_cast<wl_proxy *>(p), t)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
        set_destroy_opcode(0U);
    }
    set_interface(&org_ukui_activation_feedback_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return org_ukui_activation_feedback_t(p); });
}

org_ukui_activation_feedback_t::org_ukui_activation_feedback_t(proxy_t const &wrapped_proxy,
                                                               construct_proxy_wrapper_tag /*unused*/)
    : proxy_t(wrapped_proxy, construct_proxy_wrapper_tag())
{
    set_interface(&org_ukui_activation_feedback_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return org_ukui_activation_feedback_t(p); });
}

org_ukui_activation_feedback_t org_ukui_activation_feedback_t::proxy_create_wrapper()
{
    return {*this, construct_proxy_wrapper_tag()};
}

const std::string org_ukui_activation_feedback_t::interface_name = "org_ukui_activation_feedback";

org_ukui_activation_feedback_t::operator org_ukui_activation_feedback *() const
{
    return reinterpret_cast<org_ukui_activation_feedback *>(c_ptr());
}

std::function<void(org_ukui_activation_t)> &org_ukui_activation_feedback_t::on_activation()
{
    return std::static_pointer_cast<events_t>(get_events())->activation;
}

int org_ukui_activation_feedback_t::dispatcher(uint32_t opcode, const std::vector<any> &args,
                                               const std::shared_ptr<detail::events_base_t> &e)
{
    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch (opcode)
    {
    case 0:
        if (events->activation)
            events->activation(org_ukui_activation_t(args[0].get<proxy_t>()));
        break;
    }
    return 0;
}

org_ukui_activation_t::org_ukui_activation_t(const proxy_t &p) : proxy_t(p)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
        set_destroy_opcode(0U);
    }
    set_interface(&org_ukui_activation_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return org_ukui_activation_t(p); });
}

org_ukui_activation_t::org_ukui_activation_t()
{
    set_interface(&org_ukui_activation_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return org_ukui_activation_t(p); });
}

org_ukui_activation_t::org_ukui_activation_t(org_ukui_activation *p, wrapper_type t) : proxy_t(reinterpret_cast<wl_proxy *>(p), t)
{
    if (proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
        set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
        set_destroy_opcode(0U);
    }
    set_interface(&org_ukui_activation_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return org_ukui_activation_t(p); });
}

org_ukui_activation_t::org_ukui_activation_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/)
    : proxy_t(wrapped_proxy, construct_proxy_wrapper_tag())
{
    set_interface(&org_ukui_activation_interface);
    set_copy_constructor([](const proxy_t &p) -> proxy_t { return org_ukui_activation_t(p); });
}

org_ukui_activation_t org_ukui_activation_t::proxy_create_wrapper()
{
    return {*this, construct_proxy_wrapper_tag()};
}

const std::string org_ukui_activation_t::interface_name = "org_ukui_activation";

org_ukui_activation_t::operator org_ukui_activation *() const
{
    return reinterpret_cast<org_ukui_activation *>(c_ptr());
}

std::function<void(std::string)> &org_ukui_activation_t::on_app_id()
{
    return std::static_pointer_cast<events_t>(get_events())->app_id;
}

std::function<void()> &org_ukui_activation_t::on_finished()
{
    return std::static_pointer_cast<events_t>(get_events())->finished;
}

int org_ukui_activation_t::dispatcher(uint32_t opcode, const std::vector<any> &args,
                                      const std::shared_ptr<detail::events_base_t> &e)
{
    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch (opcode)
    {
    case 0:
        if (events->app_id)
            events->app_id(args[0].get<std::string>());
        break;
    case 1:
        if (events->finished)
            events->finished();
        break;
    }
    return 0;
}
