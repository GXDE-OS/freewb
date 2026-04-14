/****************************************************************************************
** 面板 UI 用辅助：面板专用枚举/CustomKeyStrings/CustomKeyValue、QString↔UTF-8、SettingsNotifier
** 以及 freewb_* 工具函数
***************************************************************************************/

#ifndef SETTINGSHELPER_H
#define SETTINGSHELPER_H

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <QFont>
#include <QObject>
#include <QString>

#include "types.h"

typedef enum
{
    CWDM_ONE_ROW,
    CWDM_MULTI_ROW,
} CandiWinDispMode;

typedef enum
{
    IM_OTHER = -1,
    IM_WUBI_FONT,
    IM_WUBI_PINYIN,
    IM_STD_PINYIN,
    IM_ENGLISH
} InputMode;

typedef enum
{
    CHAR_SIMPLIFIED,
    CHAR_TRADITIONAL
} CharFontMode;

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

struct CustomKeyStrings
{
    std::string commChar;
    std::string shiftChar;
    std::string commMark;
    std::string shiftMark;
};

typedef enum
{
    AWGO_FORBID,
    AWGO_LOSS,
    AWGO_SAVE
} AutoWordGroupOpt;

typedef enum
{
    RSSK_NONE,
    RSSK_SEMI_QUOTE,
    RSSK_COMMA_PERIOD,
    RSSK_CTRL,
    RSSK_SHIFT
} RcodeSelectShortcutKey;

typedef enum
{
    CPSK_SUB_EQUAL,
    CPSK_COMMA_PERIOD,
    CPSK_UP_DWON,
    CPSK_PAGE_UP_DWON
} CandiPageShortcutKey;

typedef enum
{
    CSF_BACK_FIND_CODE,    // 反查编码
    CSF_ONLINE_ADD_WORD,   // 在线加词
    CSF_ONLINE_DEL_WORD,   // 在线删词
    CSF_SWITCH_KEYBOARD,   // 切换软键盘
    CSF_SWITCH_CHAR_SET,   // 切换字符集
    CSF_SWITCH_INPUT_MODE, // 切换输入模式
    // CSF_SWITCH_WORD_STATE,    // 切换字词状态（历史项，当前 UI 屏蔽）
    CSF_SWITCH_S_IN_T_OUT,       // 切换简入繁出
    CSF_SETUP_OPTION,            // 打开系统设置
    CSF_SHOW_HIDE_STATUS_BAR,    // 显/隐状态栏
    CSF_SHOW_HIDE_CANDIDATE_WIN, // 显/隐候选窗
    CSF_SWITCH_WORD_LEXICON,     // 切换词库
    // CSF_ADD_CHAR_AFTER_OUTPUT,// 输出项后加字符（历史项，当前 UI 屏蔽）
    CSF_SWITCH_SKIN,           // 切换皮肤
    CSF_QUICK_DEL_SCREEN_CHAR, // 快删上屏项
    CSF_MARK_AUTO_PAIR,        // 标点自动配对

    CSF_NUM
} CustomShortcutFunction;

typedef enum
{
    CSK_NONE,
    CSK_INSERT,
    CSK_DEL,
    CSK_ESC,
    CSK_BACKSPACE,
    CSK_HOME,
    CSK_END,
    CSK_LEFT,
    CSK_RIGHT,
    CSK_UP,
    CSK_DOWN,
    CSK_QUOTE,
    CSK_SEMICOLON,
    CSK_BACK_SLASH,
    CSK_LEFT_BRACKETS,
    CSK_RIGHT_BRACKETS,
    CSK_COMMA,
    CSK_PERIOD,
    CSK_SLASH,
    CSK_BACK_QUOTE,
    CSK_EQUAL,
    CSK_DASH,
    CSK_F1,
    CSK_F2,
    CSK_F3,
    CSK_F4,
    CSK_F5,
    CSK_F6,
    CSK_F7,
    CSK_F8,
    CSK_F9,
    CSK_F10,
    CSK_F11,
    CSK_F12,
    CSK_a,
    CSK_b,
    CSK_c,
    CSK_d,
    CSK_e,
    CSK_f,
    CSK_g,
    CSK_h,
    CSK_i,
    CSK_j,
    CSK_k,
    CSK_l,
    CSK_m,
    CSK_n,
    CSK_o,
    CSK_p,
    CSK_q,
    CSK_r,
    CSK_s,
    CSK_t,
    CSK_u,
    CSK_v,
    CSK_w,
    CSK_x,
    CSK_y,
    CSK_z,

    CSK_NUM
} CombineShortcutKey;

typedef enum
{
    SSK_NONE,
    SSK_SEMICOLON,
    SSK_QUOTE,
    SSK_COMMA,
    SSK_PERIOD,
    SSK_BACK_QUOTE,
    SSK_LEFT_BRACKETS,
    SSK_RIGHT_BRACKETS,
    SSK_BACK_SLASH,
    SSK_SLASH,
    SSK_CHAR_u,
    SSK_CHAR_i,
    SSK_CHAR_v,
    SSK_CHAR_z,

    SSK_NUM
} SingleShortcutKey;

typedef enum
{
    CESSK_NONE,
    CESSK_LEFT_SHIFT,
    CESSK_RIGHT_SHIFT,
    CESSK_SHIFT,
    CESSK_LEFT_CTRL,
    CESSK_RIGHT_CTRL,
    CESSK_CTRL,

    CESSK_NUM,
} CnEnSwitchShortcutKey;

namespace settings
{
class Settings;
}

/** 运行时接口：面板会话内存态，不对应单一 ini 键；皮肤/词库列表为界面侧缓存。 */
bool freewb_runtime_use_audio_file();
void freewb_runtime_set_use_audio_file(bool enabled);
bool freewb_runtime_toggle_show_all_group();
bool freewb_runtime_show_all_group();
const std::vector<std::string> &freewb_runtime_skin_list();
void freewb_runtime_set_skin_list(const std::vector<std::string> &skinList);
const std::vector<std::string> &freewb_runtime_lexicon_list();
void freewb_runtime_set_lexicon_list(const std::vector<std::string> &lexiconList);

/** 单字符配置：ini 中单字符项（如句读分隔符）与 std::string 互转。 */
char freewb_separate_char_from_string(const std::string &value);
std::string freewb_separate_char_to_string(char ch);

/** ini 词条 / 存储串 ↔ 枚举索引：候选翻页、中英切换修饰键、选择码单键、组合快捷键第二键；含下拉展示名与 ini 字面量。 */
int freewb_candi_page_index_from_token(const std::string &token);
std::string freewb_candi_page_token_from_index(int index);
/** 按翻页方案写入 prevPageKey/nextPageKey（candiPageKey 为方案标记）。 */
void freewb_apply_candidate_page_hotkeys(settings::Settings &cfg, CandiPageShortcutKey scheme);
int freewb_cn_en_switch_index_from_token(const std::string &token);
std::string freewb_cn_en_switch_token_from_index(int index);
int freewb_single_shortcut_index_from_token(const std::string &token);
const std::string &freewb_single_shortcut_display_name(int index);
const std::string &freewb_single_shortcut_ini_token(int index);
const std::string &freewb_single_shortcut_ini_from_stored(const std::string &stored);
const std::string &freewb_single_shortcut_display_from_stored(const std::string &stored);
int freewb_combine_shortcut_index_from_token(const std::string &token);
std::string freewb_combine_shortcut_token_from_index(int index);
const std::string &freewb_combine_shortcut_display_name(int index);

/** 自定义功能快捷键：存储串与 CombineShortcutKey 索引互转（Ctrl/Shift + 第二键 的拼法） */
int freewb_combine_shortcut_index_from_custom_shortcut_value(const std::string &value);
std::string freewb_custom_shortcut_value_from_combine_index(int index);
std::string freewb_custom_shortcut_display_name_from_combine_index(int index);

/** 自定义功能快捷键：按 funcIndex 读写 Settings 中多键组合（修饰键 + 第二键等），生成展示标签与落盘串 */
int freewb_custom_shortcut_get_combine_index(const settings::Settings &cfg, int funcIndex);
std::string freewb_custom_shortcut_display_label(const settings::Settings &cfg, int funcIndex);
const std::string &freewb_custom_shortcut_key_display(const settings::Settings &cfg, int funcIndex);
void freewb_custom_shortcut_set_combine_index(settings::Settings &cfg, int funcIndex, int combineIndex);

/** 二三重码组合选择与 second/third 键名同步 */
void freewb_apply_recode_select_pair(settings::Settings &cfg, RcodeSelectShortcutKey key);

/** 软键盘自定义：整段 chars / marks 与逐键 CustomKeyStrings 表互转（拆分、合并写回） */
std::vector<CustomKeyStrings> freewb_build_custom_key_table(const std::string &chars, const std::string &marks, int keyCount);
std::string freewb_flatten_custom_char_value(const std::vector<CustomKeyStrings> &table);
std::string freewb_flatten_custom_mark_value(const std::vector<CustomKeyStrings> &table);
CustomKeyStrings freewb_custom_key_info_from_values(const std::string &chars, const std::string &marks, int keyIdx, int keyCount);
bool freewb_custom_key_info_apply_to_values(std::string &chars, std::string &marks, int keyIdx, int keyCount, const CustomKeyStrings &keyValue);

struct CustomKeyValue
{
    QString commChar;
    QString shiftChar;
    QString commMark;
    QString shiftMark;
};

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

QString toQStringUtf8(const std::string &s);
std::string fromStdUtf8(const QString &q);
CustomKeyValue customKeyToQt(const CustomKeyStrings &s);
CustomKeyStrings customKeyFromQt(const CustomKeyValue &v);

/** 候选文字体：由 candiTextFontName + candiTextFontSize 构造（持久化字段，非 QFont::toString）。 */
QFont freewb_candi_text_qfont(const settings::Settings &cfg);
QString freewb_candi_text_font_display_label(const settings::Settings &cfg);
void freewb_candi_text_font_apply_qfont(settings::Settings &cfg, const QFont &font);

#endif
