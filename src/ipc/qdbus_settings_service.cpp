#include "qdbus_settings_service.h"

#include <QClipboard>
#include <QDBusConnection>
#include <QGuiApplication>

#include "log.h"

namespace freewb::ipc
{

QDBusSettingsService::QDBusSettingsService(QObject *parent) : QObject(parent)
{
    registerQDBusService();
}

QDBusSettingsService::~QDBusSettingsService()
{
    unRegisterQDBusService();
}

void QDBusSettingsService::registerQDBusService()
{
    if (!QDBusConnection::connectToBus(QDBusConnection::SessionBus, FREEWUBI_SETTINGS_BUSNAME)
             .registerService(FREEWUBI_SETTINGS_SERVICENAME))
    {
        FREEWB_ERROR("QDBusSettingsService: registerService failed");
        return;
    }

    const bool ok = QDBusConnection::connectToBus(QDBusConnection::SessionBus, FREEWUBI_SETTINGS_BUSNAME)
                        .registerObject(FREEWUBI_SETTINGS_OBJECTPATH, this, QDBusConnection::ExportAllSlots);
    FREEWB_DEBUG("QDBusSettingsService: registerObject {}", ok);
}

void QDBusSettingsService::unRegisterQDBusService()
{
    if (!QDBusConnection(FREEWUBI_SETTINGS_BUSNAME).isConnected())
    {
        return;
    }
    QDBusConnection(FREEWUBI_SETTINGS_BUSNAME).unregisterObject(FREEWUBI_SETTINGS_OBJECTPATH);
    QDBusConnection(FREEWUBI_SETTINGS_BUSNAME).unregisterService(FREEWUBI_SETTINGS_SERVICENAME);
    QDBusConnection::disconnectFromBus(FREEWUBI_SETTINGS_BUSNAME);
}

void QDBusSettingsService::slot_switch_input_mode(const QString &inputMode)
{
    emit signal_switch_input_mode(inputMode);
}

void QDBusSettingsService::slot_dbus_dict_query(const QString &text)
{
    emit signal_dict_query(text);
}

void QDBusSettingsService::slot_dbus_generate_usr_word(int flg, const QString &wordText, const QString &wordCode)
{
    emit signal_generate_usr_word(flg, wordText, wordCode);
}

void QDBusSettingsService::slot_dbus_delete_usr_word(int flg, const QString &wordText, const QString &wordCode)
{
    emit signal_delete_usr_word(flg, wordText, wordCode);
}

void QDBusSettingsService::slot_dbus_panel_exit()
{
    emit signal_panel_exit();
}

void QDBusSettingsService::slot_dbus_switch_vk(int flg)
{
    emit signal_switch_vk(flg);
}

void QDBusSettingsService::slot_dbus_show_vk()
{
    emit signal_show_vk();
}

void QDBusSettingsService::slot_dbus_hide_vk()
{
    emit signal_hide_vk();
}

void QDBusSettingsService::slot_dbus_switch_char_set()
{
    emit signal_switch_char_set();
}

void QDBusSettingsService::slot_dbus_switch_simp_or_trad()
{
    emit signal_switch_simp_or_trad();
}

void QDBusSettingsService::slot_dbus_switch_toolbar_hide_flg()
{
    emit signal_switch_toolbar_hide_flg();
}

void QDBusSettingsService::slot_dbus_switch_lexicon()
{
    emit signal_switch_lexicon();
}

void QDBusSettingsService::slot_dbus_switch_mark_auto_pairs_flg()
{
    emit signal_switch_mark_auto_pairs_flg();
}

void QDBusSettingsService::slot_dbus_switch_caps_state()
{
    emit signal_switch_caps_state();
}

void QDBusSettingsService::slot_dbus_switch_char_width()
{
    emit signal_switch_char_width();
}

void QDBusSettingsService::slot_dbus_switch_punctuation_mode()
{
    emit signal_switch_punctuation_mode();
}

QString QDBusSettingsService::slot_dbus_get_clipboard_text()
{
    if (QClipboard *clipboard = QGuiApplication::clipboard())
    {
        return clipboard->text();
    }
    return {};
}

void QDBusSettingsService::slot_dbus_open_freewb_dir()
{
    emit signal_open_freewb_dir();
}

void QDBusSettingsService::slot_dbus_word_freq_switch_ok(const QString &wordText, int flg)
{
    emit signal_word_freq_switch_ok(wordText, flg);
}

void QDBusSettingsService::slot_dbus_open_ui_setting()
{
    emit signal_open_ui_setting();
}

void QDBusSettingsService::slot_dbus_open_advanced_setting()
{
    emit signal_open_advanced_setting();
}

void QDBusSettingsService::slot_dbus_edit_usr_table()
{
    emit signal_edit_usr_table();
}

void QDBusSettingsService::slot_dbus_edit_wubi_table()
{
    emit signal_edit_wubi_table();
}

void QDBusSettingsService::slot_dbus_edit_pinyin_table()
{
    emit signal_edit_pinyin_table();
}

void QDBusSettingsService::slot_dbus_show_version_info()
{
    emit signal_show_version_info();
}

void QDBusSettingsService::slot_dbus_edit_quick_table()
{
    emit signal_edit_quick_table();
}

void QDBusSettingsService::slot_dbus_set_recode_calib_flg(int flg)
{
    emit signal_set_recode_calib_flg(flg);
}

} // namespace freewb::ipc