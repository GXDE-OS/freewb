#include "settingshelper.h"

#include <array>
#include <vector>

#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

#include "key.h"
#include "settings.h"

namespace
{
struct UiRuntimeState
{
    std::vector<std::string> skinList;
    std::vector<std::string> lexiconList;
};

UiRuntimeState g_uiRuntimeState;

} // namespace

const std::array<CnEnSwitchPreset, 3> &freewb_cn_en_switch_presets()
{
    static const std::array<CnEnSwitchPreset, 3> k = {{
        {"KEY_NONE", ""},
        {"KEY_SHIFT", "Shift"},
        {"KEY_CTRL", "Ctrl"},
    }};
    return k;
}

int freewb_cn_en_switch_preset_index(const std::string &token)
{
    const auto &presets = freewb_cn_en_switch_presets();
    for (size_t i = 0; i < presets.size(); ++i)
    {
        if (token == presets[i].token)
            return static_cast<int>(i);
    }
    return -1;
}

SettingsNotifier::SettingsNotifier(QObject *parent) : QObject(parent)
{
}

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

void freewb_candi_text_font_apply_qfont(settings::Settings &cfg, const QFont &font)
{
    int psz = font.pointSize();
    if (psz <= 0)
        psz = font.pixelSize() > 0 ? font.pixelSize() : 14;
    cfg.set_candiTextFontName(fromStdUtf8(font.family()));
    cfg.set_candiTextFontSize(psz);
}

QIcon freewb_icon_from_skin_path(const QString &path, const QSize &logicalSize, qreal devicePixelRatio)
{
    if (!path.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive))
    {
        return QIcon(path);
    }

    const qreal dpr = qMax(1.0, devicePixelRatio);
    const int pixmapW = qMax(1, qRound(logicalSize.width() * dpr));
    const int pixmapH = qMax(1, qRound(logicalSize.height() * dpr));
    QPixmap pixmap(pixmapW, pixmapH);
    pixmap.fill(Qt::transparent);
    QSvgRenderer renderer(path);
    if (renderer.isValid())
    {
        QPainter painter(&pixmap);
        renderer.render(&painter, QRectF(0, 0, pixmapW, pixmapH));
    }
    pixmap.setDevicePixelRatio(dpr);
    return QIcon(pixmap);
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

std::string freewb_custom_shortcut_format(const std::string &value)
{
    const char *keyTok = freewb::Key::readKeyString(value.c_str());
    if (!keyTok)
        return {};
    const std::string tok = keyTok;
    if (tok == "KEY_NONE")
        return {};
    const FreewbKeySym sym = freewb::Key::keySymFromUniqueName(tok.c_str());
    if (sym == FreewbKey_None)
        return {};
    return std::string("Ctrl+") + freewb::Key::keySymToName(sym);
}

std::vector<CustomKeyStrings> freewb_build_custom_key_table(const std::string &chars, const std::string &marks, int keyCount)
{
    std::vector<CustomKeyStrings> table(static_cast<size_t>(keyCount));

    auto splitSpacesPreserveEmptyBetween = [](const std::string &value)
    {
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

bool freewb_custom_key_info_apply_to_values(std::string &chars, std::string &marks, int keyIdx, int keyCount,
                                            const CustomKeyStrings &keyValue)
{
    if (keyIdx < 0 || keyIdx >= keyCount)
        return false;
    std::vector<CustomKeyStrings> table = freewb_build_custom_key_table(chars, marks, keyCount);
    table[static_cast<size_t>(keyIdx)] = keyValue;
    chars = freewb_flatten_custom_char_value(table);
    marks = freewb_flatten_custom_mark_value(table);
    return true;
}
