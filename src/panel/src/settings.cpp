#include "settings.h"

#include <QByteArray>
#include <QTemporaryFile>

#include "commdefine.h"
#include "utils/SimpleIni.h"

#define CONFIG_FILE INSTALL_DIR + "/config/config.ini"

namespace
{

QByteArray configIniPathUtf8()
{
    return (CONFIG_FILE).toUtf8();
}

bool csIniLoadOrEmpty(CSimpleIniA &ini, const char *path)
{
    SI_Error e = ini.LoadFile(path);
    return e >= 0 || e == SI_FILE;
}

QColor csIniGetColor(const CSimpleIniA &ini, const char *sec, const char *key, const QColor &def)
{
    const char *v = ini.GetValue(sec, key, nullptr);
    if (!v || !*v)
        return def;
    QByteArray raw(v);
    QString s = QString::fromUtf8(raw);
    if (s.startsWith(QLatin1Char('#')))
    {
        QColor c(s);
        if (c.isValid())
            return c;
    }
    if (s.contains(QLatin1Char(',')))
    {
        QStringList p = s.split(QLatin1Char(','));
        if (p.size() >= 3)
        {
            QColor c(p[0].trimmed().toInt(), p[1].trimmed().toInt(), p[2].trimmed().toInt(), p.size() > 3 ? p[3].trimmed().toInt() : 255);
            if (c.isValid())
                return c;
        }
    }
    if (s.startsWith(QLatin1String("@Variant")))
    {
        QTemporaryFile tf;
        if (tf.open())
        {
            tf.write("[tmp]\nx=");
            tf.write(raw);
            tf.write("\n");
            tf.flush();
            QSettings st(tf.fileName(), QSettings::IniFormat);
            st.setIniCodec("UTF-8");
            st.beginGroup(QStringLiteral("tmp"));
            QColor c = st.value(QStringLiteral("x")).value<QColor>();
            st.endGroup();
            if (c.isValid())
                return c;
        }
    }
    return def;
}

QString colorToIniString(const QColor &c)
{
    return c.name(QColor::HexArgb);
}

void csIniSetColor(CSimpleIniA &ini, const char *sec, const char *key, const QColor &c)
{
    ini.SetValue(sec, key, colorToIniString(c).toUtf8().constData(), nullptr, true);
}

bool csIniSaveValue(const char *section, const char *key, const QString &value)
{
    QByteArray path = configIniPathUtf8();
    CSimpleIniA ini(true, false, false);
    if (!csIniLoadOrEmpty(ini, path.constData()))
        return false;
    ini.SetValue(section, key, value.toUtf8().constData(), nullptr, true);
    return ini.SaveFile(path.constData(), false) >= 0;
}

bool csIniSaveInt(const char *section, const char *key, int value)
{
    QByteArray path = configIniPathUtf8();
    CSimpleIniA ini(true, false, false);
    if (!csIniLoadOrEmpty(ini, path.constData()))
        return false;
    ini.SetLongValue(section, key, value, nullptr, false, true);
    return ini.SaveFile(path.constData(), false) >= 0;
}

bool csIniSaveBool(const char *section, const char *key, bool value)
{
    QByteArray path = configIniPathUtf8();
    CSimpleIniA ini(true, false, false);
    if (!csIniLoadOrEmpty(ini, path.constData()))
        return false;
    ini.SetBoolValue(section, key, value, nullptr, true);
    return ini.SaveFile(path.constData(), false) >= 0;
}

} // namespace

Settings g_settings;

QMap<SingleShortcutKey, QString> Settings::s_singleShortcutKeyName;
QMap<CombineShortcutKey, QString> Settings::s_combineShortcutKeyName;
QMap<SingleShortcutKey, QString> Settings::s_singleShortcutKeyName_1;
QMap<CombineShortcutKey, QString> Settings::s_combineShortcutKeyName_1;
QMap<CnEnSwitchShortcutKey, QString> Settings::s_cnEnSwitchShortcutKeyName;

bool Settings::s_isShowAllGroup;
bool Settings::s_isShowAllGroupBuf;

SettingCommon Settings::s_common;
SettingCommon Settings::s_commonBuf;

SettingAdvance Settings::s_advance;
SettingAdvance Settings::s_advanceBuf;

SettingOthers Settings::s_others;
SettingOthers Settings::s_othersBuf;

SettingUi Settings::s_ui;
SettingUi Settings::s_uiBuf;

SettingCandidateWinUi Settings::s_candidateWinUi;
SettingCandidateWinUi Settings::s_candidateWinUiBuf;

SettingCandidateWinOption Settings::s_candidateWinOption;
SettingCandidateWinOption Settings::s_candidateWinOptionBuf;

SettingShortcutKey Settings::s_shortcutKey;
SettingShortcutKey Settings::s_shortcutKeyBuf;

SettingCustomKey Settings::s_customKey;
SettingCustomKey Settings::s_customKeyBuf;

SettingMisc Settings::s_miscSetting;

QString Settings::s_curUsedLexicon;
QStringList Settings::s_lexiconList;
QStringList Settings::s_skinList;

bool Settings::s_needNotifyFcitx = false;
bool Settings::s_settingDataIsChanged = false;

Settings::Settings(QObject *parent) : QObject(parent)
{
}

bool Settings::useFreewbFont()
{
    return s_miscSetting.useFreewbFont;
}

// 初始化常量数据成员,该类数据成员只供外部读取，不可对其进行修改！
void Settings::init_const_data_member()
{
    // 单快捷键
    s_singleShortcutKeyName[SSK_NONE] = "禁止";
    s_singleShortcutKeyName[SSK_SEMICOLON] = "分号( ; )";
    s_singleShortcutKeyName[SSK_QUOTE] = "引号( ' )";
    s_singleShortcutKeyName[SSK_COMMA] = "逗号( , )";
    s_singleShortcutKeyName[SSK_PERIOD] = "句号( . )";
    s_singleShortcutKeyName[SSK_BACK_QUOTE] = "反引号( ` )";
    s_singleShortcutKeyName[SSK_LEFT_BRACKETS] = "左中( [ )";
    s_singleShortcutKeyName[SSK_RIGHT_BRACKETS] = "右中( ] )";
    s_singleShortcutKeyName[SSK_BACK_SLASH] = "反斜( \\ )";
    s_singleShortcutKeyName[SSK_SLASH] = "除号( / )";
    s_singleShortcutKeyName[SSK_CHAR_u] = "u";
    s_singleShortcutKeyName[SSK_CHAR_i] = "i";
    s_singleShortcutKeyName[SSK_CHAR_v] = "v";
    s_singleShortcutKeyName[SSK_CHAR_z] = "z";

    s_singleShortcutKeyName_1[SSK_NONE] = "KEY_NONE";
    s_singleShortcutKeyName_1[SSK_SEMICOLON] = ";";      //"KEY_SEMICOLON";
    s_singleShortcutKeyName_1[SSK_QUOTE] = "\'";         //"KEY_QUOTE";
    s_singleShortcutKeyName_1[SSK_COMMA] = ",";          //"KEY_COMMA";
    s_singleShortcutKeyName_1[SSK_PERIOD] = ".";         //"KEY_PERIOD";
    s_singleShortcutKeyName_1[SSK_BACK_QUOTE] = "`";     //"KEY_BACK_QUOTE";
    s_singleShortcutKeyName_1[SSK_LEFT_BRACKETS] = "[";  //"KEY_LEFT_BRACKETS";
    s_singleShortcutKeyName_1[SSK_RIGHT_BRACKETS] = "]"; //"KEY_RIGHT_BRACKETS";
    s_singleShortcutKeyName_1[SSK_BACK_SLASH] = "\\";    //"KEY_BACK_SLASH";
    s_singleShortcutKeyName_1[SSK_SLASH] = "/";          //"KEY_SLASH";
    s_singleShortcutKeyName_1[SSK_CHAR_u] = "u";         //"KEY_U";
    s_singleShortcutKeyName_1[SSK_CHAR_i] = "i";         //"KEY_I";
    s_singleShortcutKeyName_1[SSK_CHAR_v] = "v";         //"KEY_V";
    s_singleShortcutKeyName_1[SSK_CHAR_z] = "z";         //"KEY_Z";

    // 用于设置窗口显示
    s_combineShortcutKeyName[CSK_NONE] = "禁止";
    s_combineShortcutKeyName[CSK_INSERT] = "Insert";
    s_combineShortcutKeyName[CSK_DEL] = "Del";
    s_combineShortcutKeyName[CSK_ESC] = "Esc";
    s_combineShortcutKeyName[CSK_BACKSPACE] = "Backspace";
    s_combineShortcutKeyName[CSK_HOME] = "Home";
    s_combineShortcutKeyName[CSK_END] = "End";
    s_combineShortcutKeyName[CSK_LEFT] = "←";
    s_combineShortcutKeyName[CSK_RIGHT] = "→";
    s_combineShortcutKeyName[CSK_UP] = "↑";
    s_combineShortcutKeyName[CSK_DOWN] = "↓";
    s_combineShortcutKeyName[CSK_QUOTE] = "引号( ' )";
    s_combineShortcutKeyName[CSK_SEMICOLON] = "分号( ; )";
    s_combineShortcutKeyName[CSK_BACK_SLASH] = "反斜杠( \\ )";
    s_combineShortcutKeyName[CSK_LEFT_BRACKETS] = "左中( [ )";
    s_combineShortcutKeyName[CSK_RIGHT_BRACKETS] = "右中( ] )";
    s_combineShortcutKeyName[CSK_COMMA] = "逗号( , )";
    s_combineShortcutKeyName[CSK_PERIOD] = "句号( . )";
    s_combineShortcutKeyName[CSK_SLASH] = "除号( / )";
    s_combineShortcutKeyName[CSK_BACK_QUOTE] = "反引号( ` )";
    s_combineShortcutKeyName[CSK_EQUAL] = "=";
    s_combineShortcutKeyName[CSK_DASH] = "-";
    s_combineShortcutKeyName[CSK_F1] = "F1";
    s_combineShortcutKeyName[CSK_F2] = "F2";
    s_combineShortcutKeyName[CSK_F3] = "F3";
    s_combineShortcutKeyName[CSK_F4] = "F4";
    s_combineShortcutKeyName[CSK_F5] = "F5";
    s_combineShortcutKeyName[CSK_F6] = "F6";
    s_combineShortcutKeyName[CSK_F7] = "F7";
    s_combineShortcutKeyName[CSK_F8] = "F8";
    s_combineShortcutKeyName[CSK_F9] = "F9";
    s_combineShortcutKeyName[CSK_F10] = "F10";
    s_combineShortcutKeyName[CSK_F11] = "F11";
    s_combineShortcutKeyName[CSK_F12] = "F12";
    s_combineShortcutKeyName[CSK_ESC] = "Esc";
    s_combineShortcutKeyName[CSK_a] = "A";
    s_combineShortcutKeyName[CSK_b] = "B";
    s_combineShortcutKeyName[CSK_c] = "C";
    s_combineShortcutKeyName[CSK_d] = "D";
    s_combineShortcutKeyName[CSK_e] = "E";
    s_combineShortcutKeyName[CSK_f] = "F";
    s_combineShortcutKeyName[CSK_g] = "G";
    s_combineShortcutKeyName[CSK_h] = "H";
    s_combineShortcutKeyName[CSK_i] = "I";
    s_combineShortcutKeyName[CSK_j] = "J";
    s_combineShortcutKeyName[CSK_k] = "K";
    s_combineShortcutKeyName[CSK_l] = "L";
    s_combineShortcutKeyName[CSK_m] = "M";
    s_combineShortcutKeyName[CSK_n] = "N";
    s_combineShortcutKeyName[CSK_o] = "O";
    s_combineShortcutKeyName[CSK_p] = "P";
    s_combineShortcutKeyName[CSK_q] = "Q";
    s_combineShortcutKeyName[CSK_r] = "R";
    s_combineShortcutKeyName[CSK_s] = "S";
    s_combineShortcutKeyName[CSK_t] = "T";
    s_combineShortcutKeyName[CSK_u] = "U";
    s_combineShortcutKeyName[CSK_v] = "V";
    s_combineShortcutKeyName[CSK_w] = "W";
    s_combineShortcutKeyName[CSK_x] = "X";
    s_combineShortcutKeyName[CSK_y] = "Y";
    s_combineShortcutKeyName[CSK_z] = "Z";

    // 用于配置文件存储
    s_combineShortcutKeyName_1[CSK_NONE] = "KEY_NONE";
    s_combineShortcutKeyName_1[CSK_INSERT] = "KEY_INSERT";
    s_combineShortcutKeyName_1[CSK_DEL] = "KEY_DEL";
    s_combineShortcutKeyName_1[CSK_ESC] = "KEY_ESC";
    s_combineShortcutKeyName_1[CSK_BACKSPACE] = "KEY_BACKSPACE";
    s_combineShortcutKeyName_1[CSK_HOME] = "KEY_HOME";
    s_combineShortcutKeyName_1[CSK_END] = "KEY_END";
    s_combineShortcutKeyName_1[CSK_LEFT] = "KEY_LEFT";
    s_combineShortcutKeyName_1[CSK_RIGHT] = "KEY_RIGHT";
    s_combineShortcutKeyName_1[CSK_UP] = "KEY_UP";
    s_combineShortcutKeyName_1[CSK_DOWN] = "KEY_DOWN";
    s_combineShortcutKeyName_1[CSK_QUOTE] = "KEY_QUOTE";
    s_combineShortcutKeyName_1[CSK_SEMICOLON] = "KEY_SEMICOLON";
    s_combineShortcutKeyName_1[CSK_BACK_SLASH] = "KEY_BACK_SLASH";
    s_combineShortcutKeyName_1[CSK_LEFT_BRACKETS] = "KEY_LEFT_BRACKET";
    s_combineShortcutKeyName_1[CSK_RIGHT_BRACKETS] = "KEY_RIGHT_BRACKET";
    s_combineShortcutKeyName_1[CSK_COMMA] = "KEY_COMMA";
    s_combineShortcutKeyName_1[CSK_PERIOD] = "KEY_PERIOD";
    s_combineShortcutKeyName_1[CSK_SLASH] = "KEY_SLASH";
    s_combineShortcutKeyName_1[CSK_BACK_QUOTE] = "KEY_BACKQUOTE";
    s_combineShortcutKeyName_1[CSK_EQUAL] = "KEY_EQUAL";
    s_combineShortcutKeyName_1[CSK_DASH] = "KEY_DASH";

    s_combineShortcutKeyName_1[CSK_F1] = "KEY_F1";
    s_combineShortcutKeyName_1[CSK_F2] = "KEY_F2";
    s_combineShortcutKeyName_1[CSK_F3] = "KEY_F3";
    s_combineShortcutKeyName_1[CSK_F4] = "KEY_F4";
    s_combineShortcutKeyName_1[CSK_F5] = "KEY_F5";
    s_combineShortcutKeyName_1[CSK_F6] = "KEY_F6";
    s_combineShortcutKeyName_1[CSK_F7] = "KEY_F7";
    s_combineShortcutKeyName_1[CSK_F8] = "KEY_F8";
    s_combineShortcutKeyName_1[CSK_F9] = "KEY_F9";
    s_combineShortcutKeyName_1[CSK_F10] = "KEY_F10";
    s_combineShortcutKeyName_1[CSK_F11] = "KEY_F11";
    s_combineShortcutKeyName_1[CSK_F12] = "KEY_F12";
    s_combineShortcutKeyName_1[CSK_ESC] = "KEY_ESC";

    s_combineShortcutKeyName_1[CSK_a] = "KEY_A";
    s_combineShortcutKeyName_1[CSK_b] = "KEY_B";
    s_combineShortcutKeyName_1[CSK_c] = "KEY_C";
    s_combineShortcutKeyName_1[CSK_d] = "KEY_D";
    s_combineShortcutKeyName_1[CSK_e] = "KEY_E";
    s_combineShortcutKeyName_1[CSK_f] = "KEY_F";
    s_combineShortcutKeyName_1[CSK_g] = "KEY_G";
    s_combineShortcutKeyName_1[CSK_h] = "KEY_H";
    s_combineShortcutKeyName_1[CSK_i] = "KEY_I";
    s_combineShortcutKeyName_1[CSK_j] = "KEY_J";
    s_combineShortcutKeyName_1[CSK_k] = "KEY_K";
    s_combineShortcutKeyName_1[CSK_l] = "KEY_L";
    s_combineShortcutKeyName_1[CSK_m] = "KEY_M";
    s_combineShortcutKeyName_1[CSK_n] = "KEY_N";
    s_combineShortcutKeyName_1[CSK_o] = "KEY_O";
    s_combineShortcutKeyName_1[CSK_p] = "KEY_P";
    s_combineShortcutKeyName_1[CSK_q] = "KEY_Q";
    s_combineShortcutKeyName_1[CSK_r] = "KEY_R";
    s_combineShortcutKeyName_1[CSK_s] = "KEY_S";
    s_combineShortcutKeyName_1[CSK_t] = "KEY_T";
    s_combineShortcutKeyName_1[CSK_u] = "KEY_U";
    s_combineShortcutKeyName_1[CSK_v] = "KEY_V";
    s_combineShortcutKeyName_1[CSK_w] = "KEY_W";
    s_combineShortcutKeyName_1[CSK_x] = "KEY_X";
    s_combineShortcutKeyName_1[CSK_y] = "KEY_Y";
    s_combineShortcutKeyName_1[CSK_z] = "KEY_Z";
    // 中英文切换快捷键
    s_cnEnSwitchShortcutKeyName[CESSK_NONE] = "KEY_NONE";
    s_cnEnSwitchShortcutKeyName[CESSK_LEFT_SHIFT] = "KEY_LEFT_SHIFT";
    s_cnEnSwitchShortcutKeyName[CESSK_RIGHT_SHIFT] = "KEY_RIGHT_SHIFT";
    s_cnEnSwitchShortcutKeyName[CESSK_SHIFT] = "KEY_SHIFT";
    s_cnEnSwitchShortcutKeyName[CESSK_LEFT_CTRL] = "KEY_LEFT_CTRL";
    s_cnEnSwitchShortcutKeyName[CESSK_RIGHT_CTRL] = "KEY_RIGHT_CTRL";
    s_cnEnSwitchShortcutKeyName[CESSK_CTRL] = "KEY_CTRL";
}

// 所有设置参数恢复默认值，该函数对正式配置变量进行读写
void Settings::slot_restore_default_all_config()
{
    s_isShowAllGroupBuf = false;

    s_commonBuf.codeRemind = true;             // 启用编码逐渐提示
    s_commonBuf.spaceFullWhenCharHalf = false; // 字符半角时空格全角
    s_commonBuf.wordThink = false;             // 启用词组联想功能
    s_commonBuf.smartMark = true;              // 启用智能标点功能
    s_commonBuf.remindExistWord = false;       // 提示系统已有词组
    s_commonBuf.alertWhenEmptyCode = true;     // 重码或空码时发声提示
    s_commonBuf.useAudioFile = true;           // 使用声音文件
    s_commonBuf.autoAdjustFreq = false;        // 启动自动调频功能

    s_advanceBuf.shiftCommitChar = true;       // Shift+字母直接上屏
    s_advanceBuf.inputStatistic = false;       // 启用输入统计功能
    s_advanceBuf.typeEffect = false;           // 启用打字音效功能
    s_advanceBuf.recodeCalib = false;          // 重码上屏校对模式
    s_advanceBuf.autoWordGroupOpt = AWGO_LOSS; // 自动词组选项

    s_othersBuf.autoToEnStr = "www. ftp: http mail. bbs.";
    s_othersBuf.autoToHalfMark = ".";
    s_othersBuf.autoToHalfMarkFlg = false;

    s_uiBuf.curSkinId = "default";       // 当前使用皮肤
    s_uiBuf.toolbarAutoLocate = false;   // 状态栏自动定位
    s_uiBuf.toolbarAutoExpand = false;   // 状态栏菜单自动伸缩
    s_uiBuf.enableUiAudioEffect = false; // 使用界面音效
    s_uiBuf.showRealtimeHelp = true;     // 显示实时帮助
    s_uiBuf.hideToolbar = false;         // 隐藏状态栏
    s_uiBuf.toolbarTransparency = 0;     // 状态栏透明度

    s_candidateWinUiBuf.candiWinDispMode = CWDM_ONE_ROW;
    s_candidateWinUiBuf.show_cand_dict = true;
    s_candidateWinUiBuf.separateChar = '.';                              // 候选词序号与词组间的分隔符
    s_candidateWinUiBuf.useGradientColor = false;                        // 是否使用渐变色
    s_candidateWinUiBuf.useBgImage = false;                              // 是否使用背景图
    s_candidateWinUiBuf.enablebgImageTiled = false;                      // 使能背景图平铺
    s_candidateWinUiBuf.radius = 5;                                      // 圆角弧度
    s_candidateWinUiBuf.candiWinTransparency = 0;                        // 透明度
    s_candidateWinUiBuf.candiWordCount = 5;                              // 候选词个数
    s_candidateWinUiBuf.candiCharCount = 9;                              // 候选词字数
    s_candidateWinUiBuf.candiTextFont = QFont("Ubuntu", 14);             // 候选词字数
    s_candidateWinUiBuf.bgImage = "";                                    // 背景图片
    s_candidateWinUiBuf.enablebgImageTiled = false;                      // 背景图平铺
    s_candidateWinUiBuf.bgColor = QColor(qRgb(255, 255, 255));           // 背景颜色
    s_candidateWinUiBuf.gradientColor0 = QColor(qRgb(255, 255, 255));    // 渐变起始色
    s_candidateWinUiBuf.gradientColor1 = QColor(qRgb(255, 255, 255));    // 渐变结束色
    s_candidateWinUiBuf.borderColor = QColor(qRgb(164, 193, 226));       // 边框颜色
    s_candidateWinUiBuf.candiWordTextColor = QColor(qRgb(95, 91, 82));   // 候选词颜色
    s_candidateWinUiBuf.candiPromptTextColor = QColor(qRgb(95, 91, 82)); // 提示信息颜色

    s_candidateWinOptionBuf.recodeSelectKey = RSSK_SEMI_QUOTE;
    s_candidateWinOptionBuf.candiPageKey = CPSK_SUB_EQUAL;
    s_candidateWinOptionBuf.secondRecodeKey = "KEY_SEMICOLON";
    s_candidateWinOptionBuf.thirdRecodeKey = "KEY_QUOTE";
    s_candidateWinOptionBuf.prevPageKey = "KEY_DASH";
    s_candidateWinOptionBuf.nextPageKey = "KEY_EQUAL";
    s_candidateWinOptionBuf.cursorFollowFlg = true;
    s_candidateWinOptionBuf.hideCandiWinFlg = false;
    s_candidateWinOptionBuf.showOpRemindInfoFlg = true;
    s_candidateWinOptionBuf.shiftSelectRecodeFlg = false;

    restore_default_shortcutkey();
    restore_default_customkey();

    s_miscSetting.enterClear = true;
    s_miscSetting.inputMethod = 1;
    s_miscSetting.currentCharset = 0;
    s_curUsedLexicon = "default";

    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

// 从配置文件读取配置数据，该函数对正式配置变量进行读写
void Settings::load_all_setting_data_from_file()
{
    QFile cfgFile(CONFIG_FILE);

    if (!cfgFile.exists())
    {
        slot_restore_default_all_config();
        s_settingDataIsChanged = true;
        save_all_setting_data_to_file();
        return;
    }

    CSimpleIniA cfgIni(true, false, false);
    if (!csIniLoadOrEmpty(cfgIni, configIniPathUtf8().constData()))
    {
        qWarning() << "config.ini load failed (parse error or out of memory):" << CONFIG_FILE;
        slot_restore_default_all_config();
        save_all_setting_data_to_file();
        copy_all_setting_data_to_buffer();
        return;
    }

    s_common.codeRemind = cfgIni.GetBoolValue("Common", "codeRemind", true);
    s_common.autoAdjustFreq = cfgIni.GetBoolValue("Common", "autoAdjustFreq", false);
    s_common.wordThink = cfgIni.GetBoolValue("Common", "wordThink", false);
    s_common.remindExistWord = cfgIni.GetBoolValue("Common", "remindExistWord", false);
    s_common.alertWhenEmptyCode = cfgIni.GetBoolValue("Common", "alertWhenEmptyCode", true);
    s_miscSetting.enterClear = cfgIni.GetBoolValue("Common", "enterClear", true);
    s_common.spaceFullWhenCharHalf = cfgIni.GetBoolValue("Common", "spaceFullWhenCharHalf", false);
    s_common.smartMark = cfgIni.GetBoolValue("Common", "smartMark", false);

    s_advance.shiftCommitChar = cfgIni.GetBoolValue("Advanced", "shiftCommitChar", true);
    s_advance.typeEffect = cfgIni.GetBoolValue("Advanced", "typeEffect", false);
    s_advance.recodeCalib = cfgIni.GetBoolValue("Advanced", "recodeCalib", false);
    s_advance.autoWordGroupOpt = static_cast<AutoWordGroupOpt>(static_cast<int>(cfgIni.GetLongValue("Advanced", "autoWordGroupOpt", 0)));

    s_others.autoToEnStr = QString::fromUtf8(cfgIni.GetValue("Others", "autoToEnStr", "", nullptr));
    s_others.autoToHalfMarkFlg = cfgIni.GetBoolValue("Others", "autoToHalfMarkFlg", false);

    set_custom_mark_value(QString::fromUtf8(cfgIni.GetValue("Misc", "CoustomMark", "", nullptr)));
    set_custom_char_value(QString::fromUtf8(cfgIni.GetValue("Misc", "CoustomChar", "", nullptr)));
    {
        const QByteArray defLex = (INSTALL_DIR + "/data/mb/default").toUtf8();
        s_curUsedLexicon = QString::fromUtf8(cfgIni.GetValue("Misc", "curUsedLexicon", defLex.constData(), nullptr));
    }
    s_miscSetting.inputMethod = static_cast<int>(cfgIni.GetLongValue("Misc", "inputMode", 1));
    s_miscSetting.currentCharset = static_cast<int>(cfgIni.GetLongValue("Misc", "currentCharset", 1));
    s_miscSetting.simpTranFlg = static_cast<int>(cfgIni.GetLongValue("Misc", "simpTradFlg", 0));

    s_ui.curSkinId = QString::fromUtf8(cfgIni.GetValue("Ui", "curSkinId", "default", nullptr));
    s_ui.toolbarAutoLocate = cfgIni.GetBoolValue("Ui", "toolbarAutoLocate", false);
    s_ui.toolbarAutoExpand = cfgIni.GetBoolValue("Ui", "toolbarAutoExpand", false);
    s_ui.enableUiAudioEffect = cfgIni.GetBoolValue("Ui", "uiAudioEffect", false);
    s_ui.showRealtimeHelp = cfgIni.GetBoolValue("Ui", "showRealtimeHelp", false);
    s_ui.hideToolbar = cfgIni.GetBoolValue("Ui", "hideToolbar", false);
    s_ui.toolbarTransparency = static_cast<int>(cfgIni.GetLongValue("Ui", "toolbarTransparency", 0));

    s_candidateWinUi.show_cand_dict = cfgIni.GetBoolValue("CandidateWinUi", "showCandDictInfo", false);
    s_candidateWinUi.candiWinDispMode = static_cast<CandiWinDispMode>(static_cast<int>(cfgIni.GetLongValue("CandidateWinUi", "candiWinDispMode", static_cast<int>(CWDM_ONE_ROW))));
    {
        const char *sepRaw = cfgIni.GetValue("CandidateWinUi", "separateChar", " ", nullptr);
        s_candidateWinUi.separateChar = QString::fromUtf8(sepRaw).at(0).toLatin1();
    }
    s_candidateWinUi.useGradientColor = cfgIni.GetBoolValue("CandidateWinUi", "useGradientColor", false);
    s_candidateWinUi.useBgImage = cfgIni.GetBoolValue("CandidateWinUi", "useBgImage", false);
    s_candidateWinUi.enablebgImageTiled = cfgIni.GetBoolValue("CandidateWinUi", "enableTiled", false);
    s_candidateWinUi.radius = static_cast<int>(cfgIni.GetLongValue("CandidateWinUi", "radius", 0));
    s_candidateWinUi.candiWinTransparency = static_cast<int>(cfgIni.GetLongValue("CandidateWinUi", "transparency", 0));
    s_candidateWinUi.candiCharCount = static_cast<int>(cfgIni.GetLongValue("CandidateWinUi", "candiCharCount", 0));

    QString fontName = QString::fromUtf8(cfgIni.GetValue("CandidateWinUi", "candiTextFontName", "", nullptr));
    if (fontName.isEmpty())
        fontName = QStringLiteral("Ubuntu");
    int fontSize = static_cast<int>(cfgIni.GetLongValue("CandidateWinUi", "candiTextFontSize", 0));
    if (fontSize < 9)
        fontSize = 14;
    s_candidateWinUi.candiTextFont = QFont(fontName, fontSize);

    s_candidateWinUi.bgImage = QString::fromUtf8(cfgIni.GetValue("CandidateWinUi", "bgImage", "", nullptr));
    s_candidateWinUi.bgColor = csIniGetColor(cfgIni, "CandidateWinUi", "bgColor", QColor());
    s_candidateWinUi.gradientColor0 = csIniGetColor(cfgIni, "CandidateWinUi", "gradientColor0", QColor());
    s_candidateWinUi.gradientColor1 = csIniGetColor(cfgIni, "CandidateWinUi", "gradientColor1", QColor());
    s_candidateWinUi.borderColor = csIniGetColor(cfgIni, "CandidateWinUi", "borderColor", QColor());
    s_candidateWinUi.candiWordTextColor = csIniGetColor(cfgIni, "CandidateWinUi", "candiWordTextColor", QColor());
    s_candidateWinUi.candiPromptTextColor = csIniGetColor(cfgIni, "CandidateWinUi", "candiPromptTextColor", QColor());

    s_candidateWinOption.recodeSelectKey = static_cast<RcodeSelectShortcutKey>(static_cast<int>(cfgIni.GetLongValue("CandidateWinOptions", "recodeSelectKey", static_cast<int>(RSSK_SEMI_QUOTE))));
    s_candidateWinOption.candiPageKey = static_cast<CandiPageShortcutKey>(static_cast<int>(cfgIni.GetLongValue("CandidateWinOptions", "candiPageKey", static_cast<int>(CPSK_SUB_EQUAL))));

    s_candidateWinOption.cursorFollowFlg = cfgIni.GetBoolValue("CandidateWinOptions", "cursorFollow", false);
    s_candidateWinOption.hideCandiWinFlg = cfgIni.GetBoolValue("CandidateWinOptions", "hideCandiWin", false);
    s_candidateWinOption.showOpRemindInfoFlg = cfgIni.GetBoolValue("CandidateWinOptions", "showOpRemindInfo", false);
    s_candidateWinOption.shiftSelectRecodeFlg = cfgIni.GetBoolValue("CandidateWinOptions", "shiftSelectRecode", false);

    s_candidateWinUi.candiWordCount = static_cast<int>(cfgIni.GetLongValue("CandidateWinOptions", "candiWordCount", 5));
    s_candidateWinOption.secondRecodeKey = QString::fromUtf8(cfgIni.GetValue("CandidateWinOptions", "secondRecodeKey", "KEY_SEMICOLON", nullptr));
    s_candidateWinOption.thirdRecodeKey = QString::fromUtf8(cfgIni.GetValue("CandidateWinOptions", "thirdRecodeKey", "KEY_QUOTE", nullptr));
    s_candidateWinOption.prevPageKey = QString::fromUtf8(cfgIni.GetValue("CandidateWinOptions", "prevPageKey", "KEY_DASH", nullptr));
    s_candidateWinOption.nextPageKey = QString::fromUtf8(cfgIni.GetValue("CandidateWinOptions", "nextPageKey", "KEY_EQUAL", nullptr));
    s_miscSetting.useFreewbFont = cfgIni.GetBoolValue("CandidateWinOptions", "useFreewbFont", false);

    s_shortcutKey.customShortcutFunc[CSF_BACK_FIND_CODE] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "backFindCode", "CTRL+KEY_SLASH", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_ONLINE_ADD_WORD] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "onlineAddWord", "CTRL+KEY_EQUAL", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_ONLINE_DEL_WORD] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "onlineDelWord", "CTRL+KEY_DASH", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_SWITCH_KEYBOARD] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "switchVKb", "CTRL+KEY_ESC", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_SWITCH_CHAR_SET] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "switchCharSet", "CTRL+KEY_M", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_SWITCH_INPUT_MODE] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "switchInputMode", "CTRL+KEY_BACK_SLASH", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_SWITCH_WORD_STATE] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "switchWordState", "CTRL+KEY_INSERT", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_SWITCH_S_IN_T_OUT] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "switchChttrans", "CTRL+KEY_J", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_SETUP_OPTION] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "setupOption", "CTRL+KEY_COMMA", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_QUICK_DEL_SCREEN_CHAR] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "quickDelScreenItem", "CTRL+KEY_BACKSPACE", nullptr)));

    s_shortcutKey.sskTmpEnglish = convert_name_to_singleShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "tempEnglish", ";", nullptr)));
    s_shortcutKey.sskShortcutInput = convert_name_to_singleShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "shortcutInput", "'", nullptr)));
    s_shortcutKey.sskTmpPinyin = convert_name_to_singleShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "tempPinyin", "`", nullptr)));

    s_shortcutKey.customShortcutFunc[CSF_MARK_AUTO_PAIR] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "markAutoPair", "", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_SWITCH_SKIN] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "switchSkin", "", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_SHOW_HIDE_STATUS_BAR] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "showHideToolbar", "", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_SHOW_HIDE_CANDIDATE_WIN] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "showHideCandiWin", "", nullptr)));

    s_shortcutKey.customShortcutFunc[CSF_SWITCH_WORD_LEXICON] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "switchLexicon", "", nullptr)));
    s_shortcutKey.customShortcutFunc[CSF_ADD_CHAR_AFTER_OUTPUT] = convert_name_to_combineShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "addCharAfterOutput", "CTRL+KEY_NONE", nullptr)));

    s_shortcutKey.ceSwitchShortcutkey = convert_name_to_cnEnSwitchShortcutKey(QString::fromUtf8(cfgIni.GetValue("ShortcutKey", "cnEnSwitch", "", nullptr)));
    s_shortcutKey.disableAllCsk = cfgIni.GetBoolValue("ShortcutKey", "disableAllShortcutKey", false);
    s_shortcutKey.disableFullHalfSwitchShortcutkey = cfgIni.GetBoolValue("ShortcutKey", "disableFullHalfSwitch", false);

    // 拷贝正式配置数据到临时配置变量
    copy_all_setting_data_to_buffer();
}

// 永久保存所有配置到配置文件中去
void Settings::save_all_setting_data_to_file()
{
    if (!s_settingDataIsChanged)
        return;
    s_settingDataIsChanged = false;

    // 将所有临时配置数据拷贝到正式配置变量中
    copy_all_buffer_to_setting_data();

    QByteArray cfgPath = configIniPathUtf8();
    CSimpleIniA cfgIni(true, false, false);
    csIniLoadOrEmpty(cfgIni, cfgPath.constData());

    cfgIni.SetBoolValue("Common", "codeRemind", s_common.codeRemind, nullptr, true);
    cfgIni.SetBoolValue("Common", "autoAdjustFreq", s_common.autoAdjustFreq, nullptr, true);
    cfgIni.SetBoolValue("Common", "wordThink", s_common.wordThink, nullptr, true);
    cfgIni.SetBoolValue("Common", "remindExistWord", s_common.remindExistWord, nullptr, true);
    cfgIni.SetBoolValue("Common", "alertWhenEmptyCode", s_common.alertWhenEmptyCode, nullptr, true);
    cfgIni.SetBoolValue("Common", "enterClear", s_miscSetting.enterClear, nullptr, true);
    cfgIni.SetBoolValue("Common", "spaceFullWhenCharHalf", s_common.spaceFullWhenCharHalf, nullptr, true);
    cfgIni.SetBoolValue("Common", "smartMark", s_common.smartMark, nullptr, true);

    cfgIni.SetBoolValue("Advanced", "shiftCommitChar", s_advance.shiftCommitChar, nullptr, true);
    cfgIni.SetBoolValue("Advanced", "typeEffect", s_advance.typeEffect, nullptr, true);
    cfgIni.SetBoolValue("Advanced", "recodeCalib", s_advance.recodeCalib, nullptr, true);
    cfgIni.SetLongValue("Advanced", "autoWordGroupOpt", static_cast<int>(s_advance.autoWordGroupOpt), nullptr, false, true);

    cfgIni.SetValue("Misc", "curUsedLexicon", (s_curUsedLexicon).toUtf8().constData(), nullptr, true);
    cfgIni.SetValue("Misc", "wubiTable", (s_curUsedLexicon + QStringLiteral("/freeime.mb")).toUtf8().constData(), nullptr, true);
    cfgIni.SetValue("Misc", "pinyinTable", (s_curUsedLexicon + QStringLiteral("/attach.mb")).toUtf8().constData(), nullptr, true);
    cfgIni.SetLongValue("Misc", "inputMode", s_miscSetting.inputMethod, nullptr, false, true);
    cfgIni.SetLongValue("Misc", "currentCharset", s_miscSetting.currentCharset, nullptr, false, true);
    cfgIni.SetLongValue("Misc", "simpTradFlg", s_miscSetting.simpTranFlg, nullptr, false, true);

    cfgIni.SetValue("Misc", "CoustomChar", (get_custom_char_value()).toUtf8().constData(), nullptr, true);
    cfgIni.SetValue("Misc", "CoustomMark", (get_custom_mark_value()).toUtf8().constData(), nullptr, true);

    cfgIni.SetValue("Others", "autoToEnStr", (s_others.autoToEnStr).toUtf8().constData(), nullptr, true);
    cfgIni.SetBoolValue("Others", "autoToHalfMarkFlg", s_others.autoToHalfMarkFlg, nullptr, true);

    cfgIni.SetValue("Ui", "curSkinId", (s_ui.curSkinId).toUtf8().constData(), nullptr, true);
    cfgIni.SetBoolValue("Ui", "toolbarAutoLocate", s_ui.toolbarAutoLocate, nullptr, true);
    cfgIni.SetBoolValue("Ui", "toolbarAutoExpand", s_ui.toolbarAutoExpand, nullptr, true);
    cfgIni.SetBoolValue("Ui", "uiAudioEffect", s_ui.enableUiAudioEffect, nullptr, true);
    cfgIni.SetBoolValue("Ui", "showRealtimeHelp", s_ui.showRealtimeHelp, nullptr, true);
    cfgIni.SetBoolValue("Ui", "hideToolbar", s_ui.hideToolbar, nullptr, true);
    cfgIni.SetLongValue("Ui", "toolbarTransparency", s_ui.toolbarTransparency, nullptr, false, true);

    cfgIni.SetLongValue("CandidateWinUi", "candiWinDispMode", static_cast<int>(s_candidateWinUi.candiWinDispMode), nullptr, false, true);
    cfgIni.SetValue("CandidateWinUi", "separateChar", (QString(QChar(s_candidateWinUi.separateChar))).toUtf8().constData(), nullptr, true);
    cfgIni.SetBoolValue("CandidateWinUi", "useGradientColor", s_candidateWinUi.useGradientColor, nullptr, true);
    cfgIni.SetBoolValue("CandidateWinUi", "useBgImage", s_candidateWinUi.useBgImage, nullptr, true);
    cfgIni.SetBoolValue("CandidateWinUi", "enableTiled", s_candidateWinUi.enablebgImageTiled, nullptr, true);

    cfgIni.SetLongValue("CandidateWinUi", "radius", s_candidateWinUi.radius, nullptr, false, true);
    cfgIni.SetLongValue("CandidateWinUi", "transparency", s_candidateWinUi.candiWinTransparency, nullptr, false, true);
    cfgIni.SetLongValue("CandidateWinUi", "candiCharCount", s_candidateWinUi.candiCharCount, nullptr, false, true);

    cfgIni.SetValue("CandidateWinUi", "candiTextFontName", (s_candidateWinUi.candiTextFont.family()).toUtf8().constData(), nullptr, true);
    cfgIni.SetLongValue("CandidateWinUi", "candiTextFontSize", s_candidateWinUi.candiTextFont.pointSize(), nullptr, false, true);
    cfgIni.SetBoolValue("CandidateWinUi", "showCandDictInfo", s_candidateWinUi.show_cand_dict, nullptr, true);

    cfgIni.SetValue("CandidateWinUi", "bgImage", (s_candidateWinUi.bgImage).toUtf8().constData(), nullptr, true);
    csIniSetColor(cfgIni, "CandidateWinUi", "bgColor", s_candidateWinUi.bgColor);
    csIniSetColor(cfgIni, "CandidateWinUi", "gradientColor0", s_candidateWinUi.gradientColor0);
    csIniSetColor(cfgIni, "CandidateWinUi", "gradientColor1", s_candidateWinUi.gradientColor1);
    csIniSetColor(cfgIni, "CandidateWinUi", "borderColor", s_candidateWinUi.borderColor);
    csIniSetColor(cfgIni, "CandidateWinUi", "candiWordTextColor", s_candidateWinUi.candiWordTextColor);
    csIniSetColor(cfgIni, "CandidateWinUi", "candiPromptTextColor", s_candidateWinUi.candiPromptTextColor);

    cfgIni.SetLongValue("CandidateWinOptions", "recodeSelectKey", static_cast<int>(s_candidateWinOption.recodeSelectKey), nullptr, false, true);
    cfgIni.SetLongValue("CandidateWinOptions", "candiPageKey", static_cast<int>(s_candidateWinOption.candiPageKey), nullptr, false, true);
    cfgIni.SetBoolValue("CandidateWinOptions", "cursorFollow", s_candidateWinOption.cursorFollowFlg, nullptr, true);
    cfgIni.SetBoolValue("CandidateWinOptions", "hideCandiWin", s_candidateWinOption.hideCandiWinFlg, nullptr, true);
    cfgIni.SetBoolValue("CandidateWinOptions", "showOpRemindInfo", s_candidateWinOption.showOpRemindInfoFlg, nullptr, true);
    cfgIni.SetBoolValue("CandidateWinOptions", "shiftSelectRecode", s_candidateWinOption.shiftSelectRecodeFlg, nullptr, true);

    cfgIni.SetLongValue("CandidateWinOptions", "candiWordCount", s_candidateWinUi.candiWordCount, nullptr, false, true);
    cfgIni.SetValue("CandidateWinOptions", "secondRecodeKey", (s_candidateWinOption.secondRecodeKey).toUtf8().constData(), nullptr, true);
    cfgIni.SetValue("CandidateWinOptions", "thirdRecodeKey", (s_candidateWinOption.thirdRecodeKey).toUtf8().constData(), nullptr, true);
    cfgIni.SetValue("CandidateWinOptions", "prevPageKey", (s_candidateWinOption.prevPageKey).toUtf8().constData(), nullptr, true);
    cfgIni.SetValue("CandidateWinOptions", "nextPageKey", (s_candidateWinOption.nextPageKey).toUtf8().constData(), nullptr, true);
    cfgIni.SetBoolValue("CandidateWinOptions", "useFreewbFont", s_miscSetting.useFreewbFont, nullptr, true);

    QString key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_BACK_FIND_CODE]];
    cfgIni.SetValue("ShortcutKey", "backFindCode", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_ONLINE_ADD_WORD]];
    cfgIni.SetValue("ShortcutKey", "onlineAddWord", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_ONLINE_DEL_WORD]];
    cfgIni.SetValue("ShortcutKey", "onlineDelWord", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SWITCH_KEYBOARD]];
    cfgIni.SetValue("ShortcutKey", "switchVKb", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SWITCH_CHAR_SET]];
    cfgIni.SetValue("ShortcutKey", "switchCharSet", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SWITCH_INPUT_MODE]];
    cfgIni.SetValue("ShortcutKey", "switchInputMode", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SWITCH_WORD_STATE]];
    cfgIni.SetValue("ShortcutKey", "switchWordState", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SWITCH_S_IN_T_OUT]];
    cfgIni.SetValue("ShortcutKey", "switchChttrans", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SETUP_OPTION]];
    cfgIni.SetValue("ShortcutKey", "setupOption", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_QUICK_DEL_SCREEN_CHAR]];
    cfgIni.SetValue("ShortcutKey", "quickDelScreenItem", (key).toUtf8().constData(), nullptr, true);

    cfgIni.SetValue("ShortcutKey", "tempEnglish", (s_singleShortcutKeyName_1[s_shortcutKey.sskTmpEnglish]).toUtf8().constData(), nullptr, true);
    cfgIni.SetValue("ShortcutKey", "shortcutInput", (s_singleShortcutKeyName_1[s_shortcutKey.sskShortcutInput]).toUtf8().constData(), nullptr, true);
    cfgIni.SetValue("ShortcutKey", "tempPinyin", (s_singleShortcutKeyName_1[s_shortcutKey.sskTmpPinyin]).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SWITCH_WORD_LEXICON]];
    cfgIni.SetValue("ShortcutKey", "switchLexicon", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_ADD_CHAR_AFTER_OUTPUT]];
    cfgIni.SetValue("ShortcutKey", "addCharAfterOutput", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_MARK_AUTO_PAIR]];
    cfgIni.SetValue("ShortcutKey", "markAutoPair", (key).toUtf8().constData(), nullptr, true);

    cfgIni.SetValue("ShortcutKey", "cnEnSwitch", (s_cnEnSwitchShortcutKeyName[s_shortcutKey.ceSwitchShortcutkey]).toUtf8().constData(), nullptr, true);
    cfgIni.SetBoolValue("ShortcutKey", "disableAllShortcutKey", s_shortcutKey.disableAllCsk, nullptr, true);
    cfgIni.SetBoolValue("ShortcutKey", "disableFullHalfSwitch", s_shortcutKey.disableFullHalfSwitchShortcutkey, nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SWITCH_SKIN]];
    cfgIni.SetValue("ShortcutKey", "switchSkin", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SHOW_HIDE_STATUS_BAR]];
    cfgIni.SetValue("ShortcutKey", "showHideToolbar", (key).toUtf8().constData(), nullptr, true);

    key = "CTRL+" + s_combineShortcutKeyName_1[s_shortcutKey.customShortcutFunc[CSF_SHOW_HIDE_CANDIDATE_WIN]];
    cfgIni.SetValue("ShortcutKey", "showHideCandiWin", (key).toUtf8().constData(), nullptr, true);

    cfgIni.SaveFile(cfgPath.constData(), false);

#ifdef DEBUG
    qDebug() << "save all setting data to file!";
#endif
    g_settings.signal_setting_data_changed_to_local();

    // if ( s_needNotifyFcitx )
    {
        s_needNotifyFcitx = false;
        g_settings.signal_setting_data_changed_to_fcitx(); // 通知fcitx配置数据发生改变

        qDebug() << "notify fcitx to reload config data!";
    }
}

void Settings::save_cur_skin_id_to_file()
{
    // qDebug() << "save cur skin id to file!";

    // 将临时配置数据拷贝到正式配置变量中
    s_ui.curSkinId = s_uiBuf.curSkinId;

    csIniSaveValue("Ui", "curSkinId", s_ui.curSkinId);

    g_settings.signal_setting_data_changed_to_local();
}

void Settings::save_userWord_change_flg_to_file(int flg)
{
    // qDebug() << flg;

    csIniSaveInt("Misc", "userWordFlg", flg);
}

void Settings::save_quickTable_change_flg_to_file(int flg)
{
    // qDebug() <<  flg;

    csIniSaveInt("Misc", "quickTableFlg", flg);
}

int Settings::get_input_method()
{
    return s_miscSetting.inputMethod;
}

void Settings::save_inputmethod_flg_to_file(InputMode im)
{
    if (im > 2)
        return;
    // qDebug() << "set inputmethod flg to file!" << im;
    s_miscSetting.inputMethod = (int)im;
    csIniSaveInt("Misc", "inputMode", static_cast<int>(im));
}

void Settings::save_char_trad_flg_to_file(CharFontMode mode)
{
    csIniSaveInt("Misc", "simpTradFlg", static_cast<int>(mode));
    s_miscSetting.simpTranFlg = (int)mode;
}

int Settings::get_charset()
{
    return s_miscSetting.currentCharset;
}

void Settings::save_charSet_flg_to_file(int flg)
{
    // qDebug() << "set char set flg to file!" << flg;

    csIniSaveInt("Misc", "currentCharset", flg);

    s_miscSetting.currentCharset = flg;
}

void Settings::save_hide_toolbar_flg_to_file()
{
    s_ui.hideToolbar = s_uiBuf.hideToolbar;

    csIniSaveBool("Ui", "hideToolbar", s_ui.hideToolbar);

    g_settings.signal_setting_data_changed_to_local();
}

void Settings::save_hide_candiwin_flg_to_file()
{
    s_candidateWinOption.hideCandiWinFlg = s_candidateWinOptionBuf.hideCandiWinFlg;

    csIniSaveBool("CandidateWinOptions", "hideCandiWin", s_candidateWinOption.hideCandiWinFlg);

    g_settings.signal_setting_data_changed_to_local();
}

void Settings::save_smart_mark_flg_to_file()
{
    s_common.smartMark = s_commonBuf.smartMark;

    csIniSaveBool("Common", "smartMark", s_common.smartMark);

    //    g_settings.signal_setting_data_changed_to_local();
    //    g_settings.signal_setting_data_changed_to_fcitx();
}

void Settings::save_recode_calib_flg_to_file()
{
    s_advance.recodeCalib = s_advanceBuf.recodeCalib;

    csIniSaveBool("Advanced", "recodeCalib", s_advance.recodeCalib);

    //    g_settings.signal_setting_data_changed_to_local();
    //    g_settings.signal_setting_data_changed_to_fcitx();
}

void Settings::save_ime_table_changed_flg_to_file(int flg)
{
#ifdef DEBUG
    qDebug() << flg;
#endif

    csIniSaveInt("Misc", "imeTableChanged", flg);
}

void Settings::save_vk_mode_flg_to_file(int flg)
{
    // qDebug() << flg;

    csIniSaveInt("Misc", "vkMode", flg);
}

const QStringList &Settings::get_exist_lexicon()
{
    return s_lexiconList;
}

void Settings::save_exist_lexicon(const QStringList &lexiconList)
{
    s_lexiconList = lexiconList;
}

const QString &Settings::get_cur_used_lexicon()
{
    return s_curUsedLexicon;
}

void Settings::save_cur_used_lexicon_to_file(const QString &lexiconName)
{
    s_curUsedLexicon = lexiconName;

    QByteArray path = configIniPathUtf8();
    CSimpleIniA ini(true, false, false);
    if (!csIniLoadOrEmpty(ini, path.constData()))
        return;
    ini.SetValue("Misc", "curUsedLexicon", s_curUsedLexicon.toUtf8().constData(), nullptr, true);
    ini.SetValue("Misc", "wubiTable", (s_curUsedLexicon + "/freeime.mb").toUtf8().constData(), nullptr, true);
    ini.SetValue("Misc", "pinyinTable", (s_curUsedLexicon + "/attach.mb").toUtf8().constData(), nullptr, true);
    ini.SaveFile(path.constData(), false);
}

// 将正式配置数据拷贝到临时数据供设置界面临时编辑
void Settings::copy_all_setting_data_to_buffer()
{
    s_isShowAllGroupBuf = s_isShowAllGroup;
    s_commonBuf = s_common;
    s_advanceBuf = s_advance;
    s_othersBuf = s_others;
    s_uiBuf = s_ui;
    s_candidateWinUiBuf = s_candidateWinUi;
    s_candidateWinOptionBuf = s_candidateWinOption;
    s_shortcutKeyBuf = s_shortcutKey;
    s_customKeyBuf = s_customKey;

    s_settingDataIsChanged = false;
    s_needNotifyFcitx = false;
}

// 将临时配置数据赋值给正式配置变量
void Settings::copy_all_buffer_to_setting_data()
{
    s_isShowAllGroup = s_isShowAllGroupBuf;
    s_common = s_commonBuf;
    s_advance = s_advanceBuf;
    s_others = s_othersBuf;
    s_ui = s_uiBuf;
    s_candidateWinUi = s_candidateWinUiBuf;
    s_candidateWinOption = s_candidateWinOptionBuf;
    s_shortcutKey = s_shortcutKeyBuf;
    s_customKey = s_customKeyBuf;
}

const QStringList &Settings::get_exist_skin()
{
    return s_skinList;
}

void Settings::save_exist_skin(const QStringList &skinList)
{
    s_skinList = skinList;
}

SingleShortcutKey Settings::convert_name_to_singleShortcutKey(const QString name)
{
    SingleShortcutKey key = SSK_NONE;
    // puts(name.toUtf8().constData());
    for (int i = 0; i < SSK_NUM; i++)
    {
        // puts(s_singleShortcutKeyName_1[static_cast<SingleShortcutKey>(i)].toUtf8().constData());
        if (name == s_singleShortcutKeyName_1[static_cast<SingleShortcutKey>(i)])
        {
            key = static_cast<SingleShortcutKey>(i);
            break;
        }
    }
    return key;
}

CombineShortcutKey Settings::convert_name_to_combineShortcutKey(const QString name)
{
    const CombineShortcutKey none = CSK_NONE;
    static constexpr int kPrefixLen = 5; // "CTRL+" / "CTRL_"

    if (name.size() < kPrefixLen || !name.startsWith(QStringLiteral("CTRL")))
        return none;

    const QChar sep = name.at(4);
    if (sep != QLatin1Char('+') && sep != QLatin1Char('_'))
        return none;

    const QString core = name.mid(kPrefixLen);
    CombineShortcutKey key = none;

    for (int i = 0; i < CSK_NUM; i++)
    {
        if (core == s_combineShortcutKeyName_1[static_cast<CombineShortcutKey>(i)])
        {
            key = static_cast<CombineShortcutKey>(i);
            break;
        }
    }

    return key;
}

CnEnSwitchShortcutKey Settings::convert_name_to_cnEnSwitchShortcutKey(const QString name)
{
    CnEnSwitchShortcutKey key = CESSK_NUM;

    for (int i = 0; i < CESSK_NUM; i++)
    {
        if (name == s_cnEnSwitchShortcutKeyName[static_cast<CnEnSwitchShortcutKey>(i)])
        {
            key = static_cast<CnEnSwitchShortcutKey>(i);
            break;
        }
    }

    return key;
}

QString Settings::get_singleShortcutKey_name(SingleShortcutKey key)
{
    return s_singleShortcutKeyName.value(key);
}

QString Settings::get_singleShortcutKey_name_1(SingleShortcutKey key)
{
    return s_singleShortcutKeyName_1.value(key);
}

QString Settings::get_customShortcutFunc_shortcutkey_name(CustomShortcutFunction func)
{
    CombineShortcutKey key = get_customShortcutFunc_shortcutkey(func);
    QString str = s_combineShortcutKeyName[key];
    if (str == "禁止")
    {
        str = "无";
    }
    else
    {
        str.insert(0, "CTRL+");
    }

    return str;
}

bool Settings::switch_group_mode()
{
    s_isShowAllGroupBuf = (s_isShowAllGroupBuf == true) ? false : true;

    return s_isShowAllGroupBuf;
}

bool Settings::is_show_all_group()
{
    return s_isShowAllGroupBuf;
}

/************************************** 常用选项设置 *********************************/

void Settings::set_codeRemind_flg(bool enabled)
{
    s_commonBuf.codeRemind = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_codeRemind_flg()
{
    return s_commonBuf.codeRemind;
}

void Settings::set_spaceFullWhenCharHalf_flg(bool enabled)
{
    s_commonBuf.spaceFullWhenCharHalf = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_spaceFullWhenCharHalf_flg()
{
    return s_commonBuf.spaceFullWhenCharHalf;
}

void Settings::set_wordThink_flg(bool enabled)
{
    s_commonBuf.wordThink = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_wordThink_flg()
{
    return s_commonBuf.wordThink;
}

void Settings::set_smartMark_flg(bool enabled)
{
    s_commonBuf.smartMark = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_smartMark_flg()
{
    return s_commonBuf.smartMark;
}

void Settings::set_remindExistWord_flg(bool enabled)
{
    s_commonBuf.remindExistWord = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_remindExistWord_flg()
{
    return s_commonBuf.remindExistWord;
}

void Settings::set_alertWhenEmptyCode_flg(bool enabled)
{
    s_commonBuf.alertWhenEmptyCode = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_alertWhenEmptyCode_flg()
{
    return s_commonBuf.alertWhenEmptyCode;
}

void Settings::set_useAudioFile_flg(bool enabled)
{
    s_commonBuf.useAudioFile = enabled;
    s_settingDataIsChanged = true;
}

bool Settings::get_useAudioFile_flg()
{
    return s_commonBuf.useAudioFile;
}

void Settings::set_autoAdjustFreq_flg(bool enabled)
{
    s_commonBuf.autoAdjustFreq = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_autoAdjustFreq_flg()
{
    return s_commonBuf.autoAdjustFreq;
}

/************************************** 高级设置 *********************************/

void Settings::set_shiftCommitChar_flg(bool enabled)
{
    s_advanceBuf.shiftCommitChar = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_shiftCommitChar_flg()
{
    return s_advanceBuf.shiftCommitChar;
}

void Settings::set_inputStatistic_flg(bool enabled)
{
    s_advanceBuf.inputStatistic = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_inputStatistic_flg()
{
    return s_advanceBuf.inputStatistic;
}

void Settings::set_typeEffect_flg(bool enabled)
{
    s_advanceBuf.typeEffect = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_typeEffect_flg()
{
    return s_advanceBuf.typeEffect;
}

void Settings::set_recodeCalib_flg(bool enabled)
{
    s_advanceBuf.recodeCalib = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_recodetCalib_flg()
{
    return s_advanceBuf.recodeCalib;
}

void Settings::set_autoWordGroupOpt_flg(AutoWordGroupOpt opt)
{
    s_advanceBuf.autoWordGroupOpt = opt;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

AutoWordGroupOpt Settings::get_autoWordGroupOpt_flg()
{
    return s_advanceBuf.autoWordGroupOpt;
}

/************************************** 其他选项设置 *********************************/

void Settings::set_autoToEn_str(const QString &str) // 自动转英文的字符串
{
    s_othersBuf.autoToEnStr = QString(str).trimmed();
    ;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

const QString &Settings::get_autoToEn_str()
{
    return s_othersBuf.autoToEnStr;
}

void Settings::set_autoToHalf_mark(const QString &str) // 数字后自动转半角的标点
{
    s_othersBuf.autoToHalfMark = QString(str).trimmed();
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

const QString &Settings::get_autoToHalf_mark()
{
    return s_othersBuf.autoToHalfMark;
}

void Settings::set_autoToHalfMark_flg(bool flg) // 数字后自动转半角标点
{
    s_othersBuf.autoToHalfMarkFlg = flg;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_autoToHalfMark_flg()
{
    return s_othersBuf.autoToHalfMarkFlg;
}

/************************************** 工具条与输入面板界面设置 *********************************/

const QString &Settings::get_cur_skin_id()
{
    return s_uiBuf.curSkinId;
}

void Settings::set_cur_skin_id(const QString &skinId)
{
    s_uiBuf.curSkinId = skinId;
    s_settingDataIsChanged = true;
}

bool Settings::get_toolbar_auto_locate_flg()
{
    return s_uiBuf.toolbarAutoLocate;
}

void Settings::set_toolbar_auto_locate_flg(bool enabled)
{
    s_uiBuf.toolbarAutoLocate = enabled;
    s_settingDataIsChanged = true;
}

bool Settings::get_toolbar_auto_expand_flg()
{
    return s_uiBuf.toolbarAutoExpand;
}

void Settings::set_toolbar_auto_expand_flg(bool enabled)
{
    s_uiBuf.toolbarAutoExpand = enabled;
    s_settingDataIsChanged = true;
}

bool Settings::get_ui_audio_effect_flg()
{
    return s_uiBuf.enableUiAudioEffect;
}

void Settings::set_ui_audio_effect_flg(bool enabled)
{
    s_uiBuf.enableUiAudioEffect = enabled;
    s_settingDataIsChanged = true;
}

bool Settings::get_show_realtime_help_flg()
{
    return s_uiBuf.showRealtimeHelp;
}

void Settings::set_show_realtime_help_flg(bool enabled)
{
    s_uiBuf.showRealtimeHelp = enabled;
    s_settingDataIsChanged = true;
}

bool Settings::get_hide_toolbar_flg()
{
    return s_uiBuf.hideToolbar;
}

void Settings::set_hide_toolbar_flg(bool enabled)
{
    s_uiBuf.hideToolbar = enabled;
    s_settingDataIsChanged = true;
}

int Settings::get_toolbar_transparency()
{
    return s_uiBuf.toolbarTransparency;
}

void Settings::set_toolbar_transparency(int trans)
{
    s_uiBuf.toolbarTransparency = trans;
    s_settingDataIsChanged = true;
}

/************************************** 快捷键界面设置 *********************************/

// 恢复所有快捷键默认配置，该函数对临时配置变量进行读写
void Settings::restore_default_shortcutkey()
{
    s_shortcutKeyBuf.sskTmpEnglish = SSK_NONE;
    s_shortcutKeyBuf.sskShortcutInput = SSK_NONE;
    s_shortcutKeyBuf.sskTmpPinyin = SSK_BACK_QUOTE;

    s_shortcutKeyBuf.customShortcutFunc[CSF_BACK_FIND_CODE] = CSK_SLASH;         // 反查编码
    s_shortcutKeyBuf.customShortcutFunc[CSF_ONLINE_ADD_WORD] = CSK_EQUAL;        // 在线加词
    s_shortcutKeyBuf.customShortcutFunc[CSF_ONLINE_DEL_WORD] = CSK_DASH;         // 在线删词
    s_shortcutKeyBuf.customShortcutFunc[CSF_SWITCH_KEYBOARD] = CSK_ESC;          // 切换软键盘
    s_shortcutKeyBuf.customShortcutFunc[CSF_SWITCH_CHAR_SET] = CSK_m;            // 切换字符集
    s_shortcutKeyBuf.customShortcutFunc[CSF_SWITCH_INPUT_MODE] = CSK_BACK_SLASH; // 切换输入模式
    s_shortcutKeyBuf.customShortcutFunc[CSF_SWITCH_WORD_STATE] = CSK_INSERT;     // 切换字词状态
    s_shortcutKeyBuf.customShortcutFunc[CSF_SWITCH_S_IN_T_OUT] = CSK_j;          // 切换简入繁出

    s_shortcutKeyBuf.customShortcutFunc[CSF_SETUP_OPTION] = CSK_COMMA;              // 系统设置
    s_shortcutKeyBuf.customShortcutFunc[CSF_SHOW_HIDE_STATUS_BAR] = CSK_LEFT;       // 显示/隐藏状态栏
    s_shortcutKeyBuf.customShortcutFunc[CSF_SHOW_HIDE_CANDIDATE_WIN] = CSK_RIGHT;   // 显示/隐藏候选窗
    s_shortcutKeyBuf.customShortcutFunc[CSF_SWITCH_WORD_LEXICON] = CSK_QUOTE;       // 切换词库
    s_shortcutKeyBuf.customShortcutFunc[CSF_ADD_CHAR_AFTER_OUTPUT] = CSK_NONE;      // 输出项后加字符
    s_shortcutKeyBuf.customShortcutFunc[CSF_SWITCH_SKIN] = CSK_NONE;                // 切换皮肤
    s_shortcutKeyBuf.customShortcutFunc[CSF_QUICK_DEL_SCREEN_CHAR] = CSK_BACKSPACE; // 快删上屏项
    s_shortcutKeyBuf.customShortcutFunc[CSF_MARK_AUTO_PAIR] = CSK_DEL;              // 标点自动配对

    s_shortcutKeyBuf.ceSwitchShortcutkey = CESSK_SHIFT; // 中英文切换

    s_shortcutKeyBuf.disableAllCsk = false;
    s_shortcutKeyBuf.disableFullHalfSwitchShortcutkey = false;

    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

void Settings::set_customShortcutFunc_shortcutkey(CustomShortcutFunction func, CombineShortcutKey key)
{
    Q_ASSERT(func < CSF_NUM);
    s_shortcutKeyBuf.customShortcutFunc[func] = key;

    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

CombineShortcutKey Settings::get_customShortcutFunc_shortcutkey(CustomShortcutFunction func)
{
    Q_ASSERT(func < CSF_NUM);
    return s_shortcutKeyBuf.customShortcutFunc[func];
}

void Settings::set_tmp_english_shortcutkey(const QString &keyName)
{
    for (int i = 0; i < SSK_NUM; i++)
    {
        if (s_singleShortcutKeyName[static_cast<SingleShortcutKey>(i)] == keyName)
        {
            s_shortcutKeyBuf.sskTmpEnglish = static_cast<SingleShortcutKey>(i);
            s_settingDataIsChanged = true;
            s_needNotifyFcitx = true;
            break;
        }
    }
}

SingleShortcutKey Settings::get_tmp_english_shortcutkey()
{
    return s_shortcutKeyBuf.sskTmpEnglish;
}

void Settings::set_shortcut_input_shortcutkey(const QString &keyName)
{
    for (int i = 0; i < SSK_NUM; i++)
    {
        if (s_singleShortcutKeyName[static_cast<SingleShortcutKey>(i)] == keyName)
        {
            s_shortcutKeyBuf.sskShortcutInput = static_cast<SingleShortcutKey>(i);
            s_settingDataIsChanged = true;
            s_needNotifyFcitx = true;
            break;
        }
    }
}

SingleShortcutKey Settings::get_quick_input_shortcutkey()
{
    return s_shortcutKeyBuf.sskShortcutInput;
}

void Settings::set_tmp_pinyin_shortcutkey(const QString &keyName)
{
    for (int i = 0; i < SSK_NUM; i++)
    {
        if (s_singleShortcutKeyName[static_cast<SingleShortcutKey>(i)] == keyName)
        {
            s_shortcutKeyBuf.sskTmpPinyin = static_cast<SingleShortcutKey>(i);
            s_settingDataIsChanged = true;
            s_needNotifyFcitx = true;
            break;
        }
    }
}

SingleShortcutKey Settings::get_tmp_pinyin_shortcutkey()
{
    return s_shortcutKeyBuf.sskTmpPinyin;
}

void Settings::set_ceSwitchShortcutkey_shortcutkey(CnEnSwitchShortcutKey key)
{
    s_shortcutKeyBuf.ceSwitchShortcutkey = key;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

CnEnSwitchShortcutKey Settings::get_ceSwitchShortcutkey_shortcutkey()
{
    return s_shortcutKeyBuf.ceSwitchShortcutkey;
}

void Settings::set_disableFullHalfSwitchShortcutkey_flg(bool enabled)
{
    s_shortcutKeyBuf.disableFullHalfSwitchShortcutkey = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_disableFullHalfSwitchShortcutkey_flg()
{
    return s_shortcutKeyBuf.disableFullHalfSwitchShortcutkey;
}

void Settings::set_disableAllCsk_flg(bool enabled)
{
    s_shortcutKeyBuf.disableAllCsk = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

bool Settings::get_disableAllCsk_flg()
{
    return s_shortcutKeyBuf.disableAllCsk;
}

/************************************** 候选窗界面设置 *********************************/

CandiWinDispMode Settings::get_candidate_win_disp_mode()
{
    return s_candidateWinUiBuf.candiWinDispMode;
}

bool Settings::get_show_cand_dict()
{
    return s_candidateWinUi.show_cand_dict;
}

void Settings::set_show_cand_dict(bool chk)
{
    s_candidateWinUiBuf.show_cand_dict = chk;
    s_candidateWinUi.show_cand_dict = chk;
}

void Settings::set_candidate_win_disp_mode(CandiWinDispMode candiWinDispMode)
{
    s_candidateWinUiBuf.candiWinDispMode = candiWinDispMode;
    s_settingDataIsChanged = true;
}

char Settings::get_separate_char()
{
    return s_candidateWinUiBuf.separateChar;
}

void Settings::set_separate_char(char ch)
{
    s_candidateWinUiBuf.separateChar = ch;
    s_settingDataIsChanged = true;
}

bool Settings::get_use_gradient_color_flg()
{
    return s_candidateWinUiBuf.useGradientColor;
}

void Settings::set_use_gradient_color_flg(bool enabled)
{
    s_candidateWinUiBuf.useGradientColor = enabled;
    s_settingDataIsChanged = true;
}

bool Settings::get_use_bg_image_flg()
{
    return s_candidateWinUiBuf.useBgImage;
}

void Settings::set_use_bg_image_flg(bool enabled)
{
    s_candidateWinUiBuf.useBgImage = enabled;
    s_settingDataIsChanged = true;
}

int Settings::get_radius()
{
    return s_candidateWinUiBuf.radius;
}

void Settings::set_radius(int radius)
{
    s_candidateWinUiBuf.radius = radius;
    s_settingDataIsChanged = true;
}

int Settings::get_candi_win_transparency()
{
    return s_candidateWinUiBuf.candiWinTransparency;
}

void Settings::set_candi_win_transparency(int trans)
{
    s_candidateWinUiBuf.candiWinTransparency = trans;
    s_settingDataIsChanged = true;
}

int Settings::get_candidate_word_count()
{
    return s_candidateWinUiBuf.candiWordCount;
}

void Settings::set_candidate_word_count(int candiWordCount)
{
    s_candidateWinUiBuf.candiWordCount = candiWordCount;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

int Settings::get_candi_char_count()
{
    return s_candidateWinUiBuf.candiCharCount;
}

void Settings::set_candi_char_count(int count)
{
    s_candidateWinUiBuf.candiCharCount = count;
    s_settingDataIsChanged = true;
}

const QFont &Settings::get_candidate_text_font()
{
    return s_candidateWinUiBuf.candiTextFont;
}

void Settings::set_candidate_text_font(const QFont &font)
{
    s_candidateWinUiBuf.candiTextFont = font;
    s_settingDataIsChanged = true;
}

QColor Settings::get_bg_color()
{
    return s_candidateWinUiBuf.bgColor;
}

void Settings::set_bg_color(const QColor &color)
{
    s_candidateWinUiBuf.bgColor = color;
    s_settingDataIsChanged = true;
}

const QColor &Settings::get_gradient_color0()
{
    return s_candidateWinUiBuf.gradientColor0;
}

void Settings::set_gradient_color0(const QColor &color)
{
    s_candidateWinUiBuf.gradientColor0 = color;
    s_settingDataIsChanged = true;
}

const QColor &Settings::get_gradient_color1()
{
    return s_candidateWinUiBuf.gradientColor1;
}

void Settings::set_gradient_color1(const QColor &color)
{
    s_candidateWinUiBuf.gradientColor1 = color;
    s_settingDataIsChanged = true;
}

QColor Settings::get_border_color()
{
    return s_candidateWinUiBuf.borderColor;
}

void Settings::set_border_color(const QColor &color)
{
    s_candidateWinUiBuf.borderColor = color;
    s_settingDataIsChanged = true;
}

const QString &Settings::get_bg_image()
{
    return s_candidateWinUiBuf.bgImage;
}

void Settings::set_bg_image(const QString &imagePath)
{
    s_candidateWinUiBuf.bgImage = imagePath;
    s_settingDataIsChanged = true;
}

bool Settings::get_bg_image_tiled_flg()
{
    return s_candidateWinUiBuf.enablebgImageTiled;
}

void Settings::set_bg_image_tiled_flg(bool enabled)
{
    s_candidateWinUiBuf.enablebgImageTiled = enabled;
    s_settingDataIsChanged = true;
}

void Settings::set_candidate_word_text_color(const QColor &color)
{
    s_candidateWinUiBuf.candiWordTextColor = color;
    s_settingDataIsChanged = true;
}

const QColor &Settings::get_candidate_word_text_color()
{
    return s_candidateWinUiBuf.candiWordTextColor;
}

void Settings::set_candidate_prompt_text_color(const QColor &color)
{
    s_candidateWinUiBuf.candiPromptTextColor = color;
    s_settingDataIsChanged = true;
}

const QColor &Settings::get_candidate_prompt_text_color()
{
    return s_candidateWinUiBuf.candiPromptTextColor;
}

/************************************** 候选窗选项设置 *********************************/

RcodeSelectShortcutKey Settings::get_recode_select_key()
{
    return s_candidateWinOptionBuf.recodeSelectKey;
}

void Settings::set_recode_select_key(RcodeSelectShortcutKey key)
{
    s_candidateWinOptionBuf.recodeSelectKey = key;
    if (key == RSSK_NONE)
    {
        s_candidateWinOptionBuf.secondRecodeKey = "KEY_NONE";
        s_candidateWinOptionBuf.thirdRecodeKey = "KEY_NONE";
    }
    else if (key == RSSK_SEMI_QUOTE)
    {
        s_candidateWinOptionBuf.secondRecodeKey = "KEY_SEMICOLON";
        s_candidateWinOptionBuf.thirdRecodeKey = "KEY_QUOTE";
    }
    else if (key == RSSK_COMMA_PERIOD)
    {
        s_candidateWinOptionBuf.secondRecodeKey = "KEY_COMMA";
        s_candidateWinOptionBuf.thirdRecodeKey = "KEY_PERIOD";
    }
    else if (key == RSSK_CTRL)
    {
        s_candidateWinOptionBuf.secondRecodeKey = "KEY_LEFT_CTRL";
        s_candidateWinOptionBuf.thirdRecodeKey = "KEY_RIGHT_CTRL";
    }
    else if (key == RSSK_SHIFT)
    {
        s_candidateWinOptionBuf.secondRecodeKey = "KEY_LEFT_SHIFT";
        s_candidateWinOptionBuf.thirdRecodeKey = "KEY_RIGHT_SHIFT";
    }

    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

// char Settings::get_second_recode_key()
//{
//     return s_candidateWinOptionBuf.secondRecodeKey;
// }

// void Settings::set_second_recode_key( char key )
//{
//     s_candidateWinOptionBuf.secondRecodeKey = key;
//     s_settingDataIsChanged = true;
//     s_needNotifyFcitx = true;
// }

// char Settings::get_third_recode_key()
//{
//     return s_candidateWinOptionBuf.thirdRecodeKey;
// }

// void Settings::set_third_recode_key( char key )
//{
//     s_candidateWinOptionBuf.thirdRecodeKey = key;
//     s_settingDataIsChanged = true;
//     s_needNotifyFcitx = true;
// }

CandiPageShortcutKey Settings::get_candi_page_key()
{
    return s_candidateWinOptionBuf.candiPageKey;
}

void Settings::set_candi_page_key(CandiPageShortcutKey key)
{
    s_candidateWinOptionBuf.candiPageKey = key;
    if (key == CPSK_SUB_EQUAL)
    {
        s_candidateWinOptionBuf.prevPageKey = "-";
        s_candidateWinOptionBuf.nextPageKey = "=";
    }
    else if (key == CPSK_COMMA_PERIOD)
    {
        s_candidateWinOptionBuf.prevPageKey = ",";
        s_candidateWinOptionBuf.nextPageKey = ".";
    }
    else if (key == CPSK_UP_DWON)
    {
        s_candidateWinOptionBuf.prevPageKey = "KEY_UP";
        s_candidateWinOptionBuf.nextPageKey = "KEY_DOWN";
    }
    else if (key == CPSK_PAGE_UP_DWON)
    {
        s_candidateWinOptionBuf.prevPageKey = "KEY_PAGE_UP";
        s_candidateWinOptionBuf.nextPageKey = "KEY_PAGE_DWON";
    }

    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

// char Settings::get_prevPage_key()
//{
//     return s_candidateWinOptionBuf.prevPageKey;
// }

// void Settings::set_prevPage_key( char key )
//{
//     s_candidateWinOptionBuf.prevPageKey = key;
// }

// char Settings::get_nextPage_key()
//{
//     return s_candidateWinOptionBuf.nextPageKey;
// }

// void Settings::set_nextPage_key( char key )
//{
//     s_candidateWinOptionBuf.nextPageKey = key;
// }

bool Settings::get_cursor_follow_flg()
{
    return s_candidateWinOptionBuf.cursorFollowFlg;
}

void Settings::set_candiWin_follow_flg(bool enabled)
{
    s_candidateWinOptionBuf.cursorFollowFlg = enabled;
    s_settingDataIsChanged = true;
}

bool Settings::get_hide_candiWin_flg()
{
    return s_candidateWinOptionBuf.hideCandiWinFlg;
}

void Settings::set_hide_candiWin_flg(bool enabled)
{
    s_candidateWinOptionBuf.hideCandiWinFlg = enabled;
    s_settingDataIsChanged = true;
}

bool Settings::get_show_op_remind_info_flg()
{
    return s_candidateWinOptionBuf.showOpRemindInfoFlg;
}

void Settings::set_show_op_remind_info_flg(bool enabled)
{
    s_candidateWinOptionBuf.showOpRemindInfoFlg = enabled;
    s_settingDataIsChanged = true;
}

bool Settings::get_shift_select_recode_flg()
{
    return s_candidateWinOptionBuf.shiftSelectRecodeFlg;
}

void Settings::set_shift_select_recode_flg(bool enabled)
{
    s_candidateWinOptionBuf.shiftSelectRecodeFlg = enabled;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}

/************************************** 自定义键盘设置 *********************************/

// 恢复所有自定义键盘字符与标点，该函数对临时配置变量进行读写，只在恢复所有默认配置时被调用
void Settings::restore_default_customkey()
{
    s_customKeyBuf.customKeyValue[KEY_0].commChar = "ˉ";
    s_customKeyBuf.customKeyValue[KEY_0].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_0].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_0].shiftMark = "）";

    s_customKeyBuf.customKeyValue[KEY_1].commChar = "，";
    s_customKeyBuf.customKeyValue[KEY_1].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_1].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_1].shiftMark = "！";

    s_customKeyBuf.customKeyValue[KEY_2].commChar = "、";
    s_customKeyBuf.customKeyValue[KEY_2].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_2].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_2].shiftMark = "·";

    s_customKeyBuf.customKeyValue[KEY_3].commChar = "；";
    s_customKeyBuf.customKeyValue[KEY_3].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_3].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_3].shiftMark = "#";

    s_customKeyBuf.customKeyValue[KEY_4].commChar = "：";
    s_customKeyBuf.customKeyValue[KEY_4].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_4].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_4].shiftMark = "￥";

    s_customKeyBuf.customKeyValue[KEY_5].commChar = "？";
    s_customKeyBuf.customKeyValue[KEY_5].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_5].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_5].shiftMark = "%";

    s_customKeyBuf.customKeyValue[KEY_6].commChar = "！";
    s_customKeyBuf.customKeyValue[KEY_6].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_6].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_6].shiftMark = "…";

    s_customKeyBuf.customKeyValue[KEY_7].commChar = "…";
    s_customKeyBuf.customKeyValue[KEY_7].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_7].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_7].shiftMark = "—";

    s_customKeyBuf.customKeyValue[KEY_8].commChar = "—";
    s_customKeyBuf.customKeyValue[KEY_8].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_8].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_8].shiftMark = "*";

    s_customKeyBuf.customKeyValue[KEY_9].commChar = "•";
    s_customKeyBuf.customKeyValue[KEY_9].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_9].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_9].shiftMark = "（";

    s_customKeyBuf.customKeyValue[KEY_A].commChar = "〔";
    s_customKeyBuf.customKeyValue[KEY_A].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_A].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_A].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_B].commChar = "（";
    s_customKeyBuf.customKeyValue[KEY_B].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_B].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_B].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_C].commChar = "【";
    s_customKeyBuf.customKeyValue[KEY_C].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_C].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_C].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_D].commChar = "〈";
    s_customKeyBuf.customKeyValue[KEY_D].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_D].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_D].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_E].commChar = "“";
    s_customKeyBuf.customKeyValue[KEY_E].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_E].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_E].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_F].commChar = "〉";
    s_customKeyBuf.customKeyValue[KEY_F].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_F].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_F].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_G].commChar = "《";
    s_customKeyBuf.customKeyValue[KEY_G].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_G].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_G].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_H].commChar = "》";
    s_customKeyBuf.customKeyValue[KEY_H].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_H].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_H].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_I].commChar = "∶";
    s_customKeyBuf.customKeyValue[KEY_I].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_I].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_I].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_J].commChar = "「";
    s_customKeyBuf.customKeyValue[KEY_J].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_J].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_J].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_K].commChar = "」";
    s_customKeyBuf.customKeyValue[KEY_K].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_K].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_K].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_L].commChar = "『";
    s_customKeyBuf.customKeyValue[KEY_L].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_L].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_L].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_M].commChar = "［";
    s_customKeyBuf.customKeyValue[KEY_M].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_M].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_M].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_N].commChar = "）";
    s_customKeyBuf.customKeyValue[KEY_N].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_N].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_N].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_O].commChar = "＂";
    s_customKeyBuf.customKeyValue[KEY_O].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_O].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_O].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_P].commChar = "＇";
    s_customKeyBuf.customKeyValue[KEY_P].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_P].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_P].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_Q].commChar = "‘";
    s_customKeyBuf.customKeyValue[KEY_Q].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_Q].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_Q].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_R].commChar = "”";
    s_customKeyBuf.customKeyValue[KEY_R].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_R].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_R].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_S].commChar = "〕";
    s_customKeyBuf.customKeyValue[KEY_S].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_S].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_S].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_T].commChar = "々";
    s_customKeyBuf.customKeyValue[KEY_T].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_T].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_T].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_U].commChar = "‖";
    s_customKeyBuf.customKeyValue[KEY_U].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_U].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_U].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_V].commChar = "】";
    s_customKeyBuf.customKeyValue[KEY_V].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_V].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_V].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_W].commChar = "’";
    s_customKeyBuf.customKeyValue[KEY_W].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_W].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_W].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_X].commChar = "〗";
    s_customKeyBuf.customKeyValue[KEY_X].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_X].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_X].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_Y].commChar = "～";
    s_customKeyBuf.customKeyValue[KEY_Y].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_Y].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_Y].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_Z].commChar = "〖";
    s_customKeyBuf.customKeyValue[KEY_Z].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_Z].commMark = "";
    s_customKeyBuf.customKeyValue[KEY_Z].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_BACKQUOTE].commChar = "。";
    s_customKeyBuf.customKeyValue[KEY_BACKQUOTE].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_BACKQUOTE].commMark = "`";
    s_customKeyBuf.customKeyValue[KEY_BACKQUOTE].shiftMark = "~";

    s_customKeyBuf.customKeyValue[KEY_SUB].commChar = "ˇ";
    s_customKeyBuf.customKeyValue[KEY_SUB].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_SUB].commMark = "－";
    s_customKeyBuf.customKeyValue[KEY_SUB].shiftMark = "—";

    s_customKeyBuf.customKeyValue[KEY_EQUAL].commChar = "¨";
    s_customKeyBuf.customKeyValue[KEY_EQUAL].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_EQUAL].commMark = "＝";
    s_customKeyBuf.customKeyValue[KEY_EQUAL].shiftMark = "+";

    s_customKeyBuf.customKeyValue[KEY_LEFT_BRACKET].commChar = "`";
    s_customKeyBuf.customKeyValue[KEY_LEFT_BRACKET].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_LEFT_BRACKET].commMark = "[";
    s_customKeyBuf.customKeyValue[KEY_LEFT_BRACKET].shiftMark = "{";

    s_customKeyBuf.customKeyValue[KEY_RIGHT_BRACKET].commChar = "|";
    s_customKeyBuf.customKeyValue[KEY_RIGHT_BRACKET].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_RIGHT_BRACKET].commMark = "]";
    s_customKeyBuf.customKeyValue[KEY_RIGHT_BRACKET].shiftMark = "}";

    s_customKeyBuf.customKeyValue[KEY_BACKSLASH].commChar = "\"";
    s_customKeyBuf.customKeyValue[KEY_BACKSLASH].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_BACKSLASH].commMark = "、";
    s_customKeyBuf.customKeyValue[KEY_BACKSLASH].shiftMark = "｜";

    s_customKeyBuf.customKeyValue[KEY_SEMICOLON].commChar = "』";
    s_customKeyBuf.customKeyValue[KEY_SEMICOLON].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_SEMICOLON].commMark = "；";
    s_customKeyBuf.customKeyValue[KEY_SEMICOLON].shiftMark = "：";

    s_customKeyBuf.customKeyValue[KEY_QUOTE].commChar = ".";
    s_customKeyBuf.customKeyValue[KEY_QUOTE].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_QUOTE].commMark = "’";
    s_customKeyBuf.customKeyValue[KEY_QUOTE].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_COMMA].commChar = "］";
    s_customKeyBuf.customKeyValue[KEY_COMMA].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_COMMA].commMark = "，";
    s_customKeyBuf.customKeyValue[KEY_COMMA].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_PERIOD].commChar = "中";
    s_customKeyBuf.customKeyValue[KEY_PERIOD].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_PERIOD].commMark = "。";
    s_customKeyBuf.customKeyValue[KEY_PERIOD].shiftMark = "";

    s_customKeyBuf.customKeyValue[KEY_SLASH].commChar = "国";
    s_customKeyBuf.customKeyValue[KEY_SLASH].shiftChar = "";
    s_customKeyBuf.customKeyValue[KEY_SLASH].commMark = "、";
    s_customKeyBuf.customKeyValue[KEY_SLASH].shiftMark = "？";
}

QString Settings::get_custom_char_value()
{
    QString value;

    for (int i = 0; i < KEY_SYMBOL_NUM; i++)
    {
        value += s_customKey.customKeyValue[i].commChar + " ";
        value += s_customKey.customKeyValue[i].shiftChar + " ";
    }

    return value;
}

// 读取配置文件时被调用
void Settings::set_custom_char_value(const QString &value)
{
    QStringList valueList = value.split(" ");
    int len = valueList.size();

    for (int i = 0; i < KEY_SYMBOL_NUM; i++)
    {
        if ((2 * i + 1) < len)
        {
            s_customKey.customKeyValue[i].commChar = valueList.at(i * 2);
            s_customKey.customKeyValue[i].shiftChar = valueList.at(i * 2 + 1);
        }
    }
}

QString Settings::get_custom_mark_value()
{
    QString value;

    for (int i = 0; i < KEY_SYMBOL_NUM; i++)
    {
        value += s_customKey.customKeyValue[i].commMark + " ";
        value += s_customKey.customKeyValue[i].shiftMark + " ";
    }

    return value;
}

// 读取配置文件时被调用
void Settings::set_custom_mark_value(const QString &value)
{
    QStringList valueList = value.split(" ");
    int len = valueList.size();

    for (int i = 0; i < KEY_SYMBOL_NUM; i++)
    {
        if ((2 * i + 1) < len)
        {
            s_customKey.customKeyValue[i].commMark = valueList.at(i * 2);
            s_customKey.customKeyValue[i].shiftMark = valueList.at(i * 2 + 1);
        }
    }
}

const CustomKeyValue &Settings::get_custom_key_info_(SymbolKeyIdx keyIdx) // 用于软键盘输入模式
{
    Q_ASSERT(keyIdx < KEY_SYMBOL_NUM);
    return s_customKey.customKeyValue[keyIdx];
}

const CustomKeyValue &Settings::get_custom_key_info(SymbolKeyIdx keyIdx)
{
    Q_ASSERT(keyIdx < KEY_SYMBOL_NUM);
    return s_customKeyBuf.customKeyValue[keyIdx];
}

void Settings::set_custom_key_info(SymbolKeyIdx keyIdx, const CustomKeyValue &keyValue)
{
    Q_ASSERT(keyIdx < KEY_SYMBOL_NUM);
    s_customKeyBuf.customKeyValue[keyIdx] = keyValue;
    s_settingDataIsChanged = true;
    s_needNotifyFcitx = true;
}
