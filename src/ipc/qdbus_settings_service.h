#ifndef QDBUS_SETTINGS_SERVICE_H
#define QDBUS_SETTINGS_SERVICE_H

#include <QObject>
#include <QString>

#include "ipc.h"

namespace freewb::ipc
{

class QDBusSettingsService final : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", FREEWUBI_SETTINGS_INTERFACE)

public:
    explicit QDBusSettingsService(QObject *parent = nullptr);
    ~QDBusSettingsService() override;

Q_SIGNALS:
    void signal_switch_input_mode(const QString &inputMode);
    void signal_dict_query(const QString &text);
    void signal_generate_usr_word(int flg, const QString &wordText, const QString &wordCode);
    void signal_delete_usr_word(int flg, const QString &wordText, const QString &wordCode);
    void signal_panel_exit();
    void signal_switch_vk(int flg);
    void signal_show_vk();
    void signal_hide_vk();
    void signal_switch_char_set();
    void signal_switch_simp_or_trad();
    void signal_switch_toolbar_hide_flg();
    void signal_switch_lexicon();
    void signal_switch_mark_auto_pairs_flg();
    void signal_switch_caps_state();
    void signal_switch_char_width();
    void signal_switch_punctuation_mode();
    void signal_open_freewb_dir();
    void signal_word_freq_switch_ok(const QString &wordText, int flg);
    void signal_open_ui_setting();
    void signal_open_advanced_setting();
    void signal_edit_usr_table();
    void signal_edit_wubi_table();
    void signal_edit_pinyin_table();
    void signal_show_version_info();
    void signal_edit_quick_table();
    void signal_set_recode_calib_flg(int flg);

public Q_SLOTS:
    void slot_switch_input_mode(const QString &inputMode);
    void slot_dbus_dict_query(const QString &text);
    void slot_dbus_generate_usr_word(int flg, const QString &wordText, const QString &wordCode);
    void slot_dbus_delete_usr_word(int flg, const QString &wordText, const QString &wordCode);
    void slot_dbus_panel_exit();
    void slot_dbus_switch_vk(int flg);
    void slot_dbus_show_vk();
    void slot_dbus_hide_vk();
    void slot_dbus_switch_char_set();
    void slot_dbus_switch_simp_or_trad();
    void slot_dbus_switch_toolbar_hide_flg();
    void slot_dbus_switch_lexicon();
    void slot_dbus_switch_mark_auto_pairs_flg();
    void slot_dbus_switch_caps_state();
    void slot_dbus_switch_char_width();
    void slot_dbus_switch_punctuation_mode();
    QString slot_dbus_get_clipboard_text();
    void slot_dbus_open_freewb_dir();
    void slot_dbus_word_freq_switch_ok(const QString &wordText, int flg);
    void slot_dbus_open_ui_setting();
    void slot_dbus_open_advanced_setting();
    void slot_dbus_edit_usr_table();
    void slot_dbus_edit_wubi_table();
    void slot_dbus_edit_pinyin_table();
    void slot_dbus_show_version_info();
    void slot_dbus_edit_quick_table();
    void slot_dbus_set_recode_calib_flg(int flg);

private:
    void registerQDBusService();
    void unRegisterQDBusService();
};

} // namespace freewb::ipc

#endif // QDBUS_SETTINGS_SERVICE_H
