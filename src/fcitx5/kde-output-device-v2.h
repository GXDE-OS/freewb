#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <wayland-client.hpp>

struct kde_output_device_v2;
struct kde_output_device_mode_v2;

namespace wayland
{
class kde_output_device_v2_t;
enum class kde_output_device_v2_subpixel : uint32_t;
enum class kde_output_device_v2_transform : uint32_t;
struct kde_output_device_v2_capability;
enum class kde_output_device_v2_vrr_policy : uint32_t;
enum class kde_output_device_v2_rgb_range : uint32_t;
enum class kde_output_device_v2_auto_rotate_policy : uint32_t;
enum class kde_output_device_v2_color_profile_source : uint32_t;
class kde_output_device_mode_v2_t;

namespace detail
{
extern const wl_interface kde_output_device_v2_interface;
extern const wl_interface kde_output_device_mode_v2_interface;
} // namespace detail

/** \brief output configuration representation

      An output device describes a display device available to the compositor.
      output_device is similar to wl_output, but focuses on output
      configuration management.

      A client can query all global output_device objects to enlist all
      available display devices, even those that may currently not be
      represented by the compositor as a wl_output.

      The client sends configuration changes to the server through the
      outputconfiguration interface, and the server applies the configuration
      changes to the hardware and signals changes to the output devices
      accordingly.

      This object is published as global during start up for every available
      display devices, or when one later becomes available, for example by
      being hotplugged via a physical connector.

*/
class kde_output_device_v2_t : public proxy_t
{
private:
    struct events_t : public detail::events_base_t
    {
        std::function<void(int32_t, int32_t, int32_t, int32_t, int32_t, std::string, std::string, int32_t)> geometry;
        std::function<void(kde_output_device_mode_v2_t)> current_mode;
        std::function<void(kde_output_device_mode_v2_t)> mode;
        std::function<void()> done;
        std::function<void(double)> scale;
        std::function<void(std::string)> edid;
        std::function<void(int32_t)> enabled;
        std::function<void(std::string)> uuid;
        std::function<void(std::string)> serial_number;
        std::function<void(std::string)> eisa_id;
        std::function<void(kde_output_device_v2_capability)> capabilities;
        std::function<void(uint32_t)> overscan;
        std::function<void(kde_output_device_v2_vrr_policy)> vrr_policy;
        std::function<void(kde_output_device_v2_rgb_range)> rgb_range;
        std::function<void(std::string)> name;
        std::function<void(uint32_t)> high_dynamic_range;
        std::function<void(uint32_t)> sdr_brightness;
        std::function<void(uint32_t)> wide_color_gamut;
        std::function<void(kde_output_device_v2_auto_rotate_policy)> auto_rotate_policy;
        std::function<void(std::string)> icc_profile_path;
        std::function<void(uint32_t, uint32_t, uint32_t)> brightness_metadata;
        std::function<void(int32_t, int32_t, int32_t)> brightness_overrides;
        std::function<void(uint32_t)> sdr_gamut_wideness;
        std::function<void(kde_output_device_v2_color_profile_source)> color_profile_source;
        std::function<void(uint32_t)> brightness;
    };

    static int dispatcher(uint32_t opcode, const std::vector<detail::any> &args, const std::shared_ptr<detail::events_base_t> &e);

    kde_output_device_v2_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/);

public:
    kde_output_device_v2_t();
    explicit kde_output_device_v2_t(const proxy_t &proxy);
    kde_output_device_v2_t(kde_output_device_v2 *p, wrapper_type t = wrapper_type::standard);

    kde_output_device_v2_t proxy_create_wrapper();

    static const std::string interface_name;

    operator kde_output_device_v2 *() const;

    /** \brief geometric properties of the output
        \param x x position within the global compositor space
        \param y y position within the global compositor space
        \param physical_width width in millimeters of the output
        \param physical_height height in millimeters of the output
        \param subpixel subpixel orientation of the output
        \param make textual description of the manufacturer
        \param model textual description of the model
        \param transform transform that maps framebuffer to output

          The geometry event describes geometric properties of the output.
          The event is sent when binding to the output object and whenever
          any of the properties change.

    */
    std::function<void(int32_t, int32_t, int32_t, int32_t, int32_t, std::string, std::string, int32_t)> &on_geometry();

    /** \brief current mode
        \param mode

          This event describes the mode currently in use for this head. It is only
          sent if the output is enabled.

    */
    std::function<void(kde_output_device_mode_v2_t)> &on_current_mode();

    /** \brief advertise available output modes and current one
        \param mode

          The mode event describes an available mode for the output.

          When the client binds to the output_device object, the server sends this
          event once for every available mode the output_device can be operated by.

          There will always be at least one event sent out on initial binding,
          which represents the current mode.

          Later if an output changes, its mode event is sent again for the
          eventual added modes and lastly the current mode. In other words, the
          current mode is always represented by the latest event sent with the current
          flag set.

          The size of a mode is given in physical hardware units of the output device.
          This is not necessarily the same as the output size in the global compositor
          space. For instance, the output may be scaled, as described in
          kde_output_device_v2.scale, or transformed, as described in
          kde_output_device_v2.transform.

    */
    std::function<void(kde_output_device_mode_v2_t)> &on_mode();

    /** \brief sent all information about output

          This event is sent after all other properties have been
          sent on binding to the output object as well as after any
          other output property change have been applied later on.
          This allows to see changes to the output properties as atomic,
          even if multiple events successively announce them.

    */
    std::function<void()> &on_done();

    /** \brief output scaling properties
        \param factor scaling factor of output

          This event contains scaling geometry information
          that is not in the geometry event. It may be sent after
          binding the output object or if the output scale changes
          later. If it is not sent, the client should assume a
          scale of 1.

          A scale larger than 1 means that the compositor will
          automatically scale surface buffers by this amount
          when rendering. This is used for high resolution
          displays where applications rendering at the native
          resolution would be too small to be legible.

          It is intended that scaling aware clients track the
          current output of a surface, and if it is on a scaled
          output it should use wl_surface.set_buffer_scale with
          the scale of the output. That way the compositor can
          avoid scaling the surface, and the client can supply
          a higher detail image.

    */
    std::function<void(double)> &on_scale();

    /** \brief advertise EDID data for the output
        \param raw base64-encoded EDID string

          The edid event encapsulates the EDID data for the outputdevice.

          The event is sent when binding to the output object. The EDID
          data may be empty, in which case this event is sent anyway.
          If the EDID information is empty, you can fall back to the name
          et al. properties of the outputdevice.

    */
    std::function<void(std::string)> &on_edid();

    /** \brief output is enabled or disabled
        \param enabled output enabled state

          The enabled event notifies whether this output is currently
          enabled and used for displaying content by the server.
          The event is sent when binding to the output object and
          whenever later on an output changes its state by becoming
          enabled or disabled.

    */
    std::function<void(int32_t)> &on_enabled();

    /** \brief A unique id for this outputdevice
        \param uuid output devices ID

          The uuid can be used to identify the output. It's controlled by
          the server entirely. The server should make sure the uuid is
          persistent across restarts. An empty uuid is considered invalid.

    */
    std::function<void(std::string)> &on_uuid();

    /** \brief Serial Number
        \param serialNumber textual representation of serial number

          Serial ID of the monitor, sent on startup before the first done event.

    */
    std::function<void(std::string)> &on_serial_number();

    /** \brief EISA ID
        \param eisaId textual representation of EISA identifier

          EISA ID of the monitor, sent on startup before the first done event.

    */
    std::function<void(std::string)> &on_eisa_id();

    /** \brief capability flags
        \param flags

          What capabilities this device has, sent on startup before the first
          done event.

    */
    std::function<void(kde_output_device_v2_capability)> &on_capabilities();

    /** \brief overscan
        \param overscan amount of overscan of the monitor

          Overscan value of the monitor in percent, sent on startup before the
          first done event.

    */
    std::function<void(uint32_t)> &on_overscan();

    /** \brief Variable Refresh Rate Policy
        \param vrr_policy

          What policy the compositor will employ regarding its use of variable
          refresh rate.

    */
    std::function<void(kde_output_device_v2_vrr_policy)> &on_vrr_policy();

    /** \brief RGB range
        \param rgb_range

          What rgb range the compositor is using for this output

    */
    std::function<void(kde_output_device_v2_rgb_range)> &on_rgb_range();

    /** \brief Output's name
        \param name

          Name of the output, it's useful to cross-reference to an zxdg_output_v1 and ultimately QScreen

    */
    std::function<void(std::string)> &on_name();

    /** \brief if HDR is enabled
        \param hdr_enabled 1 if enabled, 0 if disabled

          Whether or not high dynamic range is enabled for this output

    */
    std::function<void(uint32_t)> &on_high_dynamic_range();

    /** \brief the brightness of sdr if hdr is enabled
        \param sdr_brightness

          If high dynamic range is used, this value defines the brightness in nits for content
          that's in standard dynamic range format. Note that while the value is in nits, that
          doesn't necessarily translate to the same brightness on the screen.

    */
    std::function<void(uint32_t)> &on_sdr_brightness();

    /** \brief if WCG is enabled
        \param wcg_enabled 1 if enabled, 0 if disabled

          Whether or not the use of a wide color gamut is enabled for this output

    */
    std::function<void(uint32_t)> &on_wide_color_gamut();

    /** \brief describes when auto rotate is used
        \param policy

    */
    std::function<void(kde_output_device_v2_auto_rotate_policy)> &on_auto_rotate_policy();

    /** \brief describes when auto rotate is used
        \param profile_path

    */
    std::function<void(std::string)> &on_icc_profile_path();

    /** \brief metadata about the screen's brightness limits
        \param max_peak_brightness in nits
        \param max_frame_average_brightness in nits
        \param min_brightness in 0.0001 nits

    */
    std::function<void(uint32_t, uint32_t, uint32_t)> &on_brightness_metadata();

    /** \brief overrides for the screen's brightness limits
        \param max_peak_brightness -1 for no override, positive values are the brightness in nits
        \param max_average_brightness -1 for no override, positive values are the brightness in nits
        \param min_brightness -1 for no override, positive values are the brightness in 0.0001 nits

    */
    std::function<void(int32_t, int32_t, int32_t)> &on_brightness_overrides();

    /** \brief describes which gamut is assumed for sRGB applications
        \param gamut_wideness 0 means rec.709 primaries, 10000 means native primaries

            This can be used to provide the colors users assume sRGB applications should have based on the
            default experience on many modern sRGB screens.

    */
    std::function<void(uint32_t)> &on_sdr_gamut_wideness();

    /** \brief describes which source the compositor uses for the color profile on an output
        \param source

    */
    std::function<void(kde_output_device_v2_color_profile_source)> &on_color_profile_source();

    /** \brief brightness multiplier
        \param brightness brightness in 0-10000

          This is the brightness modifier of the output. It doesn't specify
          any absolute values, but is merely a multiplier on top of other
          brightness values, like sdr_brightness and brightness_metadata.
          0 is the minimum brightness (not completely dark) and 10000 is
          the maximum brightness.
          This is currently only supported / meaningful while HDR is active.

    */
    std::function<void(uint32_t)> &on_brightness();
};

/** \brief subpixel geometry information

        This enumeration describes how the physical pixels on an output are
        laid out.

  */
enum class kde_output_device_v2_subpixel : uint32_t
{
    unknown = 0,
    none = 1,
    horizontal_rgb = 2,
    horizontal_bgr = 3,
    vertical_rgb = 4,
    vertical_bgr = 5
};

/** \brief transform from framebuffer to output

        This describes the transform, that a compositor will apply to a
        surface to compensate for the rotation or mirroring of an
        output device.

        The flipped values correspond to an initial flip around a
        vertical axis followed by rotation.

        The purpose is mainly to allow clients to render accordingly and
        tell the compositor, so that for fullscreen surfaces, the
        compositor is still able to scan out directly client surfaces.

  */
enum class kde_output_device_v2_transform : uint32_t
{
    normal = 0,
    _90 = 1,
    _180 = 2,
    _270 = 3,
    flipped = 4,
    flipped_90 = 5,
    flipped_180 = 6,
    flipped_270 = 7
};

/** \brief describes capabilities of the outputdevice

        Describes what capabilities this device has.

  */
struct kde_output_device_v2_capability : public wayland::detail::bitfield<7, 2>
{
    kde_output_device_v2_capability(const wayland::detail::bitfield<7, 2> &b) : wayland::detail::bitfield<7, 2>(b)
    {
    }

    kde_output_device_v2_capability(const uint32_t value) : wayland::detail::bitfield<7, 2>(value)
    {
    }

    /** \brief if this output_device can use overscan */
    static const wayland::detail::bitfield<7, 2> overscan;
    /** \brief if this outputdevice supports variable refresh rate */
    static const wayland::detail::bitfield<7, 2> vrr;
    /** \brief if setting the rgb range is possible */
    static const wayland::detail::bitfield<7, 2> rgb_range;
    /** \brief if this outputdevice supports high dynamic range */
    static const wayland::detail::bitfield<7, 2> high_dynamic_range;
    /** \brief if this outputdevice supports a wide color gamut */
    static const wayland::detail::bitfield<7, 2> wide_color_gamut;
    /** \brief if this outputdevice supports autorotation */
    static const wayland::detail::bitfield<7, 2> auto_rotate;
    /** \brief if this outputdevice supports icc profiles */
    static const wayland::detail::bitfield<7, 2> icc_profile;
};

/** \brief describes vrr policy

        Describes when the compositor may employ variable refresh rate

  */
enum class kde_output_device_v2_vrr_policy : uint32_t
{
    never = 0,
    always = 1,
    automatic = 2
};

/** \brief describes RGB range policy

        Whether full or limited color range should be used

  */
enum class kde_output_device_v2_rgb_range : uint32_t
{
    automatic = 0,
    full = 1,
    limited = 2
};

/** \brief describes when auto rotate should be used

  */
enum class kde_output_device_v2_auto_rotate_policy : uint32_t
{
    never = 0,
    in_tablet_mode = 1,
    always = 2
};

/** \brief which source the compositor should use for the color profile on an output

  */
enum class kde_output_device_v2_color_profile_source : uint32_t
{
    sRGB = 0,
    ICC = 1,
    EDID = 2
};

/** \brief output mode

      This object describes an output mode.

      Some heads don't support output modes, in which case modes won't be
      advertised.

      Properties sent via this interface are applied atomically via the
      kde_output_device.done event. No guarantees are made regarding the order
      in which properties are sent.

*/
class kde_output_device_mode_v2_t : public proxy_t
{
private:
    struct events_t : public detail::events_base_t
    {
        std::function<void(int32_t, int32_t)> size;
        std::function<void(int32_t)> refresh;
        std::function<void()> preferred;
        std::function<void()> removed;
    };

    static int dispatcher(uint32_t opcode, const std::vector<detail::any> &args, const std::shared_ptr<detail::events_base_t> &e);

    kde_output_device_mode_v2_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/);

public:
    kde_output_device_mode_v2_t();
    explicit kde_output_device_mode_v2_t(const proxy_t &proxy);
    kde_output_device_mode_v2_t(kde_output_device_mode_v2 *p, wrapper_type t = wrapper_type::standard);

    kde_output_device_mode_v2_t proxy_create_wrapper();

    static const std::string interface_name;

    operator kde_output_device_mode_v2 *() const;

    /** \brief mode size
        \param width width of the mode in hardware units
        \param height height of the mode in hardware units

          This event describes the mode size. The size is given in physical
          hardware units of the output device. This is not necessarily the same as
          the output size in the global compositor space. For instance, the output
          may be scaled or transformed.

    */
    std::function<void(int32_t, int32_t)> &on_size();

    /** \brief mode refresh rate
        \param refresh vertical refresh rate in mHz

          This event describes the mode's fixed vertical refresh rate. It is only
          sent if the mode has a fixed refresh rate.

    */
    std::function<void(int32_t)> &on_refresh();

    /** \brief mode is preferred

          This event advertises this mode as preferred.

    */
    std::function<void()> &on_preferred();

    /** \brief the mode has been destroyed

          The compositor will destroy the object immediately after sending this
          event, so it will become invalid and the client should release any
          resources associated with it.

    */
    std::function<void()> &on_removed();
};

} // namespace wayland
