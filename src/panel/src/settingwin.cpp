#include "settingwin.h"

#include <array>
#include <vector>

#include <QDateTime>
#include <QFont>

#include "config.h"
#include "inputwin.h"
#include "key.h"
#include "settings.h"
#include "settingshelper.h"
#include "ui_settingwin.h"

namespace
{
QColor swQColorFromSpec(const std::string &s)
{
    return QColor(toQStringUtf8(s));
}

struct UiSessionState
{
    bool useAudioFile = true;
    bool showAllGroup = false;
};

UiSessionState g_uiSessionState;

bool useAudioFile()
{
    return g_uiSessionState.useAudioFile;
}

void setUseAudioFile(bool enabled)
{
    g_uiSessionState.useAudioFile = enabled;
}

bool toggleShowAllGroup()
{
    g_uiSessionState.showAllGroup = !g_uiSessionState.showAllGroup;
    return g_uiSessionState.showAllGroup;
}

bool showAllGroup()
{
    return g_uiSessionState.showAllGroup;
}

/* KEY_* token → 可读 name。空 / "KEY_NONE" 返回 emptyLabel（下拉展示传"禁止"/"无"等占位
 * 文字；默认 {} 表示不展示）；未知 token（解析失败）返回 {} 让调用方过滤。 */
QString keyTokenName(const std::string &token, const QString &emptyLabel = {})
{
    if (token.empty() || token == "KEY_NONE")
        return emptyLabel;
    const FreewbKeySym sym = freewb::Key::keySymFromUniqueName(token.c_str());
    if (sym == FreewbKey_None)
        return {};
    return QString::fromUtf8(freewb::Key::keySymToName(sym));
}

QString customShortcutItemLabel(const std::string &value)
{
    const char *keyTok = freewb::Key::readKeyString(value.c_str());
    return keyTokenName(keyTok ? keyTok : std::string{}, QStringLiteral("无"));
}

/* 单键选择码（临时英文/快捷输入/临时拼音等）允许的 KEY_* token 候选列表；
 * INI 直接落 token 字符串（"KEY_NONE" 表示禁用）。首项固定 "KEY_NONE"。 */
const std::vector<std::string> &singleShortcutCandidates()
{
    static const std::vector<std::string> k = {
        "KEY_NONE",          "KEY_SEMICOLON",  "KEY_QUOTE", "KEY_COMMA", "KEY_PERIOD", "KEY_BACKQUOTE", "KEY_LEFT_BRACKET",
        "KEY_RIGHT_BRACKET", "KEY_BACK_SLASH", "KEY_SLASH", "KEY_u",     "KEY_i",      "KEY_v",         "KEY_z",
    };
    return k;
}

/* 自定义功能键（Ctrl+第二键）允许的 KEY_* 第二键 token 候选列表；
 * INI 以 "CTRL+<KEY_X>" 形式落盘，"KEY_NONE" 表示未配置。首项固定 "KEY_NONE"。 */
const std::vector<std::string> &customShortcutCandidates()
{
    static const std::vector<std::string> k = {
        "KEY_NONE",
        "KEY_INSERT",
        "KEY_DEL",
        "KEY_ESC",
        "KEY_BACKSPACE",
        "KEY_HOME",
        "KEY_END",
        "KEY_LEFT",
        "KEY_RIGHT",
        "KEY_UP",
        "KEY_DOWN",
        "KEY_QUOTE",
        "KEY_SEMICOLON",
        "KEY_BACK_SLASH",
        "KEY_LEFT_BRACKET",
        "KEY_RIGHT_BRACKET",
        "KEY_COMMA",
        "KEY_PERIOD",
        "KEY_SLASH",
        "KEY_BACKQUOTE",
        "KEY_EQUAL",
        "KEY_DASH",
        "KEY_F1",
        "KEY_F2",
        "KEY_F3",
        "KEY_F4",
        "KEY_F5",
        "KEY_F6",
        "KEY_F7",
        "KEY_F8",
        "KEY_F9",
        "KEY_F10",
        "KEY_F11",
        "KEY_F12",
        "KEY_A",
        "KEY_B",
        "KEY_C",
        "KEY_D",
        "KEY_E",
        "KEY_F",
        "KEY_G",
        "KEY_H",
        "KEY_I",
        "KEY_J",
        "KEY_K",
        "KEY_L",
        "KEY_M",
        "KEY_N",
        "KEY_O",
        "KEY_P",
        "KEY_Q",
        "KEY_R",
        "KEY_S",
        "KEY_T",
        "KEY_U",
        "KEY_V",
        "KEY_W",
        "KEY_X",
        "KEY_Y",
        "KEY_Z",
    };
    return k;
}

using CustomShortcutGetter = const std::string &(settings::Settings::*)() const;
using CustomShortcutSetter = void (settings::Settings::*)(const std::string &);

struct CustomShortcutAccessor
{
    CustomShortcutGetter getter;
    CustomShortcutSetter setter;
};

/* 14 项自定义功能键 accessor 表；顺序与 settingwin.ui 中 cmbFunction 下拉项严格对应。
 * 如需增减或改序，必须同步调整 .ui 中的条目顺序。 */
const std::array<CustomShortcutAccessor, 14> kCustomShortcutAccessors = {{
    {&settings::Settings::get_backFindCode, &settings::Settings::set_backFindCode},             // 反查编码
    {&settings::Settings::get_onlineAddWord, &settings::Settings::set_onlineAddWord},           // 在线加词
    {&settings::Settings::get_onlineDelWord, &settings::Settings::set_onlineDelWord},           // 在线删词
    {&settings::Settings::get_switchVKb, &settings::Settings::set_switchVKb},                   // 切换软键盘
    {&settings::Settings::get_switchCharSet, &settings::Settings::set_switchCharSet},           // 切换字符集
    {&settings::Settings::get_switchInputMode, &settings::Settings::set_switchInputMode},       // 切换输入模式
    {&settings::Settings::get_switchChttrans, &settings::Settings::set_switchChttrans},         // 切换简入繁出
    {&settings::Settings::get_setupOption, &settings::Settings::set_setupOption},               // 打开系统设置
    {&settings::Settings::get_showHideToolbar, &settings::Settings::set_showHideToolbar},       // 显/隐状态栏
    {&settings::Settings::get_showHideCandiWin, &settings::Settings::set_showHideCandiWin},     // 显/隐候选窗
    {&settings::Settings::get_switchLexicon, &settings::Settings::set_switchLexicon},           // 切换词库
    {&settings::Settings::get_switchSkin, &settings::Settings::set_switchSkin},                 // 切换皮肤
    {&settings::Settings::get_quickDelScreenItem, &settings::Settings::set_quickDelScreenItem}, // 快删上屏项
    {&settings::Settings::get_markAutoPair, &settings::Settings::set_markAutoPair},             // 标点自动配对
}};

const CustomShortcutAccessor *customShortcutAccessor(int funcIndex)
{
    if (funcIndex < 0 || static_cast<size_t>(funcIndex) >= kCustomShortcutAccessors.size())
        return nullptr;
    return &kCustomShortcutAccessors[static_cast<size_t>(funcIndex)];
}

int customShortcutCount()
{
    return static_cast<int>(kCustomShortcutAccessors.size());
}

std::string customShortcutGetValue(const settings::Settings &cfg, int funcIndex)
{
    const CustomShortcutAccessor *accessor = customShortcutAccessor(funcIndex);
    if (!accessor || !accessor->getter)
        return "CTRL+KEY_NONE";
    return (cfg.*(accessor->getter))();
}

void customShortcutSetValue(settings::Settings &cfg, int funcIndex, const std::string &value)
{
    const CustomShortcutAccessor *accessor = customShortcutAccessor(funcIndex);
    if (!accessor || !accessor->setter)
        return;
    (cfg.*(accessor->setter))(value);
}

void swBuildSingleShortcutCombo(QComboBox *combo, const std::string &selected, const std::string &forbiddenA,
                                const std::string &forbiddenB)
{
    combo->clear();
    const auto &candidates = singleShortcutCandidates();
    for (const auto &token : candidates)
    {
        if (token != "KEY_NONE" && (token == forbiddenA || token == forbiddenB))
            continue;
        combo->addItem(keyTokenName(token, QStringLiteral("禁止")), QString::fromStdString(token));
    }

    std::string target = selected;
    if (target != "KEY_NONE" && (target == forbiddenA || target == forbiddenB))
        target = "KEY_NONE";

    const int pos = combo->findData(QString::fromStdString(target));
    combo->setCurrentIndex(pos >= 0 ? pos : 0);
}

/* 中英切换的"KEY_SHIFT"/"KEY_CTRL" 聚合项等价覆盖左右两个物理键；单键/空返回自身。 */
std::pair<std::string, std::string> cnEnSwitchCoverKeys(const std::string &token)
{
    if (token == "KEY_SHIFT")
        return {"KEY_LEFT_SHIFT", "KEY_RIGHT_SHIFT"};
    if (token == "KEY_CTRL")
        return {"KEY_LEFT_CTRL", "KEY_RIGHT_CTRL"};
    return {token, std::string()};
}

bool keyTokenHit(const std::string &x, const std::string &a, const std::string &b)
{
    if (x.empty() || x == "KEY_NONE")
        return false;
    return x == a || x == b;
}

bool pairIntersects(const std::string &a1, const std::string &a2, const std::string &b1, const std::string &b2)
{
    return keyTokenHit(b1, a1, a2) || keyTokenHit(b2, a1, a2);
}

/* 键对预设通用结构：两个 KEY_* token，显示时合成 "name1/name2"。*/
struct KeyPairPreset
{
    const char *first;
    const char *second;
};

/* 键对预设 → 下拉显示文本。二者皆 KEY_NONE 时统一显示 "无"。 */
QString keyPairPresetLabel(const KeyPairPreset &preset)
{
    const std::string a = preset.first ? preset.first : "";
    const std::string b = preset.second ? preset.second : "";
    if ((a.empty() || a == "KEY_NONE") && (b.empty() || b == "KEY_NONE"))
        return QStringLiteral("无");
    const QString first = keyTokenName(a);
    const QString second = keyTokenName(b);
    if (first.isEmpty() || second.isEmpty())
        return {};
    return QStringLiteral("%1/%2").arg(first, second);
}

/* 二三重码选择键：UI 下拉 ↔ 物理键对互查；INI 对应secondRecodeKey/thirdRecodeKey。 */
const std::array<KeyPairPreset, 5> &recodeSelectPresets()
{
    static const std::array<KeyPairPreset, 5> k = {{
        {"KEY_NONE", "KEY_NONE"},
        {"KEY_SEMICOLON", "KEY_QUOTE"},
        {"KEY_COMMA", "KEY_PERIOD"},
        {"KEY_LEFT_CTRL", "KEY_RIGHT_CTRL"},
        {"KEY_LEFT_SHIFT", "KEY_RIGHT_SHIFT"},
    }};
    return k;
}

int recodeSelectPresetIndex(const std::string &second, const std::string &third)
{
    const auto &presets = recodeSelectPresets();
    for (size_t i = 0; i < presets.size(); ++i)
    {
        if (second == presets[i].first && third == presets[i].second)
            return static_cast<int>(i);
    }
    return -1;
}

/* 候选上下翻页键：UI 下拉 ↔ 物理键对互查；INI 对应prevPageKey/nextPageKey。 */
const std::array<KeyPairPreset, 4> &candiPagePresets()
{
    static const std::array<KeyPairPreset, 4> k = {{
        {"KEY_DASH", "KEY_EQUAL"},
        {"KEY_COMMA", "KEY_PERIOD"},
        {"KEY_UP", "KEY_DOWN"},
        {"KEY_PAGE_UP", "KEY_PAGE_DOWN"},
    }};
    return k;
}

int candiPagePresetIndex(const std::string &prev, const std::string &next)
{
    const auto &presets = candiPagePresets();
    for (size_t i = 0; i < presets.size(); ++i)
    {
        if (prev == presets[i].first && next == presets[i].second)
            return static_cast<int>(i);
    }
    return -1;
}

/* 以预设表为数据源填充下拉：每项 text 来自 keyPairPresetLabel，
 * userData 存"预设索引"，activated 槽凭 userData 回查。
 */
template <typename Preset, size_t N>
void swBuildKeyPairCombo(QComboBox *combo, const std::array<Preset, N> &presets, int currentIndex)
{
    combo->clear();
    for (size_t i = 0; i < presets.size(); ++i)
    {
        combo->addItem(keyPairPresetLabel(presets[i]), static_cast<int>(i));
    }
    combo->setCurrentIndex(currentIndex);
}

} // namespace

// 设置窗口样式表
#define QSS_FILE ":/qss/settingwin.qss"

#define QSS_TOOL_TIPS                                                                                                            \
    "color: rgb(56, 56, 56);"                                                                                                    \
    "font: 12pt \"Ubuntu\";"                                                                                                     \
    "padding: 10px;"                                                                                                             \
    "background-color: rgb(254, 255, 226);"                                                                                      \
    "border-radius: 5px;"                                                                                                        \
    "border-width: 2px;"                                                                                                         \
    "border-style: solid;"                                                                                                       \
    "border-color: rgb(200, 200, 200);"

#define QSS_BORDER_ACTIVE "color: rgb(255, 255, 255);background-color: rgb(10, 120, 203);"
#define QSS_BORDER_DEACTIVE "color: rgb(0, 0, 0);background-color: rgb(200, 200, 200);"

#define QSS_BTN_CLOSE0 "border-image: url(:/image/setting/close0.png);"
#define QSS_BTN_CLOSE1 "border-image: url(:/image/setting/close1.png);"
#define QSS_BTN_CLOSE2 "border-image: url(:/image/setting/close2.png);"

// 设置组子分组图标
#define ICO_SETTING_GROUP ":/image/setting/group.png"

SettingWin::SettingWin(QWidget *parent) : QWidget(parent), ui(new Ui::SettingWin)
{
    ui->setupUi(this);

    init_member_data(); // 确保相关的数据成员初始化完成后再初始化界面外观
    init_mouse_hover_tips();
    init_window_appearance();
    install_evt_filter();

    move(m_defaultPopPosition);

    /////////////////////////
    // ui->labelBugFeedback->hide();
    ui->ckbInputStatistic->hide();
    ui->label_37->hide();
    ui->label_38->hide();
    ui->label_40->hide();
    ui->label_41->hide();
    ui->ledt2ndRecode->hide();
    ui->ledt3rdRecode->hide();
    ui->ledtPrecPage->hide();
    ui->ledtNextPage->hide();
    ui->ckbShiftSelectRecode->hide();
    ui->ckbAutoLocate->hide();
    ui->cmbWhenLossLocation->hide();
    ui->labelLossLocate->hide();
}

SettingWin::~SettingWin()
{
    delete ui;

    if (m_kbCustomKeyChar)
    {
        delete m_kbCustomKeyChar;
        m_kbCustomKeyChar = nullptr;
    }

    if (m_kbCustomKeyMark)
    {
        delete m_kbCustomKeyMark;
        m_kbCustomKeyMark = nullptr;
    }

    if (m_customKeyDialog)
    {
        delete m_customKeyDialog;
        m_customKeyDialog = nullptr;
    }
}

void SettingWin::init_window_appearance()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);

    setWindowIcon(QIcon(":/image/setting/logo.png"));
    setWindowTitle(_("settings"));
    // setFont(freewb_candi_text_qfont(settings::instance()));

    ui->labelVersionNum->setText(FREEWB_VERSION);
    ui->labelVersion->setText(_("freewb"));

    // 载入窗口全局UI样式表
    QFile qssFile(QSS_FILE);
    if (!qssFile.open(QFile::ReadOnly))
    {
        qWarning() << "open qss file failed!";
    }
    else
    {
        this->setStyleSheet(qssFile.readAll());
        qssFile.close();
    }
}

void SettingWin::init_member_data()
{
    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();

    QDesktopWidget *d = QApplication::desktop();
    m_defaultPopPosition = QPoint((d->width() - size().width()) / 2, (d->height() - size().height()) / 2);

    // 初始化设置界面的软键盘
    m_kbCustomKeyChar = new Keyboard(VKM_CUSTOM_CHAR, ui->pageCustomKeyChar);
    m_kbCustomKeyChar->move(40, 120);

    m_kbCustomKeyMark = new Keyboard(VKM_CUSTOM_MARK, ui->pageCustomKeyMark);
    m_kbCustomKeyMark->move(40, 120);

    // 自定义软键盘点击
    connect(m_kbCustomKeyChar, SIGNAL(signal_custom_key_clicked(SymbolKeyIdx, const QString &, const CustomKeyValue &)), this,
            SLOT(slot_custom_keyboard_char_clicked(SymbolKeyIdx, const QString &, const CustomKeyValue &)));
    connect(m_kbCustomKeyMark, SIGNAL(signal_custom_key_clicked(SymbolKeyIdx, const QString &, const CustomKeyValue &)), this,
            SLOT(slot_custom_keyboard_mark_clicked(SymbolKeyIdx, const QString &, const CustomKeyValue &)));

    m_customKeyDialog = new CustomKeyDialog();
    connect(m_customKeyDialog, SIGNAL(signal_custom_ok_btn_clicked(const QString &, const QString &)), this,
            SLOT(slot_custom_btn_ok_clicked(const QString &, const QString &)));
}

void SettingWin::init_mouse_hover_tips()
{
#define TIPS_FILE INSTALL_DIR + "/data/setting_tips.txt"

    m_tooltipsWin.setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    m_tooltipsWin.setAttribute(Qt::WA_TranslucentBackground);
    m_tooltipsLabel = new QLabel(&m_tooltipsWin);
    m_tooltipsLabel->setStyleSheet(QSS_TOOL_TIPS);

    QFile tipsFile(TIPS_FILE);
    QTextStream textStream(&tipsFile);
    if (!tipsFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << TIPS_FILE << "open failed!";
        return;
    }

    QMap<int, QString> tipsMap;
    QStringList textList = textStream.readAll().split('\n');
    tipsFile.close();
    foreach(QString line, textList)
    {
        if (line.startsWith("1"))
        {
            // QStringList tips = line.split( '=' );
            QStringList tips;
            tips << line.left(4);
            tips << line.mid(5);
            if (tips.length() > 1)
            {
                QString value = tips.at(1);
                tipsMap.insert(tips.at(0).toInt(), value.replace("\\n", "\n"));
            }
        }
    }

    m_tipsTextMap.insert(ui->ckbCodeRemind, tipsMap.value(1001));
    m_tipsTextMap.insert(ui->ckbSpaceFullWhenCharHalf, tipsMap.value(1002));
    m_tipsTextMap.insert(ui->ckbWordThink, tipsMap.value(1003));
    m_tipsTextMap.insert(ui->ckbSmartMark, tipsMap.value(1004));
    m_tipsTextMap.insert(ui->ckbRemindExistWord, tipsMap.value(1005));
    m_tipsTextMap.insert(ui->ckbAlertWhenEmptyCode, tipsMap.value(1006));
    m_tipsTextMap.insert(ui->ckbUseAudioFile, tipsMap.value(1007));
    m_tipsTextMap.insert(ui->ckbAutoAdjustFreq, tipsMap.value(1008));

    m_tipsTextMap.insert(ui->ckbShiftCommitChar, tipsMap.value(1101));
    m_tipsTextMap.insert(ui->ckbInputStatistic, tipsMap.value(1102));
    m_tipsTextMap.insert(ui->ckbTypeEffect, tipsMap.value(1103));
    m_tipsTextMap.insert(ui->ckbRepeatCalib, tipsMap.value(1104));
    m_tipsTextMap.insert(ui->ckbAutoWordGroup, tipsMap.value(1105));

    m_tipsTextMap.insert(ui->ledtAutoToEnStr, tipsMap.value(1201));
    m_tipsTextMap.insert(ui->ledtAutoToHalf, tipsMap.value(1202));

    m_tipsTextMap.insert(ui->cmbSkinSelect, tipsMap.value(1301));
    m_tipsTextMap.insert(ui->ckbAutoLocate, tipsMap.value(1302));
    m_tipsTextMap.insert(ui->cmbWhenLossLocation, tipsMap.value(1303));
    m_tipsTextMap.insert(ui->ckbAutoExtend, tipsMap.value(1304));
    m_tipsTextMap.insert(ui->ckbEnableUiAudioEffect, tipsMap.value(1305));
    m_tipsTextMap.insert(ui->ckbDispRealHelp, tipsMap.value(1306));
    m_tipsTextMap.insert(ui->ckbHideToolbar, tipsMap.value(1307));
    m_tipsTextMap.insert(ui->spbToolbarTransparency, tipsMap.value(1308));

    m_tipsTextMap.insert(ui->cmbCandiWinMode, tipsMap.value(1401));
    m_tipsTextMap.insert(ui->ledtSeparateChar, tipsMap.value(1402));
    m_tipsTextMap.insert(ui->ckbUseGradientBgColor, tipsMap.value(1403));
    m_tipsTextMap.insert(ui->ckbUseBgImage, tipsMap.value(1404));
    m_tipsTextMap.insert(ui->ckbUseTile, tipsMap.value(1405));
    m_tipsTextMap.insert(ui->spbCornerRadian, tipsMap.value(1406));
    m_tipsTextMap.insert(ui->spbCandiTransparency, tipsMap.value(1407));
    m_tipsTextMap.insert(ui->spbCandiItemNum, tipsMap.value(1408));
    m_tipsTextMap.insert(ui->spbCandiCharNum, tipsMap.value(1409));

    m_tipsTextMap.insert(ui->btnCandiFont, tipsMap.value(1410));
    m_tipsTextMap.insert(ui->btnCandiBg, tipsMap.value(1411));
    m_tipsTextMap.insert(ui->btnCandiBgColor0, tipsMap.value(1412));
    m_tipsTextMap.insert(ui->btnCandiBgColor1, tipsMap.value(1413));
    m_tipsTextMap.insert(ui->btnCandiBorderColor, tipsMap.value(1414));
    m_tipsTextMap.insert(ui->btnCandiAutoWord, tipsMap.value(1415));
    m_tipsTextMap.insert(ui->btnCandiPrompt, tipsMap.value(1416));

    m_tipsTextMap.insert(ui->ledt2ndRecode, tipsMap.value(1501));
    m_tipsTextMap.insert(ui->ledt3rdRecode, tipsMap.value(1502));
    m_tipsTextMap.insert(ui->ckbCursorFollow, tipsMap.value(1503));
    m_tipsTextMap.insert(ui->ckbHideCandiChinese, tipsMap.value(1504));
    m_tipsTextMap.insert(ui->ckbDispOpPrompt, tipsMap.value(1505));
    m_tipsTextMap.insert(ui->ckbShiftSelectRecode, tipsMap.value(1506));
    m_tipsTextMap.insert(ui->ledtPrecPage, tipsMap.value(1507));
    m_tipsTextMap.insert(ui->ledtNextPage, tipsMap.value(1508));
    m_tipsTextMap.insert(ui->cmb23RecodeSelect, tipsMap.value(1509));
    m_tipsTextMap.insert(ui->cmbPrevNextPage, tipsMap.value(1510));
    m_tipsTextMap.insert(ui->ckbDispOpDict, tipsMap.value(1511));

    m_tipsTextMap.insert(ui->cmbFunction, tipsMap.value(1601));
    m_tipsTextMap.insert(ui->cmbShortcutKey, tipsMap.value(1602));
    m_tipsTextMap.insert(ui->ckbDisableAllShortcutKey, tipsMap.value(1603));
    m_tipsTextMap.insert(ui->ckbDisableFullHalfKey, tipsMap.value(1604));
    m_tipsTextMap.insert(ui->cmbTmpEnglish, tipsMap.value(1605));
    m_tipsTextMap.insert(ui->cmbShortcutInput, tipsMap.value(1606));
    m_tipsTextMap.insert(ui->cmbTmpPinyin, tipsMap.value(1607));
    m_tipsTextMap.insert(ui->cmbSwitchCnEn, tipsMap.value(1608));
}

void SettingWin::install_evt_filter()
{
    installEventFilter(this);

#if 0
//    QList<QWidget *> widgets = findChildren<QWidget *>();
//    foreach( QWidget *widget, widgets )
//    {
//        widget->installEventFilter( this );
//    }
#else
    ui->ckbCodeRemind->installEventFilter(this);
    ui->ckbSpaceFullWhenCharHalf->installEventFilter(this);
    ui->ckbWordThink->installEventFilter(this);
    ui->ckbSmartMark->installEventFilter(this);
    ui->ckbRemindExistWord->installEventFilter(this);
    ui->ckbAlertWhenEmptyCode->installEventFilter(this);
    ui->ckbUseAudioFile->installEventFilter(this);
    ui->ckbAutoAdjustFreq->installEventFilter(this);

    ui->ckbShiftCommitChar->installEventFilter(this);
    ui->ckbInputStatistic->installEventFilter(this);
    ui->ckbTypeEffect->installEventFilter(this);
    ui->ckbRepeatCalib->installEventFilter(this);
    ui->ckbAutoWordGroup->installEventFilter(this);

    ui->ledtAutoToEnStr->installEventFilter(this);
    ui->ledtAutoToHalf->installEventFilter(this);

    ui->cmbSkinSelect->installEventFilter(this);
    ui->ckbAutoLocate->installEventFilter(this);
    ui->cmbWhenLossLocation->installEventFilter(this);
    ui->ckbAutoExtend->installEventFilter(this);
    ui->ckbEnableUiAudioEffect->installEventFilter(this);
    ui->ckbDispRealHelp->installEventFilter(this);
    ui->ckbHideToolbar->installEventFilter(this);
    ui->spbToolbarTransparency->installEventFilter(this);

    ui->cmbCandiWinMode->installEventFilter(this);
    ui->ledtSeparateChar->installEventFilter(this);
    ui->ckbUseGradientBgColor->installEventFilter(this);
    ui->ckbUseBgImage->installEventFilter(this);
    ui->ckbUseTile->installEventFilter(this);
    ui->spbCornerRadian->installEventFilter(this);
    ui->spbCandiTransparency->installEventFilter(this);
    ui->spbCandiItemNum->installEventFilter(this);
    ui->spbCandiCharNum->installEventFilter(this);
    ui->cmb23RecodeSelect->installEventFilter(this);
    ui->cmbPrevNextPage->installEventFilter(this);

    ui->btnCandiFont->installEventFilter(this);
    ui->btnCandiBg->installEventFilter(this);
    ui->btnCandiBgColor0->installEventFilter(this);
    ui->btnCandiBgColor1->installEventFilter(this);
    ui->btnCandiBorderColor->installEventFilter(this);
    ui->btnCandiAutoWord->installEventFilter(this);
    ui->btnCandiPrompt->installEventFilter(this);

    ui->ledt2ndRecode->installEventFilter(this);
    ui->ledt3rdRecode->installEventFilter(this);
    ui->ckbCursorFollow->installEventFilter(this);
    ui->ckbHideCandiChinese->installEventFilter(this);
    ui->ckbDispOpPrompt->installEventFilter(this);
    ui->ckbDispOpDict->installEventFilter(this);

    ui->ckbShiftSelectRecode->installEventFilter(this);
    ui->ledtPrecPage->installEventFilter(this);
    ui->ledtNextPage->installEventFilter(this);

    ui->cmbFunction->installEventFilter(this);
    ui->cmbShortcutKey->installEventFilter(this);
    ui->ckbDisableAllShortcutKey->installEventFilter(this);
    ui->ckbDisableFullHalfKey->installEventFilter(this);
    ui->cmbTmpEnglish->installEventFilter(this);
    ui->cmbShortcutInput->installEventFilter(this);
    ui->cmbTmpPinyin->installEventFilter(this);
    ui->cmbSwitchCnEn->installEventFilter(this);
#endif
}

void SettingWin::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }

    QWidget::mousePressEvent(event);
}

void SettingWin::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void SettingWin::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseIsPressed)
    {
        QPoint mouseCurrPosition = event->globalPos();
        move(pos() + mouseCurrPosition - m_mouseLastPosition);
        m_mouseLastPosition = mouseCurrPosition;
    }

    QWidget::mouseMoveEvent(event);
}

bool SettingWin::eventFilter(QObject *obj, QEvent *event)
{
    bool isProcessed = false;

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvt = static_cast<QKeyEvent *>(event);
        if (keyEvt->key() == Qt::Key_Escape)
            close();
    }
    else if (event->type() == QEvent::ToolTip)
    {
        // qDebug() << obj->objectName();
        if (m_tipsTextMap.contains(qobject_cast<QWidget *>(obj)))
        {
            QString tips = m_tipsTextMap.value(qobject_cast<QWidget *>(obj));
            if (!tips.isEmpty())
            {
                m_tooltipsLabel->setText(tips);
                m_tooltipsLabel->adjustSize();
                m_tooltipsWin.adjustSize();
                m_tooltipsWin.move(QCursor::pos().x() + 10, QCursor::pos().y() + 10);
                m_tooltipsWin.show();
                m_tooltipsWinShowFlg = true;
            }
        }
    }
    else if (event->type() == QEvent::Leave && m_tooltipsWinShowFlg)
    {
        m_tooltipsWin.hide();
        m_tooltipsWinShowFlg = false;
    }

    if (isProcessed == false)
    {
        return QWidget::eventFilter(obj, event);
    }

    return isProcessed;
}

void SettingWin::slot_open_win()
{
    slot_init_all_setting_page();

    show();

    activateWindow();

    ui->listWidget->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(0);
}

void SettingWin::slot_init_all_setting_page()
{
    update_listwidget_item();

    init_common_page();
    init_advance_page();
    init_others_page();
    init_shortcutkey_page();
    init_ui_setting_page();
    init_candidate_ui_page();
    init_candidate_option_page();
}

void SettingWin::slot_show_version_info()
{
    slot_init_all_setting_page();
    show();
    activateWindow();

    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        if (ui->listWidget->item(i)->text() == _("Version information"))
        {
            ui->listWidget->setCurrentRow(i);
        }
    }
    ui->stackedWidget->setCurrentWidget(ui->pageVersionInfo);
}

// 更新设置界面左侧的设置选项组
void SettingWin::update_listwidget_item()
{
    ui->listWidget->clear();

    m_listItemCommon = new QListWidgetItem(_("Common options"), ui->listWidget);
    m_listItemAdvance = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), _("Advanced options"), ui->listWidget);
    m_listItemOthers = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), _("Other settings"), ui->listWidget);

    if (showAllGroup())
    {
        m_listItemUi = new QListWidgetItem(_("Interface settings"), ui->listWidget);
        m_listItemCandidateWinUi = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), _("Candidate window interface"), ui->listWidget);
        m_listItemCandidateWinOption =
            new QListWidgetItem(QIcon(ICO_SETTING_GROUP), _("Candidate window options"), ui->listWidget);
    }

    m_listItemShortcutKey = new QListWidgetItem(_("Setting shortcut keys"), ui->listWidget);
    m_listItemCustomKeyChar = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), _("Define soft keyboard"), ui->listWidget);
    m_listItemCustomKeyMark = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), _("Custom punctuation"), ui->listWidget);
    m_listItemVersionInfo = new QListWidgetItem(_("Version information"), ui->listWidget);
    // m_listItemBug = new QListWidgetItem( "问题反馈", ui->listWidget );

    ui->stackedWidget->setCurrentWidget(ui->pageCommon);
    ui->listWidget->setCurrentRow(0);
}

// 初始化常用选项设置页面
void SettingWin::init_common_page()
{
    ui->ckbCodeRemind->setChecked(settings::instance().get_codeRemind());
    ui->ckbSpaceFullWhenCharHalf->setChecked(settings::instance().get_spaceFullWhenCharHalf());
    ui->ckbWordThink->setChecked(settings::instance().get_wordThink());
    ui->ckbSmartMark->setChecked(settings::instance().get_smartMark());
    ui->ckbRemindExistWord->setChecked(settings::instance().get_remindExistWord());
    ui->ckbAlertWhenEmptyCode->setChecked(settings::instance().get_alertWhenEmptyCode());
    ui->ckbUseAudioFile->setChecked(useAudioFile());
    ui->labelUseAudioFile->hide();
    ui->ckbUseAudioFile->hide(); ///////////////
    ui->ckbAutoAdjustFreq->setChecked(settings::instance().get_autoAdjustFreq());
}

// 初始化高级设置页面
void SettingWin::init_advance_page()
{
    ui->ckbShiftCommitChar->setChecked(settings::instance().get_shiftCommitChar());
    ui->ckbInputStatistic->setChecked(settings::instance().get_inputStatistic());
    ui->ckbTypeEffect->setChecked(settings::instance().get_typeEffect());
    ui->ckbRepeatCalib->setChecked(settings::instance().get_recodeCalib());

    AutoWordGroupOpt opt = static_cast<AutoWordGroupOpt>(settings::instance().get_autoWordGroupOpt());
    if (opt == AWGO_FORBID)
    {
        ui->ckbAutoWordGroup->setCurrentIndex(0);
    }
    else if (opt == AWGO_LOSS)
    {
        ui->ckbAutoWordGroup->setCurrentIndex(1);
    }
    else if (opt == AWGO_SAVE)
    {
        ui->ckbAutoWordGroup->setCurrentIndex(2);
    }
}

// 初始化其它选项设置页面
void SettingWin::init_others_page()
{
    ui->ledtAutoToEnStr->setText(toQStringUtf8(settings::instance().get_autoToEnStr()));
    // ui->ledtAutoToHalf->setText( settings::instance().get_CoustomMark() );
    ui->ledtAutoToHalf->hide();
    ui->ckbAutoHalfMarkAfterNum->setChecked(settings::instance().get_autoToHalfMarkFlg());
}

// 初始化快捷键设置页
void SettingWin::init_shortcutkey_page()
{
    update_custom_shortkey_cmb();
    update_tmp_engish_cmb();
    update_short_input_cmb();
    update_tmp_pinyin_cmb();

    ui->ckbDisableFullHalfKey->setChecked(settings::instance().get_disableFullHalfSwitch());
    ui->ckbDisableAllShortcutKey->setChecked(settings::instance().get_disableAllShortcutKey());
    // 该项依赖 fcitx 全局配置，不再由本页提供设置
    ui->labelCnEn->hide();
    ui->cmbSwitchCnEn->hide();
}

// 设置界面--更新自定义功能快捷键选择框
void SettingWin::update_custom_shortkey_cmb()
{
    ui->cmbShortcutKey->clear();
    const int curFunc = ui->cmbFunction->currentIndex();
    const std::string currentValue = customShortcutGetValue(settings::instance(), curFunc);
    const auto &candidates = customShortcutCandidates();
    for (const auto &tok : candidates)
    {
        const std::string value = std::string("CTRL+") + tok;
        /* 同一组合键只允许绑到一个功能上，排除其他功能已占用的项（KEY_NONE 不受此约束）。 */
        if (tok != "KEY_NONE")
        {
            bool used = false;
            const int funcCount = customShortcutCount();
            for (int j = 0; j < funcCount; ++j)
            {
                if (j == curFunc)
                    continue;
                if (customShortcutGetValue(settings::instance(), j) == value)
                {
                    used = true;
                    break;
                }
            }
            if (used)
                continue;
        }
        const QString label = customShortcutItemLabel(value);
        if (label.isEmpty())
            continue;
        ui->cmbShortcutKey->addItem(label, QString::fromStdString(value));
    }

    const int pos = ui->cmbShortcutKey->findData(QString::fromStdString(currentValue));
    ui->cmbShortcutKey->setCurrentIndex(pos >= 0 ? pos : 0);
}

// 设置界面--更新临时英文选项框
void SettingWin::update_tmp_engish_cmb()
{
    swBuildSingleShortcutCombo(ui->cmbTmpEnglish, settings::instance().get_tempEnglish(),
                               settings::instance().get_shortcutInput(), settings::instance().get_tempPinyin());
}

// 设置界面--更新快捷输入选项框
void SettingWin::update_short_input_cmb()
{
    swBuildSingleShortcutCombo(ui->cmbShortcutInput, settings::instance().get_shortcutInput(),
                               settings::instance().get_tempEnglish(), settings::instance().get_tempPinyin());
}

// 设置界面--更新临时拼音选项框
void SettingWin::update_tmp_pinyin_cmb()
{
    swBuildSingleShortcutCombo(ui->cmbTmpPinyin, settings::instance().get_tempPinyin(), settings::instance().get_tempEnglish(),
                               settings::instance().get_shortcutInput());
}

void SettingWin::init_ui_setting_page()
{
    init_skin_select_cmb();
    ui->ckbAutoLocate->setChecked(settings::instance().get_toolbarAutoLocate());
    ui->ckbAutoExtend->setChecked(settings::instance().get_toolbarAutoExpand());
    ui->ckbEnableUiAudioEffect->setChecked(settings::instance().get_uiAudioEffect());
    ui->ckbDispRealHelp->setChecked(settings::instance().get_showRealtimeHelp());
    ui->ckbHideToolbar->setChecked(settings::instance().get_hideToolbar());
    ui->ckbDispRealHelp->setChecked(settings::instance().get_showRealtimeHelp());
    ui->spbToolbarTransparency->setValue(settings::instance().get_toolbarTransparency());
    // ui->labelToolbar->setWindowOpacity( 1 - ui->spbToolbarTransparency->value()/100.0 ); ?无效
}

void SettingWin::init_skin_select_cmb()
{
    ui->cmbSkinSelect->clear();

    QString skinDir = INSTALL_DIR + "/skin/";
    QStringList dirList = QDir(skinDir).entryList(QDir::Dirs);
    dirList.removeOne(".");
    dirList.removeOne("..");

    QStringList skinIdList;
    foreach(QString skinId, dirList)
    {
        if (QFile(skinDir + skinId + "/skin.ini").exists())
        {
            skinIdList << skinId;
            ui->cmbSkinSelect->addItem(skinId);
        }
    }
    std::vector<std::string> skinVec;
    skinVec.reserve(static_cast<size_t>(skinIdList.size()));
    for (const QString &id : skinIdList)
        skinVec.push_back(fromStdUtf8(id));
    freewb_runtime_set_skin_list(skinVec);

    for (int i = 0; i < ui->cmbSkinSelect->count(); i++)
    {
        if (ui->cmbSkinSelect->itemText(i) == toQStringUtf8(settings::instance().get_curSkinId()))
        {
            ui->cmbSkinSelect->setCurrentIndex(i);
            break;
        }
    }

    update_toolbar_preview(ui->cmbSkinSelect->currentText());
}

void SettingWin::update_toolbar_preview(const QString &skinId)
{
    QString iamge = INSTALL_DIR + "/skin/" + skinId + "/toolbar.png";
    ui->labelToolbar->setStyleSheet(QString("border-image:url(%1);").arg(iamge));
}

void SettingWin::init_candidate_ui_page()
{
    if (static_cast<CandiWinDispMode>(settings::instance().get_candiWinDispMode()) == CWDM_ONE_ROW)
    {
        ui->cmbCandiWinMode->setCurrentIndex(0);
    }
    else if (static_cast<CandiWinDispMode>(settings::instance().get_candiWinDispMode()) == CWDM_MULTI_ROW)
    {
        ui->cmbCandiWinMode->setCurrentIndex(1);
    }
    const char separateChar = freewb_separate_char_from_string(settings::instance().get_separateChar());
    ui->ledtSeparateChar->setText(separateChar == 0 ? QString() : QString(QChar(separateChar)));

    ui->ckbUseGradientBgColor->setChecked(settings::instance().get_useGradientColor());
    ckb_useGradientBgColor_updated(settings::instance().get_useGradientColor());
    ui->ckbUseBgImage->setChecked(settings::instance().get_useBgImage());
    ckb_useBgImage_updated(settings::instance().get_useBgImage());
    ui->ckbUseTile->setChecked(settings::instance().get_enableTiled());
    if (settings::instance().get_curSkinId() == std::string("default"))
    {
        ui->spbCornerRadian->setEnabled(true);
        ui->ckbUseGradientBgColor->setEnabled(true);
        ui->ckbUseBgImage->setEnabled(true);
        if (settings::instance().get_useBgImage())
        {
            ui->ckbUseTile->setEnabled(true);
        }
    }
    else
    {
        ui->spbCornerRadian->setEnabled(false);
        ui->ckbUseGradientBgColor->setEnabled(false);
        ui->ckbUseBgImage->setEnabled(false);
        ui->ckbUseTile->setEnabled(false);
    }

    ui->spbCornerRadian->setValue(settings::instance().get_radius());
    ui->spbCandiTransparency->setValue(settings::instance().get_transparency());
    ui->spbCandiItemNum->setValue(settings::instance().get_candiWordCount());
    ui->spbCandiCharNum->setValue(settings::instance().get_candiCharCount());

    update_fram_candidate_win();
}

void SettingWin::ckb_useGradientBgColor_updated(bool checked)
{
    if (checked)
    {
        ui->ckbUseBgImage->setChecked(false);
        ui->ckbUseTile->setEnabled(false);
        ui->btnCandiBg->hide();
        ui->btnCandiBgColor0->show();
        ui->btnCandiBgColor1->show();
    }
    else if (!ui->ckbUseBgImage->isChecked())
    {
        ui->btnCandiBg->setText(_("Background color"));
        ui->btnCandiBg->show();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
    }
}

void SettingWin::ckb_useBgImage_updated(bool checked)
{
    if (checked)
    {
        ui->ckbUseGradientBgColor->setChecked(false);
        ui->ckbUseTile->setEnabled(true);
        ui->btnCandiBg->setText(_("Background image"));
        ui->btnCandiBg->show();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
    }
    else if (!ui->ckbUseGradientBgColor->isChecked())
    {
        ui->ckbUseTile->setEnabled(false);
        ui->btnCandiBg->setText(_("Background color"));
        ui->btnCandiBg->show();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
    }
}

void SettingWin::update_fram_candidate_win()
{
    QString style;

    if (settings::instance().get_curSkinId() == std::string("default"))
    {
        int boederRadius = settings::instance().get_radius();
        QColor boderColor = swQColorFromSpec(settings::instance().get_borderColor());
        QColor bgColor = swQColorFromSpec(settings::instance().get_bgColor());
        QColor gradienColor0 = swQColorFromSpec(settings::instance().get_gradientColor0());
        QColor gradienColor1 = swQColorFromSpec(settings::instance().get_gradientColor1());

        QString borderColorStyle =
            QString("border-color:rgb(%1,%2,%3);").arg(boderColor.red()).arg(boderColor.green()).arg(boderColor.blue());

        if (settings::instance().get_useGradientColor())
        {
            QString gradienColorStyle = QString("background-color:qlineargradient(spread:pad,x1:0, y1:0, x2:0, y2:1,stop:0 "
                                                "rgb(%1,%2,%3),stop:1 rgb(%4,%5,%6));")
                                            .arg(gradienColor0.red())
                                            .arg(gradienColor0.green())
                                            .arg(gradienColor0.blue())
                                            .arg(gradienColor1.red())
                                            .arg(gradienColor1.green())
                                            .arg(gradienColor1.blue());
            style = QString("#framCandidateWin{"
                            "border-width:1px;"
                            "border-style:solid;"
                            "border-radius:%1px;"
                            "%2"
                            "%3"
                            "}")
                        .arg(boederRadius)
                        .arg(borderColorStyle)
                        .arg(gradienColorStyle);
        }
        else if (settings::instance().get_useBgImage())
        {
            QString bgImageStyle = QString("%1:url(%2);")
                                       .arg(settings::instance().get_enableTiled() ? "background-image" : "border-image")
                                       .arg(toQStringUtf8(settings::instance().get_bgImage()));

            style = QString("#framCandidateWin{"
                            "border-width:1px;"
                            "border-style:solid;"
                            "border-radius:%1px;"
                            "%2"
                            "%3"
                            "}")
                        .arg(boederRadius)
                        .arg(borderColorStyle)
                        .arg(bgImageStyle);
        }
        else
        {
            style = QString("#framCandidateWin{"
                            "border-width:1px;"
                            "border-style:solid;"
                            "border-radius:%1px;"
                            "%2"
                            "background-color:rgb(%3,%4,%5);"
                            "}")
                        .arg(boederRadius)
                        .arg(borderColorStyle)
                        .arg(bgColor.red())
                        .arg(bgColor.green())
                        .arg(bgColor.blue());
        }

        if (settings::instance().get_useGradientColor())
        {
            ui->btnCandiBgColor0->show();
            ui->btnCandiBgColor1->show();
        }
        else
        {
            ui->btnCandiBg->show();
        }

        ui->btnCandiBorderColor->show();
    }
    else
    {
        ui->btnCandiBg->hide();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
        ui->btnCandiBorderColor->hide();
    }

    ui->framCandidateWin->setStyleSheet(style);

    QColor color = swQColorFromSpec(settings::instance().get_candiWordTextColor());
    ui->btnCandiAutoWord->setStyleSheet(QString("color:rgb(%1,%2,%3);").arg(color.red()).arg(color.green()).arg(color.blue()));

    color = swQColorFromSpec(settings::instance().get_candiPromptTextColor());
    ui->btnCandiPrompt->setStyleSheet(QString("color:rgb(%1,%2,%3);").arg(color.red()).arg(color.green()).arg(color.blue()));

    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::init_candidate_option_page()
{
    swBuildKeyPairCombo(ui->cmb23RecodeSelect, recodeSelectPresets(),
                        recodeSelectPresetIndex(settings::instance().get_secondRecodeKey(),
                                                settings::instance().get_thirdRecodeKey()));
    swBuildKeyPairCombo(ui->cmbPrevNextPage, candiPagePresets(),
                        candiPagePresetIndex(settings::instance().get_prevPageKey(), settings::instance().get_nextPageKey()));
    // ui->ledt2ndRecode->setText( QChar(settings::instance().get_second_recode_key()) );
    // ui->ledt3rdRecode->setText( QChar(settings::instance().get_third_recode_key()) );
    ui->ckbCursorFollow->setChecked(settings::instance().get_cursorFollow());
    ui->ckbHideCandiChinese->setChecked(settings::instance().get_hideCandiWin());
    ui->ckbDispOpPrompt->setChecked(settings::instance().get_showOpRemindInfo());
    ui->ckbDispOpDict->setChecked(settings::instance().get_showCandDictInfo());
    // ui->ledtPrecPage->setText( QChar(settings::instance().get_prevPage_key()) );
    // ui->ledtNextPage->setText( QChar(settings::instance().get_nextPage_key()) );
    ui->ckbShiftSelectRecode->setChecked(settings::instance().get_shiftSelectRecode());
}

// 切换设置界面右侧选项页
void SettingWin::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (current == m_listItemCommon)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageCommon);
    }
    else if (current == m_listItemAdvance)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageAdvance);
    }
    else if (current == m_listItemOthers)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageOthers);
    }
    else if (current == m_listItemUi)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageUi);
    }
    else if (current == m_listItemCandidateWinUi)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageCandidateWinUi);
    }
    else if (current == m_listItemCandidateWinOption)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageCandidateWinOption);
    }
    else if (current == m_listItemShortcutKey)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageShortcutKey);
    }
    else if (current == m_listItemCustomKeyChar)
    {
        m_kbCustomKeyChar->update_keyboard_button(); // 初始化虚拟键盘按钮上显示的自定义符号
        ui->stackedWidget->setCurrentWidget(ui->pageCustomKeyChar);
    }
    else if (current == m_listItemCustomKeyMark)
    {
        m_kbCustomKeyMark->update_keyboard_button(); // 初始化虚拟键盘按钮上显示的自定义符号
        ui->stackedWidget->setCurrentWidget(ui->pageCustomKeyMark);
    }
    else if (current == m_listItemVersionInfo)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageVersionInfo);
    }
    else if (current == m_listItemBug)
    {
        // QDesktopServices::openUrl( QUrl("http://www.freewb.org") );
    }
}

void SettingWin::on_btnSettingOption_clicked()
{
    toggleShowAllGroup();
    if (showAllGroup())
    {
        ui->btnSettingOption->setText(_("Display [Common] options"));
    }
    else
    {
        ui->btnSettingOption->setText(_("Display [All] options"));
    }

    update_listwidget_item();
}

void SettingWin::on_btnOk_clicked()
{
    close();
    move(m_defaultPopPosition);
    puts("win seting");
    settings::instance().save();
    g_settingsNotifier.notifySettingDataChangedToFcitx();
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_btnCancel_clicked()
{
    close();
    move(m_defaultPopPosition);
}

void SettingWin::on_btnHelp_clicked()
{
    system("firefox ~/.local/freewb/help/help.html > /dev/null 2>&1 &");
}

void SettingWin::on_ckbCodeRemind_stateChanged(int arg1)
{
    //    qDebug() << DBG_TRACE << arg1;

    if (arg1 == Qt::Checked)
    {
        settings::instance().set_codeRemind(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_codeRemind(false);
    }
}

void SettingWin::on_ckbSpaceFullWhenCharHalf_toggled(bool checked)
{
    settings::instance().set_spaceFullWhenCharHalf(checked);
}

void SettingWin::on_ckbWordThink_toggled(bool checked)
{
    settings::instance().set_wordThink(checked);
}

void SettingWin::on_ckbSmartMark_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        settings::instance().set_smartMark(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_smartMark(false);
    }
}

void SettingWin::on_ckbRemindExistWord_toggled(bool checked)
{
    settings::instance().set_remindExistWord(checked);
}

void SettingWin::on_ckbAlertWhenEmptyCode_toggled(bool checked)
{
    settings::instance().set_alertWhenEmptyCode(checked);
}

void SettingWin::on_ckbUseAudioFile_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        setUseAudioFile(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        setUseAudioFile(false);
    }
}

void SettingWin::on_ckbAutoAdjustFreq_toggled(bool checked)
{
    settings::instance().set_autoAdjustFreq(checked);
}

void SettingWin::on_ckbShiftCommitChar_toggled(bool checked)
{
    settings::instance().set_shiftCommitChar(checked);
}

void SettingWin::on_ckbInputStatistic_toggled(bool checked)
{
    settings::instance().set_inputStatistic(checked);
}

void SettingWin::on_ckbRepeatCalib_toggled(bool checked)
{
    settings::instance().set_recodeCalib(checked);
}

void SettingWin::on_ckbTypeEffect_toggled(bool checked)
{
    settings::instance().set_typeEffect(checked);
}

void SettingWin::on_ckbAutoWordGroup_activated(int index)
{
    if (index == 0)
    {
        settings::instance().set_autoWordGroupOpt(AWGO_LOSS);
    }
    else if (index == 1)
    {
        settings::instance().set_autoWordGroupOpt(AWGO_FORBID);
    }
    else if (index == 2)
    {
        settings::instance().set_autoWordGroupOpt(AWGO_SAVE);
    }
}

void SettingWin::on_ledtAutoToEnStr_textChanged(const QString &arg1)
{
    settings::instance().set_autoToEnStr(fromStdUtf8(arg1));
}

void SettingWin::on_ledtAutoToHalf_textChanged(const QString &arg1)
{
    settings::instance().set_CoustomMark(fromStdUtf8(arg1));
}

void SettingWin::on_ckbAutoHalfMarkAfterNum_toggled(bool checked)
{
    settings::instance().set_autoToHalfMarkFlg(checked);
}

void SettingWin::on_cmbFunction_activated(int index)
{
    Q_UNUSED(index)
    update_custom_shortkey_cmb();
}

void SettingWin::on_cmbShortcutKey_activated(int index)
{
    Q_UNUSED(index);
    const std::string value = ui->cmbShortcutKey->currentData().toString().toStdString();
    customShortcutSetValue(settings::instance(), ui->cmbFunction->currentIndex(), value);
}

void SettingWin::on_ckbDisableAllShortcutKey_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        settings::instance().set_disableAllShortcutKey(true);
        ui->cmbFunction->setEnabled(false);
        ui->cmbShortcutKey->setEnabled(false);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_disableAllShortcutKey(false);
        ui->cmbFunction->setEnabled(true);
        ui->cmbShortcutKey->setEnabled(true);
    }
}

void SettingWin::on_ckbDisableFullHalfKey_toggled(bool checked)
{
    settings::instance().set_disableFullHalfSwitch(checked);
}

void SettingWin::on_cmbSwitchCnEn_activated(int index)
{
    const auto &presets = freewb_cn_en_switch_presets();
    if (index < 0 || static_cast<size_t>(index) >= presets.size())
        return;
    const auto cover = cnEnSwitchCoverKeys(presets[static_cast<size_t>(index)].token);
    const std::string sec = settings::instance().get_secondRecodeKey();
    const std::string thi = settings::instance().get_thirdRecodeKey();

    if (pairIntersects(cover.first, cover.second, sec, thi))
    {
        m_msgBox = new QMessageBox(this);
        m_msgBox->setIcon(QMessageBox::Warning);
        m_msgBox->setText(
            _("The shortcut key you set will conflict with the second and third recode selection key, confirm setting?"));
        m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        m_msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::Yes)->setText(_("Yes(&Y)"));
        m_msgBox->button(QMessageBox::No)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::No)->setText(_("No(&N)"));
        m_msgBox->setDefaultButton(QMessageBox::No);
        int ret = m_msgBox->exec();
        delete m_msgBox;
        if (ret == QMessageBox::No)
        {
            ui->cmbSwitchCnEn->setCurrentIndex(freewb_cn_en_switch_preset_index(settings::instance().get_cnEnSwitch()));
            return;
        }
    }

    settings::instance().set_cnEnSwitch(presets[static_cast<size_t>(index)].token);
}

void SettingWin::on_cmbTmpEnglish_activated(const QString &arg1)
{
    Q_UNUSED(arg1);
    settings::instance().set_tempEnglish(ui->cmbTmpEnglish->currentData().toString().toStdString());

    update_short_input_cmb();
    update_tmp_pinyin_cmb();
}

void SettingWin::on_cmbShortcutInput_activated(const QString &arg1)
{
    Q_UNUSED(arg1);
    settings::instance().set_shortcutInput(ui->cmbShortcutInput->currentData().toString().toStdString());

    update_tmp_engish_cmb();
    update_tmp_pinyin_cmb();
}

void SettingWin::on_cmbTmpPinyin_activated(const QString &arg1)
{
    Q_UNUSED(arg1);
    settings::instance().set_tempPinyin(ui->cmbTmpPinyin->currentData().toString().toStdString());

    update_tmp_engish_cmb();
    update_short_input_cmb();
}

void SettingWin::on_btnRestoreShortcutKey_clicked()
{
    m_msgBox = new QMessageBox(this);
    // m_msgBox->setWindowFlag( Qt::FramelessWindowHint );
    m_msgBox->setIcon(QMessageBox::Warning);
    m_msgBox->setText(_("Confirm to restore all shortcut keys to the default key value?"));
    m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    m_msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
    m_msgBox->button(QMessageBox::Yes)->setText(_("Yes(&Y)"));
    m_msgBox->button(QMessageBox::No)->setIcon(QIcon());
    m_msgBox->button(QMessageBox::No)->setText(_("No(&N)"));
    m_msgBox->setDefaultButton(QMessageBox::Yes);

    int ret = m_msgBox->exec();
    delete m_msgBox;
    if (ret == QMessageBox::Yes)
    {
        settings::instance().restore_default_shortcutkey();
        init_shortcutkey_page();
    }
}

void SettingWin::slot_custom_keyboard_char_clicked(SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue)
{
    // qDebug() << keyValue.commChar;

    m_curSymbolKeyIdx = keyIdx;
    m_curCustomKeyValue = keyValue;

    m_customKeyDialog->setWindowTitle(_("Set keyboard characters"));
    m_customKeyDialog->set_custom_symbol(VKM_CUSTOM_CHAR, keyName, keyValue.commChar, keyValue.shiftChar);
    m_customKeyDialog->exec();
}

void SettingWin::slot_custom_keyboard_mark_clicked(SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue)
{
    // qDebug() << keyValue.commMark;

    m_curSymbolKeyIdx = keyIdx;
    m_curCustomKeyValue = keyValue;

    m_customKeyDialog->setWindowTitle(_("Set keyboard punctuation"));
    m_customKeyDialog->set_custom_symbol(VKM_CUSTOM_MARK, keyName, keyValue.commMark, keyValue.shiftMark);
    m_customKeyDialog->exec();
}

void SettingWin::slot_custom_btn_ok_clicked(const QString &commSymbol, const QString &shiftSymbol)
{
    // qDebug() << commSymbol << shiftSymbol;

    if (ui->stackedWidget->currentWidget() == ui->pageCustomKeyChar)
    {
        m_curCustomKeyValue.commChar = commSymbol;
        m_curCustomKeyValue.shiftChar = shiftSymbol;
        m_kbCustomKeyChar->update_customkey_button(m_curSymbolKeyIdx, commSymbol, shiftSymbol);
    }
    else
    {
        m_curCustomKeyValue.commMark = commSymbol;
        m_curCustomKeyValue.shiftMark = shiftSymbol;
        m_kbCustomKeyMark->update_customkey_button(m_curSymbolKeyIdx, commSymbol, shiftSymbol);
    }
#ifdef DEBUG
    qDebug() << m_curCustomKeyValue.commChar << m_curCustomKeyValue.shiftChar << m_curCustomKeyValue.commMark
             << m_curCustomKeyValue.shiftMark;
#endif
    std::string chars = settings::instance().get_CoustomChar();
    std::string marks = settings::instance().get_CoustomMark();
    if (freewb_custom_key_info_apply_to_values(chars, marks, static_cast<int>(m_curSymbolKeyIdx), KEY_SYMBOL_NUM,
                                               customKeyFromQt(m_curCustomKeyValue)))
    {
        settings::instance().set_CoustomChar(chars);
        settings::instance().set_CoustomMark(marks);
    }
}

void SettingWin::on_cmbSkinSelect_activated(const QString &arg1)
{
    settings::instance().set_curSkinId(fromStdUtf8(arg1));
    update_toolbar_preview(arg1);
    init_candidate_ui_page();
}

void SettingWin::on_ckbAutoLocate_toggled(bool checked)
{
    settings::instance().set_toolbarAutoLocate(checked);
}

void SettingWin::on_ckbAutoExtend_toggled(bool checked)
{
    settings::instance().set_toolbarAutoExpand(checked);
}

void SettingWin::on_ckbEnableUiAudioEffect_toggled(bool checked)
{
    settings::instance().set_uiAudioEffect(checked);
}

void SettingWin::on_ckbDispRealHelp_toggled(bool checked)
{
    settings::instance().set_showRealtimeHelp(checked);
}

void SettingWin::on_ckbHideToolbar_toggled(bool checked)
{
    settings::instance().set_hideToolbar(checked);
}

void SettingWin::on_spbToolbarTransparency_valueChanged(int arg1)
{
    settings::instance().set_toolbarTransparency(arg1);
    // ui->labelToolbar->setWindowOpacity( 1 - arg1/100.0 );
}

void SettingWin::on_cmbCandiWinMode_activated(int index)
{
    if (index == 0)
    {
        settings::instance().set_candiWinDispMode(CWDM_ONE_ROW);
    }
    else if (index == 1)
    {
        settings::instance().set_candiWinDispMode(CWDM_MULTI_ROW);
    }
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ledtSeparateChar_textChanged(const QString &arg1)
{
    if (!arg1.length())
    {
        settings::instance().set_separateChar(freewb_separate_char_to_string(0));
    }
    else
    {
        settings::instance().set_separateChar(freewb_separate_char_to_string(arg1.toLatin1().at(0)));
    }
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ckbUseGradientBgColor_toggled(bool checked)
{
    settings::instance().set_useGradientColor(checked);
    if (checked)
    {
        settings::instance().set_useBgImage(false);
    }
    ckb_useGradientBgColor_updated(checked);
    update_fram_candidate_win();
}

void SettingWin::on_ckbUseBgImage_toggled(bool checked)
{
    settings::instance().set_useBgImage(checked);
    if (checked)
    {
        settings::instance().set_useGradientColor(false);
    }
    ckb_useBgImage_updated(checked);
    update_fram_candidate_win();
}

void SettingWin::on_ckbUseTile_toggled(bool checked)
{
    settings::instance().set_enableTiled(checked);
    update_fram_candidate_win();
}

// 通过setValue函数设置值时也会出发该槽函数!!!!!!!!
void SettingWin::on_spbCandiItemNum_valueChanged(int arg1)
{
    settings::instance().set_candiWordCount(arg1);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_spbCornerRadian_valueChanged(int arg1)
{
    settings::instance().set_radius(arg1);
    update_fram_candidate_win();
}

void SettingWin::on_spbCandiTransparency_valueChanged(int arg1)
{
    settings::instance().set_transparency(arg1);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_spbCandiCharNum_valueChanged(int arg1)
{
    settings::instance().set_candiCharCount(arg1);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_btnCandiFont_clicked()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont(freewb_candi_text_qfont(settings::instance()));
    if (dialog.exec() == QFontDialog::Accepted)
    {
        freewb_candi_text_font_apply_qfont(settings::instance(), dialog.selectedFont());
        g_settingsNotifier.notifySettingDataChangedToLocal();
        // ui->btnCandiFont->setFont( dialog.selectedFont() );
    }
}

void SettingWin::on_btnCandiBg_clicked()
{
    if (settings::instance().get_useBgImage())
    {
        QString file =
            QFileDialog::getOpenFileName(this, _("Select background image"), qgetenv("HOME"), "Images(*.png *.bmp *.jpg)");
        settings::instance().set_bgImage(fromStdUtf8(file));
        update_fram_candidate_win();
    }
    else
    {
        QColorDialog dialog(this);

        dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_bgColor()));
        if (dialog.exec() == QColorDialog::Accepted)
        {
            settings::instance().set_bgColor(fromStdUtf8(dialog.selectedColor().name(QColor::HexArgb)));
            update_fram_candidate_win();
        }
    }
}

void SettingWin::on_btnCandiBgColor0_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_gradientColor0()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        settings::instance().set_gradientColor0(fromStdUtf8(dialog.selectedColor().name(QColor::HexArgb)));
        update_fram_candidate_win();
    }
}

void SettingWin::on_btnCandiBgColor1_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_gradientColor1()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        settings::instance().set_gradientColor1(fromStdUtf8(dialog.selectedColor().name(QColor::HexArgb)));
        update_fram_candidate_win();
    }
}

void SettingWin::on_btnCandiBorderColor_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_borderColor()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        settings::instance().set_borderColor(fromStdUtf8(dialog.selectedColor().name(QColor::HexArgb)));
        update_fram_candidate_win();
    }
}

void SettingWin::on_btnCandiAutoWord_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_candiWordTextColor()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        QColor color = dialog.selectedColor();
        settings::instance().set_candiWordTextColor(fromStdUtf8(color.name(QColor::HexArgb)));
        ui->btnCandiAutoWord->setStyleSheet(
            QString("color:rgb(%1,%2,%3);").arg(color.red()).arg(color.green()).arg(color.blue()));
        g_settingsNotifier.notifySettingDataChangedToLocal();
    }
}

void SettingWin::on_btnCandiPrompt_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_candiPromptTextColor()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        QColor color = dialog.selectedColor();
        settings::instance().set_candiPromptTextColor(fromStdUtf8(color.name(QColor::HexArgb)));
        ui->btnCandiPrompt->setStyleSheet(QString("color:rgb(%1,%2,%3);").arg(color.red()).arg(color.green()).arg(color.blue()));
        g_settingsNotifier.notifySettingDataChangedToLocal();
    }
}

/******************************** 候选窗选项 ********************************/
// void SettingWin::on_ledt2ndRecode_textChanged( const QString &arg1 )
//{
//     if ( !arg1.isEmpty() )
//     {
//         settings::instance().set_second_recode_key( arg1.at(0).toLatin1() );
//     }
// }

// void SettingWin::on_ledt3rdRecode_textChanged( const QString &arg1 )
//{
//     if ( !arg1.isEmpty() )
//     {
//         settings::instance().set_third_recode_key( arg1.at(0).toLatin1() );
//     }
// }

void SettingWin::on_ckbShiftSelectRecode_toggled(bool checked)
{
    settings::instance().set_shiftSelectRecode(checked);
}

void SettingWin::on_ckbCursorFollow_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        settings::instance().set_cursorFollow(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_cursorFollow(false);
    }
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ckbHideCandiChinese_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        settings::instance().set_hideCandiWin(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_hideCandiWin(false);
    }
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ckbDispOpPrompt_toggled(bool checked)
{
    settings::instance().set_showOpRemindInfo(checked);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ckbDispOpDict_toggled(bool checked)
{
    // printf("dict=%d\n",checked);
    settings::instance().set_showCandDictInfo(checked);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_cmb23RecodeSelect_activated(int index)
{
    Q_UNUSED(index);
    const auto &presets = recodeSelectPresets();
    bool ok = false;
    const int presetIdx = ui->cmb23RecodeSelect->currentData().toInt(&ok);
    if (!ok || presetIdx < 0 || static_cast<size_t>(presetIdx) >= presets.size())
        return;
    const auto &p = presets[static_cast<size_t>(presetIdx)];
    const std::string prev = settings::instance().get_prevPageKey();
    const std::string next = settings::instance().get_nextPageKey();
    const auto cessk = cnEnSwitchCoverKeys(settings::instance().get_cnEnSwitch());

    QString conflictInfo;
    if (pairIntersects(p.first, p.second, prev, next))
    {
        conflictInfo = _("The shortcut key you set will conflict with the up and down page key, confirm setting?");
    }
    else if (pairIntersects(p.first, p.second, cessk.first, cessk.second))
    {
        conflictInfo = _("The shortcut key you set will conflict with the Chinese/English switch key, confirm setting?");
    }

    if (!conflictInfo.isEmpty())
    {
        m_msgBox = new QMessageBox(this);
        m_msgBox->setIcon(QMessageBox::Warning);
        m_msgBox->setText(conflictInfo);
        m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        m_msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::Yes)->setText(_("Yes(&Y)"));
        m_msgBox->button(QMessageBox::No)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::No)->setText(_("No(&N)"));
        m_msgBox->setDefaultButton(QMessageBox::No);
        int ret = m_msgBox->exec();
        delete m_msgBox;
        if (ret == QMessageBox::No)
        {
            ui->cmb23RecodeSelect->setCurrentIndex(
                recodeSelectPresetIndex(settings::instance().get_secondRecodeKey(), settings::instance().get_thirdRecodeKey()));
            return;
        }
    }

    settings::instance().set_secondRecodeKey(p.first);
    settings::instance().set_thirdRecodeKey(p.second);
}

void SettingWin::on_cmbPrevNextPage_activated(int index)
{
    Q_UNUSED(index);
    const auto &presets = candiPagePresets();
    bool ok = false;
    const int presetIdx = ui->cmbPrevNextPage->currentData().toInt(&ok);
    if (!ok || presetIdx < 0 || static_cast<size_t>(presetIdx) >= presets.size())
        return;
    const auto &p = presets[static_cast<size_t>(presetIdx)];
    const std::string sec = settings::instance().get_secondRecodeKey();
    const std::string thi = settings::instance().get_thirdRecodeKey();

    if (pairIntersects(p.first, p.second, sec, thi))
    {
        m_msgBox = new QMessageBox(this);
        m_msgBox->setIcon(QMessageBox::Warning);
        m_msgBox->setText(
            _("The shortcut key you set will conflict with the second and third recode selection key, confirm setting?"));
        m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        m_msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::Yes)->setText(_("Yes(&Y)"));
        m_msgBox->button(QMessageBox::No)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::No)->setText(_("No(&N)"));
        m_msgBox->setDefaultButton(QMessageBox::No);
        int ret = m_msgBox->exec();
        delete m_msgBox;
        if (ret == QMessageBox::No)
        {
            ui->cmbPrevNextPage->setCurrentIndex(
                candiPagePresetIndex(settings::instance().get_prevPageKey(), settings::instance().get_nextPageKey()));
            return;
        }
    }

    settings::instance().set_prevPageKey(p.first);
    settings::instance().set_nextPageKey(p.second);
}
