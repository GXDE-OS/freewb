#include "settings.h"

#include <array>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace settings
{

const char kDefaultCoustomChar[] = u8R"cc(ˉ  ，  、  ；  ：  ？  ！  …  —  •  〔  （  【  〈  “  〉  《  》  ∶  「  」  『  ［  ）  ＂  ＇  ‘  ”  〕  々  ‖  】  ’  〗  ～  〖  。  ˇ  ¨  `  |  "  』  .  ］  中  国  )cc";
const char kDefaultCoustomMark[] = u8R"cc( ）  ！  ·  #  ￥  %  …  —  *  （                                                     ` ~ － — ＝ + [ { ] } 、 ｜ ； ： ’  ，  。  、 ？ )cc";

namespace
{

void applyOneEntryToIni(CSimpleIniA &ini, const ConfigEntry &e)
{
    switch (e.type)
    {
    case ValueType::Bool:
        ini.SetBoolValue(e.section, e.uniquename, e.value.boolValue, nullptr, true);
        break;
    case ValueType::Int:
        ini.SetLongValue(e.section, e.uniquename, static_cast<long>(e.value.intValue), nullptr, false, true);
        break;
    case ValueType::String:
        ini.SetValue(e.section, e.uniquename, e.value.stringValue.c_str(), nullptr, true);
        break;
    }
}

} // namespace

ConfigEntry::ValueSlots Settings::makeValueSlots(ValueType type, const char *text)
{
    const std::string s = text ? text : "";
    ConfigEntry::ValueSlots slots = {false, 0, s};
    if (type == ValueType::Bool)
    {
        if (s == "true" || s == "1" || s == "yes" || s == "on")
            slots.boolValue = true;
        else if (s == "false" || s == "0" || s == "no" || s == "off")
            slots.boolValue = false;
        else
            slots.boolValue = !s.empty();
    }
    else if (type == ValueType::Int)
        slots.intValue = std::stoi(s);
    return slots;
}

Settings::Settings()
{
    const char *home = std::getenv("HOME");
    std::string prefix = home ? home : "";
    if (!prefix.empty() && prefix.back() != '/')
        prefix += '/';
    ini_path_ = prefix + ".local/freewb/config/config.ini";
    appendEntriesFromDef();
    load();
}

void Settings::load()
{
    CSimpleIniA ini(true, false, false);
    const SI_Error e = ini.LoadFile(ini_path_.c_str());
    if (e < 0 && e != SI_FILE)
        return;
    for (auto &kv : entries_)
    {
        ConfigEntry &e = kv.second;
        switch (e.type)
        {
        case ValueType::Bool:
        {
            e.value.boolValue = ini.GetBoolValue(e.section, e.uniquename, e.defaultValue.boolValue);
            break;
        }
        case ValueType::Int:
        {
            e.value.intValue = static_cast<int>(ini.GetLongValue(e.section, e.uniquename, static_cast<long>(e.defaultValue.intValue)));
            break;
        }
        case ValueType::String:
        {
            const char *v = ini.GetValue(e.section, e.uniquename, e.defaultValue.stringValue.c_str(), nullptr);
            e.value.stringValue = v ? std::string(v) : e.defaultValue.stringValue;
            break;
        }
        }
    }
}

ConfigEntry *Settings::findEntryByName(const std::string &uniquename)
{
    const auto it = entries_.find(uniquename);
    if (it == entries_.end())
        return nullptr;
    return &it->second;
}

void Settings::persistEntry(const ConfigEntry &entry)
{
    CSimpleIniA ini(true, false, false);
    const SI_Error err = ini.LoadFile(ini_path_.c_str());
    if (err < 0 && err != SI_FILE)
        return;
    applyOneEntryToIni(ini, entry);
    (void)ini.SaveFile(ini_path_.c_str(), false);
}

bool Settings::readBool(const std::string &uniquename) const
{
    const auto it = entries_.find(uniquename);
    return it == entries_.end() ? false : it->second.value.boolValue;
}

int Settings::readInt(const std::string &uniquename) const
{
    const auto it = entries_.find(uniquename);
    return it == entries_.end() ? 0 : it->second.value.intValue;
}

const std::string &Settings::readString(const std::string &uniquename) const
{
    static const std::string kEmpty;
    const auto it = entries_.find(uniquename);
    return it == entries_.end() ? kEmpty : it->second.value.stringValue;
}

void Settings::writeBool(const std::string &uniquename, bool value)
{
    ConfigEntry *e = findEntryByName(uniquename);
    if (!e)
        return;
    e->value.boolValue = value;
    persistEntry(*e);
}

void Settings::writeInt(const std::string &uniquename, int value)
{
    ConfigEntry *e = findEntryByName(uniquename);
    if (!e)
        return;
    e->value.intValue = value;
    persistEntry(*e);
}

void Settings::writeString(const std::string &uniquename, const std::string &value)
{
    ConfigEntry *e = findEntryByName(uniquename);
    if (!e)
        return;
    e->value.stringValue = value;
    persistEntry(*e);
}

bool Settings::restore_default_shortcutkey()
{
    for (auto &kv : entries_)
    {
        if (std::string(kv.second.section) == "ShortcutKey")
            kv.second.value = kv.second.defaultValue;
    }
    return save();
}

void Settings::restoreAllDefaults()
{
    for (auto &kv : entries_)
        kv.second.value = kv.second.defaultValue;
}

bool Settings::save()
{
    CSimpleIniA ini(true, false, false);
    (void)ini.LoadFile(ini_path_.c_str());
    for (const auto &kv : entries_)
        applyOneEntryToIni(ini, kv.second);
    return ini.SaveFile(ini_path_.c_str(), false) >= 0;
}

void Settings::reload()
{
    for (auto &kv : entries_)
        kv.second.value = kv.second.defaultValue;
    load();
}

Settings &instance()
{
    static Settings s;
    return s;
}

} // namespace settings
