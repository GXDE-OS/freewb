/****************************************************************************************
** 面板 UI 跨窗口辅助层：集中存放真正跨 TU 复用的类型与工具。
**
**   - 软键盘相关：SymbolKeyIdx / CtrlKeyIdx / CustomKeyStrings / CustomKeyValue
**                （keyboard + settingwin 共用）
**   - 中英切换键预设：CnEnSwitchPreset（inputwin 展示 displayName，settingwin 下拉反查 token）
**   - SettingsNotifier：本地/Fcitx 配置变更信号转发
**   - 运行时态：皮肤/词库列表（mainprogram 载入，contextmenu / settingwin 消费）
**   - QString ↔ UTF-8 / char ↔ std::string 等基础互转
**   - 自定义功能键 "CTRL+<KEY_X>" 的展示格式化（toolbarwin / inputwin 操作提示共用）
**
** 窗口专属枚举（CandiWinDispMode / InputMode / CharFontMode / AutoWordGroupOpt）已拆回
** 各自窗口 .h；仅 settingwin 使用的 accessor 表、候选列表、预设表（RecodeSelectPreset /
** CandiPagePreset）、会话开关（useAudioFile / showAllGroup）已下沉到 settingwin.cpp 匿名 ns。
***************************************************************************************/

#ifndef SETTINGSHELPER_H
#define SETTINGSHELPER_H

#include <array>
#include <string>
#include <vector>

#include <QFont>
#include <QIcon>
#include <QObject>
#include <QSize>
#include <QString>

// ─── 软键盘按键索引（keyboard + settingwin 共用）─────────────────────────────────────
// 软键盘符号键（0 ~ KEY_SYMBOL_NUM-1 为可自定义符号键）
typedef enum
{
    KEY_0 = 0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    KEY_BACKQUOTE,
    KEY_SUB,
    KEY_EQUAL,
    KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET,
    KEY_BACKSLASH,
    KEY_SEMICOLON,
    KEY_QUOTE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,

    KEY_SYMBOL_NUM
} SymbolKeyIdx;

// 软键盘控制键（接在 SymbolKeyIdx 之后）
typedef enum
{
    KEY_BACKSAPCE = KEY_SYMBOL_NUM,
    KEY_TAB,
    KEY_CAPS,
    KEY_ENTER,
    KEY_SHIFT,
    KEY_INSERT,
    KEY_DEL,
    KEY_SPACE,
    KEY_ESC,

    KEY_ALL_NUM
} CtrlKeyIdx;

// ─── 软键盘自定义字符/标点（keyboard + settingwin 共用）──────────────────────────────
// 单键自定义字符/标点（INI 存 UTF-8，std::string）
struct CustomKeyStrings
{
    std::string commChar;
    std::string shiftChar;
    std::string commMark;
    std::string shiftMark;
};

// UI 侧呈现用的 QString 形态
struct CustomKeyValue
{
    QString commChar;
    QString shiftChar;
    QString commMark;
    QString shiftMark;
};

// ─── 中英切换键预设（inputwin + settingwin 共用）─────────────────────────────────────
// 下标对齐 settingwin.ui 中 cmbChEnSwitchKey 顺序；settingwin 下拉反查，inputwin 展示 displayName
struct CnEnSwitchPreset
{
    const char *token;
    const char *displayName;
};

namespace settings
{
class Settings;
}

/** 皮肤/词库列表：mainprogram 载入后写入，contextmenu / settingwin 按需读取。 */
const std::vector<std::string> &freewb_runtime_skin_list();
void freewb_runtime_set_skin_list(const std::vector<std::string> &skinList);
const std::vector<std::string> &freewb_runtime_lexicon_list();
void freewb_runtime_set_lexicon_list(const std::vector<std::string> &lexiconList);

// ═════════════════════════════════════════════════════════════════════════════════════
// 字符串 / 配置值互转
// ═════════════════════════════════════════════════════════════════════════════════════

QString toQStringUtf8(const std::string &s);
std::string fromStdUtf8(const QString &q);
CustomKeyValue customKeyToQt(const CustomKeyStrings &s);
CustomKeyStrings customKeyFromQt(const CustomKeyValue &v);

/** 单字符 INI 项（如句读分隔符）与 std::string 互转。 */
char freewb_separate_char_from_string(const std::string &value);
std::string freewb_separate_char_to_string(char ch);

// ═════════════════════════════════════════════════════════════════════════════════════
// 中英切换键预设查询
// ═════════════════════════════════════════════════════════════════════════════════════

const std::array<CnEnSwitchPreset, 7> &freewb_cn_en_switch_presets();

/** token → 预设下标反查（未命中返回 -1），用于 init 选中 / 展示 displayName。 */
int freewb_cn_en_switch_preset_index(const std::string &token);

// ═════════════════════════════════════════════════════════════════════════════════════
// 自定义功能键格式化（Ctrl+KEY_X）
// accessor 表、候选列表、下拉/展示函数已就近放在 settingwin.cpp 匿名 namespace
// ═════════════════════════════════════════════════════════════════════════════════════

/** "CTRL+<KEY_X>" → "Ctrl+<name>"；KEY_NONE / 解析失败返回空串。 */
std::string freewb_custom_shortcut_format(const std::string &value);

// ═════════════════════════════════════════════════════════════════════════════════════
// 软键盘自定义字符/标点：INI 空格分隔扁平串 ↔ 逐键 CustomKeyStrings 表
// ═════════════════════════════════════════════════════════════════════════════════════

std::vector<CustomKeyStrings> freewb_build_custom_key_table(const std::string &chars, const std::string &marks, int keyCount);
std::string freewb_flatten_custom_char_value(const std::vector<CustomKeyStrings> &table);
std::string freewb_flatten_custom_mark_value(const std::vector<CustomKeyStrings> &table);
CustomKeyStrings freewb_custom_key_info_from_values(const std::string &chars, const std::string &marks, int keyIdx, int keyCount);
bool freewb_custom_key_info_apply_to_values(std::string &chars, std::string &marks, int keyIdx, int keyCount,
                                            const CustomKeyStrings &keyValue);

// ═════════════════════════════════════════════════════════════════════════════════════
// 候选文字体：由 candiTextFontName + candiTextFontSize 构造（持久化字段，非 QFont::toString）
// ═════════════════════════════════════════════════════════════════════════════════════

QFont freewb_candi_text_qfont(const settings::Settings &cfg);
void freewb_candi_text_font_apply_qfont(settings::Settings &cfg, const QFont &font);

/** 皮肤目录下图标路径 → QIcon：.svg 按逻辑尺寸栅格化，其余格式使用 QIcon(path)。 */
QIcon freewb_icon_from_skin_path(const QString &path, const QSize &logicalSize, qreal devicePixelRatio);

// ═════════════════════════════════════════════════════════════════════════════════════
// 配置变更通知：在面板各窗口 / 引擎侧之间转发 signal
// ═════════════════════════════════════════════════════════════════════════════════════

class SettingsNotifier : public QObject
{
    Q_OBJECT

public:
    explicit SettingsNotifier(QObject *parent = nullptr);

    /** 通知面板内窗口（候选窗、工具栏、软键盘等）从 settings 重新载入。 */
    void notifySettingDataChangedToLocal();
    /** 通知输入法框架引擎侧重载配置。 */
    void notifySettingDataChangedToFcitx();

signals:
    void signal_setting_data_changed_to_local();
    void signal_setting_data_changed_to_fcitx();
};

extern SettingsNotifier g_settingsNotifier;

#endif
