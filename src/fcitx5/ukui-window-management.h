#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <wayland-client.hpp>

struct ukui_window_management;
struct ukui_window;
struct org_ukui_activation_feedback;
struct org_ukui_activation;

namespace wayland
{
class ukui_window_management_t;
enum class ukui_window_management_show_desktop : uint32_t;
class ukui_window_t;
enum class ukui_window_state : uint32_t;
class org_ukui_activation_feedback_t;
class org_ukui_activation_t;

namespace detail
{
extern const wl_interface ukui_window_management_interface;
extern const wl_interface ukui_window_interface;
extern const wl_interface org_ukui_activation_feedback_interface;
extern const wl_interface org_ukui_activation_interface;
} // namespace detail

/** \brief application windows management

      This interface manages application windows. It provides requests to show and hide the desktop
      and emits an event every time a window is created so that the client can use it to manage the
      window.

*/
class ukui_window_management_t : public proxy_t
{
private:
    struct events_t : public detail::events_base_t
    {
        std::function<void(uint32_t)> show_desktop_changed;
        std::function<void(std::string)> stacking_order_changed;
        std::function<void(std::string)> window_created;
    };

    static int dispatcher(uint32_t opcode, const std::vector<detail::any> &args, const std::shared_ptr<detail::events_base_t> &e);

    ukui_window_management_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/);

public:
    ukui_window_management_t();
    explicit ukui_window_management_t(const proxy_t &proxy);
    ukui_window_management_t(ukui_window_management *p, wrapper_type t = wrapper_type::standard);

    ukui_window_management_t proxy_create_wrapper();

    static const std::string interface_name;

    operator ukui_window_management *() const;

    /** \brief show/hide the desktop
        \param state requested state

          Tell the compositor to show/hide the desktop.

    */
    void show_desktop(uint32_t state);

    /** \brief Minimum protocol version required for the \ref show_desktop function
     */
    static constexpr std::uint32_t show_desktop_since_version = 1;

    /** \brief
        \param internal_window_uuid The internal window uuid of the window to create

    */
    ukui_window_t create_window(std::string const &internal_window_uuid);

    /** \brief Minimum protocol version required for the \ref create_window function
     */
    static constexpr std::uint32_t create_window_since_version = 1;

    /** \brief notify the client when the show desktop mode is entered/left
        \param state new state

          This event will be sent whenever the show desktop mode changes. E.g. when it is entered or
          left.

          On binding the interface the current state is sent.

    */
    std::function<void(uint32_t)> &on_show_desktop_changed();

    /** \brief notify the client when stacking order changed
        \param uuids internal windows uuid, use ';' separated

          This event will be sent when stacking order changed and on bind

    */
    std::function<void(std::string)> &on_stacking_order_changed();

    /** \brief notify the client that a window was mapped
        \param uuid internal window uuid

          This event will be sent immediately after a window is mapped.

    */
    std::function<void(std::string)> &on_window_created();
};

/** \brief

  */
enum class ukui_window_management_show_desktop : uint32_t
{
    disabled = 0,
    enabled = 1
};

/** \brief interface to control application windows

      Manages and control an application window.

*/
class ukui_window_t : public proxy_t
{
private:
    struct events_t : public detail::events_base_t
    {
        std::function<void(std::string)> title_changed;
        std::function<void(std::string)> app_id_changed;
        std::function<void(uint32_t)> state_changed;
        std::function<void(std::string)> themed_icon_name_changed;
        std::function<void()> unmapped;
        std::function<void()> initial_state;
        std::function<void(ukui_window_t)> parent_window;
        std::function<void(int32_t, int32_t, uint32_t, uint32_t)> geometry;
        std::function<void()> icon_changed;
        std::function<void(uint32_t)> pid_changed;
        std::function<void(std::string)> virtual_desktop_entered;
        std::function<void(std::string)> virtual_desktop_left;
        std::function<void(std::string, std::string)> application_menu;
        std::function<void(std::string)> activity_entered;
        std::function<void(std::string)> activity_left;
        std::function<void(std::string)> resource_name_changed;
    };

    static int dispatcher(uint32_t opcode, const std::vector<detail::any> &args, const std::shared_ptr<detail::events_base_t> &e);

    ukui_window_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/);

public:
    ukui_window_t();
    explicit ukui_window_t(const proxy_t &proxy);
    ukui_window_t(ukui_window *p, wrapper_type t = wrapper_type::standard);

    ukui_window_t proxy_create_wrapper();

    static const std::string interface_name;

    operator ukui_window *() const;

    /** \brief set window state
        \param flags bitfield of set state flags
        \param state bitfield of state flags

          Set window state. Can set multiple states at once.

          Values for state argument are described by ukui_window_management.state and can be used
          together in a bitfield.
          The flags bitfield describes which flags are supposed to be set, the
          state bitfield the value for the set flags.

          The state argument is not a boolean value, but a bitfield, so it is possible to set multiple
          states at once. Only the states that are set in the flags bitfield will be changed.

          For example:

          If flags was 0x14(keep_above|maximized) and state was 0x04(maximized), the window would be
          maximized, but not keep above.

          If flags was 0x04(maximized) and state was 0x14(keep_above|maximized), the window would be
          maximized but not change keep above state.

    */
    void set_state(uint32_t flags, uint32_t state);

    /** \brief Minimum protocol version required for the \ref set_state function
     */
    static constexpr std::uint32_t set_state_since_version = 1;

    /** \brief set the geometry for a taskbar/desktop entry
        \param entry the taskbar/desktop entry for the window
        \param x
        \param y
        \param width
        \param height

          Sets the geometry of the taskbar/desktop entry for this window. The geometry is relative to
          a panel/desktop in particular.

    */
    void set_startup_geometry(surface_t const &entry, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    /** \brief Minimum protocol version required for the \ref set_startup_geometry function
     */
    static constexpr std::uint32_t set_startup_geometry_since_version = 1;

    /** \brief set the geometry for a taskbar entry
        \param panel
        \param x
        \param y
        \param width
        \param height

          Sets the geometry of the taskbar entry for this window. The geometry is relative to a panel
          in particular.

    */
    void set_minimized_geometry(surface_t const &panel, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    /** \brief Minimum protocol version required for the \ref set_minimized_geometry function
     */
    static constexpr std::uint32_t set_minimized_geometry_since_version = 1;

    /** \brief set the geometry for a taskbar entry
        \param panel

          Remove the task geometry information for a particular panel.

    */
    void unset_minimized_geometry(surface_t const &panel);

    /** \brief Minimum protocol version required for the \ref unset_minimized_geometry function
     */
    static constexpr std::uint32_t unset_minimized_geometry_since_version = 1;

    /** \brief close window

          Close this window.

    */
    void close();

    /** \brief Minimum protocol version required for the \ref close function
     */
    static constexpr std::uint32_t close_since_version = 1;

    /** \brief request move

          Request an interactive move for this window. The pointer will move to the center of the
          window and move with the pointer.

          When mouse button is released, the interactive move ends.

    */
    void request_move();

    /** \brief Minimum protocol version required for the \ref request_move function
     */
    static constexpr std::uint32_t request_move_since_version = 1;

    /** \brief request resize

          Request an interactive resize for this window. The pointer will move to window right bottom
          corner and resize with the pointer.

          When mouse button is released, the interactive resize ends.

    */
    void request_resize();

    /** \brief Minimum protocol version required for the \ref request_resize function
     */
    static constexpr std::uint32_t request_resize_since_version = 1;

    /** \brief Requests to get the window icon
        \param fd file descriptor for the icon

          The compositor will write the window icon into the provided file descriptor. The data is a
          serialized QIcon with QDataStream.

    */
    void get_icon(int fd);

    /** \brief Minimum protocol version required for the \ref get_icon function
     */
    static constexpr std::uint32_t get_icon_since_version = 1;

    /** \brief map window on a virtual desktop
        \param id desktop id

          Make the window enter a virtual desktop. A window can enter more than one virtual desktop.
          if the id is empty or invalid, no action will be performed.

    */
    void request_enter_virtual_desktop(std::string const &id);

    /** \brief Minimum protocol version required for the \ref request_enter_virtual_desktop function
     */
    static constexpr std::uint32_t request_enter_virtual_desktop_since_version = 1;

    /** \brief map window on a virtual desktop
  RFC: do this with an empty id to
          request_enter_virtual_desktop?
          Make the window enter a new virtual desktop. If the server consents the request, it will
          create a new virtual desktop and assign the window to it.

    */
    void request_enter_new_virtual_desktop();

    /** \brief Minimum protocol version required for the \ref request_enter_new_virtual_desktop function
     */
    static constexpr std::uint32_t request_enter_new_virtual_desktop_since_version = 1;

    /** \brief remove a window from a virtual desktop
        \param id desktop id

          Make the window exit a virtual desktop. If it exits all desktops it will be considered on
          all of them.

    */
    void request_leave_virtual_desktop(std::string const &id);

    /** \brief Minimum protocol version required for the \ref request_leave_virtual_desktop function
     */
    static constexpr std::uint32_t request_leave_virtual_desktop_since_version = 1;

    /** \brief map window on an activity
        \param id activity id

          Make the window enter an activity. A window can enter more activity. If the id is empty or
          invalid, no action will be performed.

    */
    void request_enter_activity(std::string const &id);

    /** \brief Minimum protocol version required for the \ref request_enter_activity function
     */
    static constexpr std::uint32_t request_enter_activity_since_version = 1;

    /** \brief remove a window from an activity
        \param id activity id

          Make the window exit a an activity. If it exits all activities it will be considered on all
          of them.

    */
    void request_leave_activity(std::string const &id);

    /** \brief Minimum protocol version required for the \ref request_leave_activity function
     */
    static constexpr std::uint32_t request_leave_activity_since_version = 1;

    /** \brief send window to specified output
        \param output

          Requests this window to be displayed in a specific output.

    */
    void send_to_output(output_t const &output);

    /** \brief Minimum protocol version required for the \ref send_to_output function
     */
    static constexpr std::uint32_t send_to_output_since_version = 1;

    /** \brief highlight the window

          Tell the compositor to highlight this window.

    */
    void highlight();

    /** \brief Minimum protocol version required for the \ref highlight function
     */
    static constexpr std::uint32_t highlight_since_version = 1;

    /** \brief unset highlight window

          Tell the compositor to unset highlight window.

    */
    void unset_highlight();

    /** \brief Minimum protocol version required for the \ref unset_highlight function
     */
    static constexpr std::uint32_t unset_highlight_since_version = 1;

    /** \brief window title has been changed
        \param title window title

          This event will be sent as soon as the window title is changed.

    */
    std::function<void(std::string)> &on_title_changed();

    /** \brief application identifier has been changed
        \param app_id

          This event will be sent as soon as the application identifier is changed.

    */
    std::function<void(std::string)> &on_app_id_changed();

    /** \brief window state has been changed
        \param flags bitfield of state flags

          This event will be sent as soon as the window state changes.

          Values for state argument are described by ukui_window_management.state. It contains
          the whole new state of the window.

    */
    std::function<void(uint32_t)> &on_state_changed();

    /** \brief window's icon name changed
        \param name the new themed icon name

          This event will be sent whenever the themed icon name changes. May be null.

    */
    std::function<void(std::string)> &on_themed_icon_name_changed();

    /** \brief window's surface was unmapped

          This event will be sent immediately after the window is closed and its surface is unmapped.

    */
    std::function<void()> &on_unmapped();

    /** \brief All initial known state is submitted

          This event will be sent immediately after all initial state been sent to the client. If the
          Plasma window is already unmapped, the unmapped event will be sent before the initial_state
          event.

    */
    std::function<void()> &on_initial_state();

    /** \brief The parent window changed
        \param parent The parent window

          This event will be sent whenever the parent window of this ukui_window changes. The
          passed parent is another ukui_window and this ukui_window is a transient window to
          the parent window. If the parent argument is null, this ukui_window does not have a
          parent window.

    */
    std::function<void(ukui_window_t)> &on_parent_window();

    /** \brief The geometry of this window in absolute coordinates
        \param x x position of the ukui_window
        \param y y position of the ukui_window
        \param width width of the ukui_window
        \param height height of the ukui_window

          This event will be sent whenever the window geometry of this ukui_window changes. The
          coordinates are in absolute coordinates of the windowing system.

    */
    std::function<void(int32_t, int32_t, uint32_t, uint32_t)> &on_geometry();

    /** \brief The icon of the window changed

          This event will be sent whenever the icon of the window changes, but there is no themed icon
          name. Common examples are Xwayland windows which have a pixmap based icon.

          The client can request the icon using get_icon.

    */
    std::function<void()> &on_icon_changed();

    /** \brief process id of application owning the window has changed
        \param pid process id

          This event will be sent when the compositor has set the process id this window belongs to.
          This should be set once before the initial_state is sent.

    */
    std::function<void(uint32_t)> &on_pid_changed();

    /** \brief the window entered a new virtual desktop
        \param id desktop id

          This event will be sent when the window has entered a new virtual desktop. The window can be
          on more than one desktop, or none: then is considered on all of them.

    */
    std::function<void(std::string)> &on_virtual_desktop_entered();

    /** \brief the window left a virtual desktop
        \param is desktop id

          This event will be sent when the window left a virtual desktop. If the window leaves all
          desktops, it can be considered on all. If the window gets manually added on all desktops,
          the server has to send virtual_desktop_left for every previous desktop it was in for the
          window to be really considered on all desktops.

    */
    std::function<void(std::string)> &on_virtual_desktop_left();

    /** \brief notify the client that the current appmenu changed
        \param service_name
        \param object_path

          This event will be sent after the application menu for the window has changed.

    */
    std::function<void(std::string, std::string)> &on_application_menu();

    /** \brief the window entered an activity
        \param id activity id

          This event will be sent when the window has entered an activity. The window can be on more
          than one activity, or none: then is considered on all of them.

    */
    std::function<void(std::string)> &on_activity_entered();

    /** \brief the window left an activity
        \param id activity id

          This event will be sent when the window left an activity. If the window leaves all
          activities, it will be considered on all. If the window gets manually added on all
          activities, the server has to send activity_left for every previous activity it was in for
          the window to be really considered on all activities.

    */
    std::function<void(std::string)> &on_activity_left();

    /** \brief X11 resource name has changed
        \param resource_name resource name

          This event will be sent when the X11 resource name of the window has changed. This is only
          set for XWayland windows.

    */
    std::function<void(std::string)> &on_resource_name_changed();
};

/** \brief

  */
enum class ukui_window_state : uint32_t
{
    active = 0x1,
    minimized = 0x2,
    maximized = 0x4,
    fullscreen = 0x8,
    keep_above = 0x10,
    keep_below = 0x20,
    on_all_desktops = 0x40,
    demands_attention = 0x80,
    closeable = 0x100,
    minimizable = 0x200,
    maximizable = 0x400,
    fullscreenable = 0x800,
    shadeable = 0x1000,
    shaded = 0x2000,
    movable = 0x4000,
    resizable = 0x8000,
    virtual_desktop_changeable = 0x10000,
    accept_focus = 0x20000,
    skiptaskbar = 0x40000,
    skipswitcher = 0x80000,
    modality = 0x100000
};

/** \brief activation feedback

      The activation manager interface provides a way to get notified when an application is about
      to be activated.

*/
class org_ukui_activation_feedback_t : public proxy_t
{
private:
    struct events_t : public detail::events_base_t
    {
        std::function<void(org_ukui_activation_t)> activation;
    };

    static int dispatcher(uint32_t opcode, const std::vector<detail::any> &args, const std::shared_ptr<detail::events_base_t> &e);

    org_ukui_activation_feedback_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/);

public:
    org_ukui_activation_feedback_t();
    explicit org_ukui_activation_feedback_t(const proxy_t &proxy);
    org_ukui_activation_feedback_t(org_ukui_activation_feedback *p, wrapper_type t = wrapper_type::standard);

    org_ukui_activation_feedback_t proxy_create_wrapper();

    static const std::string interface_name;

    operator org_ukui_activation_feedback *() const;

    /** \brief notify that an app is starting
        \param id

          Will be issued when an app is set to be activated. It offers an instance of
          org_ukui_activation that will tell us the app_id and the extent of the activation.

    */
    std::function<void(org_ukui_activation_t)> &on_activation();
};

/** \brief

*/
class org_ukui_activation_t : public proxy_t
{
private:
    struct events_t : public detail::events_base_t
    {
        std::function<void(std::string)> app_id;
        std::function<void()> finished;
    };

    static int dispatcher(uint32_t opcode, const std::vector<detail::any> &args, const std::shared_ptr<detail::events_base_t> &e);

    org_ukui_activation_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/);

public:
    org_ukui_activation_t();
    explicit org_ukui_activation_t(const proxy_t &proxy);
    org_ukui_activation_t(org_ukui_activation *p, wrapper_type t = wrapper_type::standard);

    org_ukui_activation_t proxy_create_wrapper();

    static const std::string interface_name;

    operator org_ukui_activation *() const;

    /** \brief Offers the app_id
        \param app_id application id, as described in xdg_activation_v1

    */
    std::function<void(std::string)> &on_app_id();

    /** \brief Notifies about activation finished, either by activation or because it got invalidated

    */
    std::function<void()> &on_finished();
};

} // namespace wayland
