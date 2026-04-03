/****************************************************************************************
** @作者：lcj
**
** @说明：
**      Settings是一个从QObject类继承而来的非UI自定义容器类，其所有数据成员与成员函数皆为静态，该类设
**      计目的为让UI与用户配置数据分离，所有与配置数据相关的UI都必须通过该类的提供的静态函数借口去读写配
**      置数据。
*************************************************************************************×**/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDebug>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QVector>

#include "inputwin.h"
#include "keyboard.h"
#include "toolbarwin.h"

class Settings;
extern Settings g_settings;

// 常用选项
typedef struct
{
    bool codeRemind;            // 启用编码逐渐提示
    bool spaceFullWhenCharHalf; // 字符半角时空格全角
    bool wordThink;             // 启用词组联想功能
    bool smartMark;             // 启用智能标点功能
    bool remindExistWord;       // 提示系统已有词组
    bool alertWhenEmptyCode;    // 重码或空码时发声提示
    bool useAudioFile;          // 使用声音文件
    bool autoAdjustFreq;        // 启动自动调频功能
} SettingCommon;

// 自动词组选项
typedef enum
{
    AWGO_FORBID, // 禁止自动造词
    AWGO_LOSS,   // 关闭极点时消失
    AWGO_SAVE    // 随时保存在词库
} AutoWordGroupOpt;

// 高级选项
typedef struct
{
    bool shiftCommitChar;              // Shift+字母直接上屏
    bool inputStatistic;               // 启用输入统计功能
    bool typeEffect;                   // 启用打字音效功能
    bool recodeCalib;                  // 重码上屏校对模式
    AutoWordGroupOpt autoWordGroupOpt; // 自动词组选项
} SettingAdvance;

// 其它设置
typedef struct
{
    QString autoToEnStr;    // 自动转英文的字符串
    QString autoToHalfMark; // 数字后自动转半角的标点
    bool autoToHalfMarkFlg;
} SettingOthers;

// 界面设置
typedef struct
{
    QString curSkinId;        // 当前使用皮肤
    bool toolbarAutoLocate;   // 状态栏自动定位
    bool toolbarAutoExpand;   // 状态栏菜单自动伸缩
    bool enableUiAudioEffect; // 使用界面音效
    bool showRealtimeHelp;    // 显示实时帮助
    bool hideToolbar;         // 隐藏状态栏
    int toolbarTransparency;  // 状态栏透明度
} SettingUi;

// 候选窗口界面设置
typedef struct
{
    CandiWinDispMode candiWinDispMode; // 候选词显示模式
    char separateChar;                 // 候选词序号与词组间的分隔符
    bool useGradientColor;             // 是否使用渐变色
    bool useBgImage;                   // 是否使用背景图
    bool enablebgImageTiled;           // 使能背景图平铺
    bool show_cand_dict;               // 是否显示候选项字典
    int radius;                        // 圆角弧度
    int candiWinTransparency;          // 候选框透明度
    int candiWordCount;                // 候选词个数
    int candiCharCount;                // 候选词字数
    QFont candiTextFont;               // 候选框字体
    QColor bgColor;                    // 背景色
    QColor gradientColor0;             // 渐变起始色
    QColor gradientColor1;             // 渐变结束色
    QColor borderColor;                // 边框颜色
    QString bgImage;                   // 背景图片
    QColor candiWordTextColor;         // 候选词颜色
    QColor candiPromptTextColor;       // 提示词颜色
} SettingCandidateWinUi;

// 二三重码选择键
typedef enum
{
    RSSK_NONE,         // 无
    RSSK_SEMI_QUOTE,   // 分号，单引号
    RSSK_COMMA_PERIOD, // 逗号句号
    RSSK_CTRL,         // 左右CTRL
    RSSK_SHIFT         // 左右SHIFT
} RcodeSelectShortcutKey;

// 上下翻页键
typedef enum
{
    CPSK_SUB_EQUAL,    // 减号等号
    CPSK_COMMA_PERIOD, // 逗号句号
    CPSK_UP_DWON,      // 上下箭头
    CPSK_PAGE_UP_DWON  // PageUp,PageDown
} CandiPageShortcutKey;

// 候选窗口选项设置
typedef struct
{
    RcodeSelectShortcutKey recodeSelectKey; // 重码选择按键组
    QString secondRecodeKey;                // 第二重码选择按键
    QString thirdRecodeKey;                 // 第三重码选择按键
    CandiPageShortcutKey candiPageKey;      // 上翻页按键组
    QString prevPageKey;                    // 上翻页按键
    QString nextPageKey;                    // 下翻页按键
    bool cursorFollowFlg;                   // 候选差窗跟随
    bool hideCandiWinFlg;                   // 隐藏候选窗
    bool showOpRemindInfoFlg;               // 显示操作提示信息
    bool shiftSelectRecodeFlg;              // 用shift按键选择重码
} SettingCandidateWinOption;

// 自定义CTRL组合快捷功能
typedef enum
{
    CSF_BACK_FIND_CODE,          // 反查编码
    CSF_ONLINE_ADD_WORD,         // 在线加词
    CSF_ONLINE_DEL_WORD,         // 在线删词
    CSF_SWITCH_KEYBOARD,         // 切换软键盘
    CSF_SWITCH_CHAR_SET,         // 切换字符集
    CSF_SWITCH_INPUT_MODE,       // 切换输入模式
    CSF_SWITCH_WORD_STATE,       // 切换字词状态
    CSF_SWITCH_S_IN_T_OUT,       // 切换简入繁出
    CSF_SETUP_OPTION,            // 系统设置
    CSF_SHOW_HIDE_STATUS_BAR,    // 显示/隐藏状态栏
    CSF_SHOW_HIDE_CANDIDATE_WIN, // 显示/隐藏候选窗
    CSF_SWITCH_WORD_LEXICON,     // 切换词库
    CSF_ADD_CHAR_AFTER_OUTPUT,   // 输出项后加字符
    CSF_SWITCH_SKIN,             // 切换皮肤
    CSF_QUICK_DEL_SCREEN_CHAR,   // 快删上屏项
    CSF_MARK_AUTO_PAIR,          // 标点自动配对

    CSF_NUM
} CustomShortcutFunction;

// 组合快捷键
typedef enum
{
    CSK_NONE, // 禁用
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
    CSK_QUOTE,          // 引号
    CSK_SEMICOLON,      // 分号
    CSK_BACK_SLASH,     // 反斜杠
    CSK_LEFT_BRACKETS,  // 左中括号
    CSK_RIGHT_BRACKETS, // 右中括号
    CSK_COMMA,          // 逗号
    CSK_PERIOD,         // 句号
    CSK_SLASH,          // 除号
    CSK_BACK_QUOTE,     // 反引号
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

// 单按键快捷键
typedef enum
{
    SSK_NONE,
    SSK_SEMICOLON,      // 分号
    SSK_QUOTE,          // 引号
    SSK_COMMA,          // 逗号
    SSK_PERIOD,         // 句号
    SSK_BACK_QUOTE,     // 反引号
    SSK_LEFT_BRACKETS,  // 左中括号
    SSK_RIGHT_BRACKETS, // 右中括号
    SSK_BACK_SLASH,     // 反斜杠
    SSK_SLASH,          // 除号
    SSK_CHAR_u,         //
    SSK_CHAR_i,         //
    SSK_CHAR_v,         //
    SSK_CHAR_z,         //

    SSK_NUM
} SingleShortcutKey;

// 中英文切换快捷键
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

// 快捷键配置
typedef struct
{
    CombineShortcutKey customShortcutFunc[CSF_NUM]; // 自定义功能快捷键
    SingleShortcutKey sskTmpEnglish;                // 临时英文快捷键
    SingleShortcutKey sskShortcutInput;             // 便捷输入快捷键
    SingleShortcutKey sskTmpPinyin;                 // 临时拼音快捷键
    CnEnSwitchShortcutKey ceSwitchShortcutkey;      // 中英文切换快捷键
    bool disableFullHalfSwitchShortcutkey;          // 禁用全半角切换键
    bool disableAllCsk;                             // 禁用所有自定义快捷键
} SettingShortcutKey;

// 自定义软键盘字符与标点
typedef struct
{
    CustomKeyValue customKeyValue[KEY_SYMBOL_NUM];
} SettingCustomKey;

// 杂项设置(只在专家设置模式可见)
typedef struct
{
    bool enterClear; // enter键是否清楚当前预编辑框内容
    int inputMethod;
    int currentCharset;
    int simpTranFlg;
    bool useFreewbFont; // 用极点自带字体
} SettingMisc;

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = nullptr);

signals:
    void signal_setting_data_changed_to_local();
    void signal_setting_data_changed_to_fcitx();

public slots:
    static void slot_restore_default_all_config();

public:
    static bool useFreewbFont();
    static void init_const_data_member();

    static void load_all_setting_data_from_file();
    static void save_all_setting_data_to_file();
    static void copy_all_setting_data_to_buffer();
    static void copy_all_buffer_to_setting_data();

    static const QStringList &get_exist_skin();
    static void save_exist_skin(const QStringList &skinList);
    static void save_cur_skin_id_to_file();
    static void save_userWord_change_flg_to_file(int flg);
    static void save_quickTable_change_flg_to_file(int flg);
    static void save_inputmethod_flg_to_file(InputMode im);
    static int get_input_method();

    static void save_char_trad_flg_to_file(CharFontMode mode);
    static void save_charSet_flg_to_file(int flg);
    static void save_hide_toolbar_flg_to_file();
    static void save_hide_candiwin_flg_to_file();
    static void save_smart_mark_flg_to_file();
    static void save_recode_calib_flg_to_file();
    static void save_ime_table_changed_flg_to_file(int flg);
    static void save_vk_mode_flg_to_file(int flg);

    static const QStringList &get_exist_lexicon();
    static void save_exist_lexicon(const QStringList &lexiconList);
    static const QString &get_cur_used_lexicon();
    static void save_cur_used_lexicon_to_file(const QString &lexiconName);

    /********************************* 设置界面 ***********************************/
    static bool switch_group_mode();
    static bool is_show_all_group();

    // 常用选项设置
    static void set_codeRemind_flg(bool enabled);
    static bool get_codeRemind_flg();
    static void set_spaceFullWhenCharHalf_flg(bool enabled);
    static bool get_spaceFullWhenCharHalf_flg();
    static void set_wordThink_flg(bool enabled);
    static bool get_wordThink_flg();
    static void set_smartMark_flg(bool enabled);
    static bool get_smartMark_flg();
    static void set_remindExistWord_flg(bool enabled);
    static bool get_remindExistWord_flg();
    static void set_alertWhenEmptyCode_flg(bool enabled);
    static bool get_alertWhenEmptyCode_flg();
    static void set_useAudioFile_flg(bool enabled);
    static bool get_useAudioFile_flg();
    static void set_autoAdjustFreq_flg(bool enabled);
    static bool get_autoAdjustFreq_flg();

    // 高级选项配置
    static void set_shiftCommitChar_flg(bool enabled); // Shift+字母直接上屏
    static bool get_shiftCommitChar_flg();
    static void set_inputStatistic_flg(bool enabled); // 启用输入统计功能
    static bool get_inputStatistic_flg();
    static void set_typeEffect_flg(bool enabled); // 启用打字音效功能
    static bool get_typeEffect_flg();
    static void set_recodeCalib_flg(bool enabled); // 重码上屏校对模式
    static bool get_recodetCalib_flg();
    static void set_autoWordGroupOpt_flg(AutoWordGroupOpt opt); // 自动词组选项
    static AutoWordGroupOpt get_autoWordGroupOpt_flg();

    // 其它设置选项
    static void set_autoToEn_str(const QString &str); // 自动转英文的字符串
    static const QString &get_autoToEn_str();
    static void set_autoToHalf_mark(const QString &str); // 数字后自动转半角的标点
    static const QString &get_autoToHalf_mark();
    static void set_autoToHalfMark_flg(bool flg);
    static bool get_autoToHalfMark_flg();
    static int get_charset();

    // 工具条与输入面板界面设置
    static void set_cur_skin_id(const QString &skinId);
    static const QString &get_cur_skin_id();
    static bool get_toolbar_auto_locate_flg();
    static void set_toolbar_auto_locate_flg(bool enabled);
    static bool get_toolbar_auto_expand_flg();
    static void set_toolbar_auto_expand_flg(bool enabled);
    static bool get_ui_audio_effect_flg();
    static void set_ui_audio_effect_flg(bool enabled);
    static bool get_show_realtime_help_flg();
    static void set_show_realtime_help_flg(bool enabled);
    static bool get_hide_toolbar_flg();
    static void set_hide_toolbar_flg(bool enabled);
    static int get_toolbar_transparency();
    static void set_toolbar_transparency(int trans);

    // 候选窗界面设置
    static CandiWinDispMode get_candidate_win_disp_mode();
    static void set_candidate_win_disp_mode(CandiWinDispMode candiWinDispMode);
    static char get_separate_char();
    static void set_separate_char(char ch);
    static bool get_use_gradient_color_flg();
    static void set_use_gradient_color_flg(bool enabled);
    static bool get_use_bg_image_flg();
    static void set_use_bg_image_flg(bool enabled);
    static int get_radius();
    static void set_radius(int radius);
    static int get_candi_win_transparency();
    static void set_candi_win_transparency(int trans);
    static void set_candidate_word_count(int candiWordCount);
    static int get_candidate_word_count();
    static int get_candi_char_count();
    static void set_candi_char_count(int count);
    static const QFont &get_candidate_text_font();
    static void set_candidate_text_font(const QFont &font);
    static QColor get_bg_color();
    static void set_bg_color(const QColor &color);
    static const QColor &get_gradient_color0();
    static void set_gradient_color0(const QColor &color);
    static const QColor &get_gradient_color1();
    static void set_gradient_color1(const QColor &color);
    static QColor get_border_color();
    static void set_border_color(const QColor &color);
    static const QString &get_bg_image();
    static void set_bg_image(const QString &imagePath);
    static bool get_bg_image_tiled_flg();
    static void set_bg_image_tiled_flg(bool enabled);
    static const QColor &get_candidate_word_text_color();
    static void set_candidate_word_text_color(const QColor &color);
    static const QColor &get_candidate_prompt_text_color();
    static void set_candidate_prompt_text_color(const QColor &color);
    static bool get_show_cand_dict();
    static void set_show_cand_dict(bool chk);

    // 候选窗选项设置
    static RcodeSelectShortcutKey get_recode_select_key();
    static void set_recode_select_key(RcodeSelectShortcutKey key);
    //    static char get_second_recode_key();
    //    static void set_second_recode_key( char key );
    //    static char get_third_recode_key();
    //    static void set_third_recode_key( char key );
    static CandiPageShortcutKey get_candi_page_key();
    static void set_candi_page_key(CandiPageShortcutKey key);
    //    static char get_prevPage_key();
    //    static void set_prevPage_key( char key );
    //    static char get_nextPage_key();
    //    static void set_nextPage_key( char key );
    static bool get_cursor_follow_flg();
    static void set_candiWin_follow_flg(bool enabled);
    static bool get_hide_candiWin_flg();
    static void set_hide_candiWin_flg(bool enabled);
    static bool get_show_op_remind_info_flg();
    static void set_show_op_remind_info_flg(bool enabled);
    static bool get_shift_select_recode_flg();
    static void set_shift_select_recode_flg(bool enabled);

    // 快捷键设置
    static void restore_default_shortcutkey();
    static void set_customShortcutFunc_shortcutkey(CustomShortcutFunction func, CombineShortcutKey key);
    static CombineShortcutKey get_customShortcutFunc_shortcutkey(CustomShortcutFunction func);
    static void set_tmp_english_shortcutkey(const QString &keyName);
    static SingleShortcutKey get_tmp_english_shortcutkey();
    static void set_shortcut_input_shortcutkey(const QString &keyName);
    static SingleShortcutKey get_quick_input_shortcutkey();
    static void set_tmp_pinyin_shortcutkey(const QString &keyName);
    static SingleShortcutKey get_tmp_pinyin_shortcutkey();
    static void set_ceSwitchShortcutkey_shortcutkey(CnEnSwitchShortcutKey key);
    static CnEnSwitchShortcutKey get_ceSwitchShortcutkey_shortcutkey();
    static void set_disableFullHalfSwitchShortcutkey_flg(bool enabled);
    static bool get_disableFullHalfSwitchShortcutkey_flg();
    static void set_disableAllCsk_flg(bool enabled);
    static bool get_disableAllCsk_flg();

    // 自定义键盘设置
    static void restore_default_customkey();
    static QString get_custom_char_value();
    static void set_custom_char_value(const QString &value);
    static QString get_custom_mark_value();
    static void set_custom_mark_value(const QString &value);
    static const CustomKeyValue &get_custom_key_info_(SymbolKeyIdx keyIdx);
    static const CustomKeyValue &get_custom_key_info(SymbolKeyIdx keyIdx);
    static void set_custom_key_info(SymbolKeyIdx keyIdx, const CustomKeyValue &keyValue);

public:
    // 显示在设置界面的自定义单快捷键名称
    static QMap<SingleShortcutKey, QString> s_singleShortcutKeyName;
    // 显示在设置界面的自定义功能组合快捷键名称
    static QMap<CombineShortcutKey, QString> s_combineShortcutKeyName;

    // 写入配置文件中的快捷键名称
    static QMap<SingleShortcutKey, QString> s_singleShortcutKeyName_1;
    static QMap<CombineShortcutKey, QString> s_combineShortcutKeyName_1;
    static QMap<CnEnSwitchShortcutKey, QString> s_cnEnSwitchShortcutKeyName;

    // 将配置文件中的快捷键名称转化为对应的快捷键索引
    static SingleShortcutKey convert_name_to_singleShortcutKey(const QString name);
    static CombineShortcutKey convert_name_to_combineShortcutKey(const QString name);
    static CnEnSwitchShortcutKey convert_name_to_cnEnSwitchShortcutKey(const QString name);

    static QString get_singleShortcutKey_name(SingleShortcutKey key);
    static QString get_singleShortcutKey_name_1(SingleShortcutKey key);
    static QString get_customShortcutFunc_shortcutkey_name(CustomShortcutFunction func);

private:
    static bool s_isShowAllGroup, s_isShowAllGroupBuf; // 正式配置变量，临时配置变量
    static SettingCommon s_common, s_commonBuf;
    static SettingAdvance s_advance, s_advanceBuf;
    static SettingOthers s_others, s_othersBuf;
    static SettingUi s_ui, s_uiBuf;
    static SettingCandidateWinUi s_candidateWinUi, s_candidateWinUiBuf;
    static SettingCandidateWinOption s_candidateWinOption, s_candidateWinOptionBuf;
    static SettingShortcutKey s_shortcutKey, s_shortcutKeyBuf;
    static SettingCustomKey s_customKey, s_customKeyBuf;
    static SettingMisc s_miscSetting;

    static bool s_settingDataIsChanged; // 配置数据是否发生改变
    static bool s_needNotifyFcitx;      // 是否需要通知fcitx配置文件发生改变

    static QStringList s_lexiconList;
    static QString s_curUsedLexicon;
    static QStringList s_skinList;
};

#endif
