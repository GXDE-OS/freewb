#include "settingshelper.h"
#include "settings.h"

#include <array>
#include <utility>
#include <vector>

namespace
{
struct UiRuntimeState
{
    bool useAudioFile = true;
    bool showAllGroup = false;
    std::vector<std::string> skinList;
    std::vector<std::string> lexiconList;
};

UiRuntimeState g_uiRuntimeState;

const std::array<std::string, SSK_NUM> &singleShortcutIniTokens()
{
    static const std::array<std::string, SSK_NUM> k = {{
        "KEY_NONE", ";", "'", ",", ".", "`", "[", "]", "\\", "/", "u", "i", "v", "z"}};
    return k;
}

const std::array<const char *, CSK_NUM> &combineShortcutTokenCStrs()
{
    static const std::array<const char *, CSK_NUM> k = {{
        "KEY_NONE", "KEY_INSERT", "KEY_DEL", "KEY_ESC", "KEY_BACKSPACE", "KEY_HOME", "KEY_END", "KEY_LEFT", "KEY_RIGHT",
        "KEY_UP", "KEY_DOWN", "KEY_QUOTE", "KEY_SEMICOLON", "KEY_BACK_SLASH", "KEY_LEFT_BRACKET", "KEY_RIGHT_BRACKET",
        "KEY_COMMA", "KEY_PERIOD", "KEY_SLASH", "KEY_BACKQUOTE", "KEY_EQUAL", "KEY_DASH", "KEY_F1", "KEY_F2", "KEY_F3",
        "KEY_F4", "KEY_F5", "KEY_F6", "KEY_F7", "KEY_F8", "KEY_F9", "KEY_F10", "KEY_F11", "KEY_F12", "KEY_A", "KEY_B",
        "KEY_C", "KEY_D", "KEY_E", "KEY_F", "KEY_G", "KEY_H", "KEY_I", "KEY_J", "KEY_K", "KEY_L", "KEY_M", "KEY_N",
        "KEY_O", "KEY_P", "KEY_Q", "KEY_R", "KEY_S", "KEY_T", "KEY_U", "KEY_V", "KEY_W", "KEY_X", "KEY_Y", "KEY_Z"}};
    return k;
}

const std::array<std::string, CSK_NUM> &combineShortcutDisplayNames()
{
    static const std::array<std::string, CSK_NUM> k = {{
        "禁止", "Insert", "Del", "Esc", "Backspace", "Home", "End", "←", "→", "↑", "↓", "引号( ' )", "分号( ; )",
        "反斜杠( \\ )", "左中( [ )", "右中( ] )", "逗号( , )", "句号( . )", "除号( / )", "反引号( ` )", "=", "-",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "A", "B", "C", "D", "E", "F", "G",
        "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"}};
    return k;
}

const std::array<std::string, SSK_NUM> &singleShortcutDisplayNames()
{
    static const std::array<std::string, SSK_NUM> k = {{
        "禁止", "分号( ; )", "引号( ' )", "逗号( , )", "句号( . )", "反引号( ` )", "左中( [ )", "右中( ] )",
        "反斜( \\ )", "除号( / )", "u", "i", "v", "z"}};
    return k;
}

struct CandiPageEntry
{
    const char *token;
    int index;
};

const std::array<CandiPageEntry, 4> &candiPageTable()
{
    static const std::array<CandiPageEntry, 4> k = {{
        CandiPageEntry{"CPSK_COMMA_PERIOD", CPSK_COMMA_PERIOD},
        {"CPSK_UP_DWON", CPSK_UP_DWON},
        {"CPSK_PAGE_UP_DWON", CPSK_PAGE_UP_DWON},
        {"CPSK_SUB_EQUAL", CPSK_SUB_EQUAL},
    }};
    return k;
}

/** 下标须与 candiPageTable() 中 CandiPageShortcutKey 索引一致 */
const std::array<std::pair<const char *, const char *>, 4> &candidatePageHotkeyTable()
{
    static const std::array<std::pair<const char *, const char *>, 4> k = {{
        {"KEY_DASH", "KEY_EQUAL"},
        {"KEY_COMMA", "KEY_PERIOD"},
        {"KEY_UP", "KEY_DOWN"},
        {"KEY_PAGE_UP", "KEY_PAGE_DWON"},
    }};
    return k;
}

struct CnEnSwitchEntry
{
    const char *token;
    int index;
};

const std::array<CnEnSwitchEntry, 6> &cnEnSwitchTable()
{
    static const std::array<CnEnSwitchEntry, 6> k = {{
        CnEnSwitchEntry{"KEY_LEFT_SHIFT", CESSK_LEFT_SHIFT},
        {"KEY_RIGHT_SHIFT", CESSK_RIGHT_SHIFT},
        {"KEY_SHIFT", CESSK_SHIFT},
        {"KEY_LEFT_CTRL", CESSK_LEFT_CTRL},
        {"KEY_RIGHT_CTRL", CESSK_RIGHT_CTRL},
        {"KEY_CTRL", CESSK_CTRL},
    }};
    return k;
}

const std::array<std::pair<const char *, const char *>, static_cast<size_t>(RSSK_SHIFT) + 1> &recodeSelectPairTable()
{
    static const std::array<std::pair<const char *, const char *>, static_cast<size_t>(RSSK_SHIFT) + 1> k = {{
        {"KEY_NONE", "KEY_NONE"},
        {"KEY_SEMICOLON", "KEY_QUOTE"},
        {"KEY_COMMA", "KEY_PERIOD"},
        {"KEY_LEFT_CTRL", "KEY_RIGHT_CTRL"},
        {"KEY_LEFT_SHIFT", "KEY_RIGHT_SHIFT"},
    }};
    return k;
}

} // namespace

SettingsNotifier::SettingsNotifier(QObject *parent) : QObject(parent) {}

void SettingsNotifier::notifySettingDataChangedToLocal()
{
    emit signal_setting_data_changed_to_local();
}

void SettingsNotifier::notifySettingDataChangedToFcitx()
{
    emit signal_setting_data_changed_to_fcitx();
}

SettingsNotifier g_settingsNotifier;

QString toQStringUtf8(const std::string &s)
{
    return QString::fromUtf8(s.data(), static_cast<int>(s.size()));
}

std::string fromStdUtf8(const QString &q)
{
    const QByteArray b = q.toUtf8();
    return std::string(b.constData(), b.size());
}

CustomKeyValue customKeyToQt(const CustomKeyStrings &s)
{
    CustomKeyValue v;
    v.commChar = toQStringUtf8(s.commChar);
    v.shiftChar = toQStringUtf8(s.shiftChar);
    v.commMark = toQStringUtf8(s.commMark);
    v.shiftMark = toQStringUtf8(s.shiftMark);
    return v;
}

CustomKeyStrings customKeyFromQt(const CustomKeyValue &v)
{
    CustomKeyStrings s;
    s.commChar = fromStdUtf8(v.commChar);
    s.shiftChar = fromStdUtf8(v.shiftChar);
    s.commMark = fromStdUtf8(v.commMark);
    s.shiftMark = fromStdUtf8(v.shiftMark);
    return s;
}

QFont freewb_candi_text_qfont(const settings::Settings &cfg)
{
    QFont f;
    f.setFamily(toQStringUtf8(cfg.get_candiTextFontName()));
    int psz = cfg.get_candiTextFontSize();
    if (psz <= 0)
        psz = 14;
    f.setPointSize(psz);
    if (f.family().isEmpty())
        f = QFont(QStringLiteral("Ubuntu"), 14);
    return f;
}

QString freewb_candi_text_font_display_label(const settings::Settings &cfg)
{
    return QStringLiteral("%1 %2").arg(toQStringUtf8(cfg.get_candiTextFontName())).arg(cfg.get_candiTextFontSize());
}

void freewb_candi_text_font_apply_qfont(settings::Settings &cfg, const QFont &font)
{
    int psz = font.pointSize();
    if (psz <= 0)
        psz = font.pixelSize() > 0 ? font.pixelSize() : 14;
    cfg.set_candiTextFontName(fromStdUtf8(font.family()));
    cfg.set_candiTextFontSize(psz);
}

bool freewb_runtime_use_audio_file()
{
    return g_uiRuntimeState.useAudioFile;
}

void freewb_runtime_set_use_audio_file(bool enabled)
{
    g_uiRuntimeState.useAudioFile = enabled;
}

bool freewb_runtime_toggle_show_all_group()
{
    g_uiRuntimeState.showAllGroup = !g_uiRuntimeState.showAllGroup;
    return g_uiRuntimeState.showAllGroup;
}

bool freewb_runtime_show_all_group()
{
    return g_uiRuntimeState.showAllGroup;
}

const std::vector<std::string> &freewb_runtime_skin_list()
{
    return g_uiRuntimeState.skinList;
}

void freewb_runtime_set_skin_list(const std::vector<std::string> &skinList)
{
    g_uiRuntimeState.skinList = skinList;
}

const std::vector<std::string> &freewb_runtime_lexicon_list()
{
    return g_uiRuntimeState.lexiconList;
}

void freewb_runtime_set_lexicon_list(const std::vector<std::string> &lexiconList)
{
    g_uiRuntimeState.lexiconList = lexiconList;
}

char freewb_separate_char_from_string(const std::string &value)
{
    return value.empty() ? 0 : value.front();
}

std::string freewb_separate_char_to_string(char ch)
{
    return ch == 0 ? std::string() : std::string(1, ch);
}

int freewb_candi_page_index_from_token(const std::string &token)
{
    for (const CandiPageEntry &e : candiPageTable())
    {
        if (token == e.token)
            return e.index;
    }
    return CPSK_SUB_EQUAL;
}

std::string freewb_candi_page_token_from_index(int index)
{
    for (const CandiPageEntry &e : candiPageTable())
    {
        if (index == e.index)
            return e.token;
    }
    return "CPSK_SUB_EQUAL";
}

void freewb_apply_candidate_page_hotkeys(settings::Settings &cfg, CandiPageShortcutKey scheme)
{
    const int i = static_cast<int>(scheme);
    const auto &t = candidatePageHotkeyTable();
    if (i < 0 || static_cast<size_t>(i) >= t.size())
        return;
    const auto &p = t[static_cast<size_t>(i)];
    cfg.set_prevPageKey(p.first);
    cfg.set_nextPageKey(p.second);
}

int freewb_cn_en_switch_index_from_token(const std::string &token)
{
    for (const CnEnSwitchEntry &e : cnEnSwitchTable())
    {
        if (token == e.token)
            return e.index;
    }
    return CESSK_NONE;
}

std::string freewb_cn_en_switch_token_from_index(int index)
{
    for (const CnEnSwitchEntry &e : cnEnSwitchTable())
    {
        if (index == e.index)
            return e.token;
    }
    return "KEY_NONE";
}

void freewb_apply_recode_select_pair(settings::Settings &cfg, RcodeSelectShortcutKey key)
{
    const int i = static_cast<int>(key);
    const auto &k = recodeSelectPairTable();
    if (i < 0 || static_cast<size_t>(i) >= k.size())
        return;
    const auto &p = k[static_cast<size_t>(i)];
    cfg.set_secondRecodeKey(p.first);
    cfg.set_thirdRecodeKey(p.second);
}

int freewb_single_shortcut_index_from_token(const std::string &token)
{
    const auto &ini = singleShortcutIniTokens();
    for (int i = 0; i < SSK_NUM; ++i)
    {
        if (token == ini[static_cast<size_t>(i)])
            return i;
    }
    return SSK_NONE;
}

const std::string &freewb_single_shortcut_display_name(int index)
{
    static const std::string kEmpty;
    const auto &k = singleShortcutDisplayNames();
    if (index < 0 || index >= SSK_NUM)
        return kEmpty;
    return k[static_cast<size_t>(index)];
}

const std::string &freewb_single_shortcut_ini_token(int index)
{
    static const std::string kEmpty;
    const auto &k = singleShortcutIniTokens();
    if (index < 0 || index >= SSK_NUM)
        return kEmpty;
    return k[static_cast<size_t>(index)];
}

const std::string &freewb_single_shortcut_ini_from_stored(const std::string &stored)
{
    const auto &ini = singleShortcutIniTokens();
    for (int i = 0; i < SSK_NUM; ++i)
    {
        if (stored == ini[static_cast<size_t>(i)])
            return ini[static_cast<size_t>(i)];
    }
    return ini[static_cast<size_t>(SSK_NONE)];
}

const std::string &freewb_single_shortcut_display_from_stored(const std::string &stored)
{
    const auto &ini = singleShortcutIniTokens();
    const auto &disp = singleShortcutDisplayNames();
    for (int i = 0; i < SSK_NUM; ++i)
    {
        if (stored == ini[static_cast<size_t>(i)])
            return disp[static_cast<size_t>(i)];
    }
    return disp[static_cast<size_t>(SSK_NONE)];
}

int freewb_combine_shortcut_index_from_token(const std::string &token)
{
    const auto &k = combineShortcutTokenCStrs();
    for (int i = 0; i < CSK_NUM; ++i)
    {
        if (token == k[static_cast<size_t>(i)])
            return i;
    }
    return CSK_NONE;
}

std::string freewb_combine_shortcut_token_from_index(int index)
{
    const auto &k = combineShortcutTokenCStrs();
    if (index < 0 || index >= CSK_NUM)
        return "KEY_NONE";
    return k[static_cast<size_t>(index)];
}

const std::string &freewb_combine_shortcut_display_name(int index)
{
    static const std::array<std::string, CSK_NUM> &labels = combineShortcutDisplayNames();
    static const std::string kEmpty;
    if (index < 0 || index >= CSK_NUM)
        return kEmpty;
    return labels[static_cast<size_t>(index)];
}

int freewb_combine_shortcut_index_from_custom_shortcut_value(const std::string &value)
{
    if (value.size() < 5 || value.compare(0, 4, "CTRL") != 0)
        return CSK_NONE;
    const char sep = value[4];
    if (sep != '+' && sep != '_')
        return CSK_NONE;
    const std::string keyToken = value.substr(5);
    const auto &k = combineShortcutTokenCStrs();
    for (int i = 0; i < CSK_NUM; ++i)
    {
        if (keyToken == k[static_cast<size_t>(i)])
            return i;
    }
    return CSK_NONE;
}

std::string freewb_custom_shortcut_value_from_combine_index(int index)
{
    const auto &k = combineShortcutTokenCStrs();
    const char *tok = (index >= 0 && index < CSK_NUM) ? k[static_cast<size_t>(index)] : "KEY_NONE";
    return std::string("CTRL+") + tok;
}

std::string freewb_custom_shortcut_display_name_from_combine_index(int index)
{
    if (index == CSK_NONE)
        return "无";
    return std::string("CTRL+") + std::string(freewb_combine_shortcut_display_name(index));
}

namespace
{
using CustomShortcutGetter = const std::string &(settings::Settings::*)() const;
using CustomShortcutSetter = void (settings::Settings::*)(const std::string &);
struct CustomShortcutAccessor
{
    const char *entryName;
    CustomShortcutGetter getter;
    CustomShortcutSetter setter;
};

const CustomShortcutAccessor *freewb_custom_shortcut_accessor(int funcIndex)
{
    static const CustomShortcutAccessor kAccessors[] = {
        {"backFindCode", &settings::Settings::get_backFindCode, &settings::Settings::set_backFindCode},
        {"onlineAddWord", &settings::Settings::get_onlineAddWord, &settings::Settings::set_onlineAddWord},
        {"onlineDelWord", &settings::Settings::get_onlineDelWord, &settings::Settings::set_onlineDelWord},
        {"switchVKb", &settings::Settings::get_switchVKb, &settings::Settings::set_switchVKb},
        {"switchCharSet", &settings::Settings::get_switchCharSet, &settings::Settings::set_switchCharSet},
        {"switchInputMode", &settings::Settings::get_switchInputMode, &settings::Settings::set_switchInputMode},
        {"switchChttrans", &settings::Settings::get_switchChttrans, &settings::Settings::set_switchChttrans},
        {"setupOption", &settings::Settings::get_setupOption, &settings::Settings::set_setupOption},
        {"showHideToolbar", &settings::Settings::get_showHideToolbar, &settings::Settings::set_showHideToolbar},
        {"showHideCandiWin", &settings::Settings::get_showHideCandiWin, &settings::Settings::set_showHideCandiWin},
        {"switchLexicon", &settings::Settings::get_switchLexicon, &settings::Settings::set_switchLexicon},
        {"switchSkin", &settings::Settings::get_switchSkin, &settings::Settings::set_switchSkin},
        {"quickDelScreenItem", &settings::Settings::get_quickDelScreenItem, &settings::Settings::set_quickDelScreenItem},
        {"markAutoPair", &settings::Settings::get_markAutoPair, &settings::Settings::set_markAutoPair}};
    if (funcIndex < 0 || funcIndex >= CSF_NUM)
        return nullptr;
    return &kAccessors[funcIndex];
}
} // namespace

int freewb_custom_shortcut_get_combine_index(const settings::Settings &cfg, int funcIndex)
{
    const CustomShortcutAccessor *accessor = freewb_custom_shortcut_accessor(funcIndex);
    if (!accessor || !accessor->getter)
        return CSK_NONE;
    return freewb_combine_shortcut_index_from_custom_shortcut_value((cfg.*(accessor->getter))());
}

std::string freewb_custom_shortcut_display_label(const settings::Settings &cfg, int funcIndex)
{
    const CustomShortcutAccessor *accessor = freewb_custom_shortcut_accessor(funcIndex);
    if (!accessor || !accessor->getter)
        return "无";
    const std::string value = (cfg.*(accessor->getter))();
    const int idx = freewb_combine_shortcut_index_from_custom_shortcut_value(value);
    if (idx == CSK_NONE)
        return "无";
    return std::string("CTRL+") + std::string(freewb_combine_shortcut_display_name(idx));
}

const std::string &freewb_custom_shortcut_key_display(const settings::Settings &cfg, int funcIndex)
{
    const CustomShortcutAccessor *accessor = freewb_custom_shortcut_accessor(funcIndex);
    if (!accessor || !accessor->getter)
        return freewb_combine_shortcut_display_name(CSK_NONE);
    const std::string value = (cfg.*(accessor->getter))();
    return freewb_combine_shortcut_display_name(freewb_combine_shortcut_index_from_custom_shortcut_value(value));
}

void freewb_custom_shortcut_set_combine_index(settings::Settings &cfg, int funcIndex, int combineIndex)
{
    const CustomShortcutAccessor *accessor = freewb_custom_shortcut_accessor(funcIndex);
    if (!accessor || !accessor->setter)
        return;
    const auto &k = combineShortcutTokenCStrs();
    const char *tok = (combineIndex >= 0 && combineIndex < CSK_NUM) ? k[static_cast<size_t>(combineIndex)] : "KEY_NONE";
    (cfg.*(accessor->setter))(std::string("CTRL+") + tok);
}

std::vector<CustomKeyStrings> freewb_build_custom_key_table(const std::string &chars, const std::string &marks, int keyCount)
{
    std::vector<CustomKeyStrings> table(static_cast<size_t>(keyCount));

    auto splitSpacesPreserveEmptyBetween = [](const std::string &value) {
        std::vector<std::string> out;
        std::string token;
        for (size_t i = 0; i < value.size(); ++i)
        {
            if (value[i] == ' ')
            {
                out.push_back(token);
                token.clear();
            }
            else
            {
                token.push_back(value[i]);
            }
        }
        out.push_back(token);
        return out;
    };

    const std::vector<std::string> charsList = splitSpacesPreserveEmptyBetween(chars);
    const std::vector<std::string> marksList = splitSpacesPreserveEmptyBetween(marks);
    for (int i = 0; i < keyCount; ++i)
    {
        const size_t base = static_cast<size_t>(i * 2);
        if (base + 1 < charsList.size())
        {
            table[static_cast<size_t>(i)].commChar = charsList[base];
            table[static_cast<size_t>(i)].shiftChar = charsList[base + 1];
        }
        if (base + 1 < marksList.size())
        {
            table[static_cast<size_t>(i)].commMark = marksList[base];
            table[static_cast<size_t>(i)].shiftMark = marksList[base + 1];
        }
    }
    return table;
}

std::string freewb_flatten_custom_char_value(const std::vector<CustomKeyStrings> &table)
{
    std::string value;
    for (size_t i = 0; i < table.size(); ++i)
    {
        value += table[i].commChar;
        value += ' ';
        value += table[i].shiftChar;
        value += ' ';
    }
    return value;
}

std::string freewb_flatten_custom_mark_value(const std::vector<CustomKeyStrings> &table)
{
    std::string value;
    for (size_t i = 0; i < table.size(); ++i)
    {
        value += table[i].commMark;
        value += ' ';
        value += table[i].shiftMark;
        value += ' ';
    }
    return value;
}

CustomKeyStrings freewb_custom_key_info_from_values(const std::string &chars, const std::string &marks, int keyIdx, int keyCount)
{
    if (keyIdx < 0 || keyIdx >= keyCount)
        return CustomKeyStrings{};
    const std::vector<CustomKeyStrings> table = freewb_build_custom_key_table(chars, marks, keyCount);
    return table[static_cast<size_t>(keyIdx)];
}

bool freewb_custom_key_info_apply_to_values(std::string &chars, std::string &marks, int keyIdx, int keyCount, const CustomKeyStrings &keyValue)
{
    if (keyIdx < 0 || keyIdx >= keyCount)
        return false;
    std::vector<CustomKeyStrings> table = freewb_build_custom_key_table(chars, marks, keyCount);
    table[static_cast<size_t>(keyIdx)] = keyValue;
    chars = freewb_flatten_custom_char_value(table);
    marks = freewb_flatten_custom_mark_value(table);
    return true;
}

