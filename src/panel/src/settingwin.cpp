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
#include "skin.h"
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

/* 13 项自定义功能键 accessor 表；顺序与 settingwin.ui 中 cmbFunction 下拉项严格对应。
 * 如需增减或改序，必须同步调整 .ui 中的条目顺序。 */
const std::array<CustomShortcutAccessor, 13> kCustomShortcutAccessors = {{
    {&settings::Settings::get_backFindCode, &settings::Settings::set_backFindCode},             // 反查编码
    {&settings::Settings::get_onlineAddWord, &settings::Settings::set_onlineAddWord},           // 在线加词
    {&settings::Settings::get_onlineDelWord, &settings::Settings::set_onlineDelWord},           // 在线删词
    {&settings::Settings::get_switchVKb, &settings::Settings::set_switchVKb},                   // 切换软键盘
    {&settings::Settings::get_switchCharSet, &settings::Settings::set_switchCharSet},           // 切换字符集
    {&settings::Settings::get_switchInputMode, &settings::Settings::set_switchInputMode},       // 切换输入模式
    {&settings::Settings::get_switchChttrans, &settings::Settings::set_switchChttrans},         // 切换简入繁出
    {&settings::Settings::get_setupOption, &settings::Settings::set_setupOption},               // 打开系统设置
    {&settings::Settings::get_showHideToolbar, &settings::Settings::set_showHideToolbar},       // 显/隐状态栏
    {&settings::Settings::get_switchLexicon, &settings::Settings::set_switchLexicon},           // 切换词库
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

/* 设置页：两 KEY_* token 是否映射到同一 keysym。 */
bool tokensSharePhysicalKey(const std::string &lhs, const std::string &rhs)
{
    if (lhs.empty() || rhs.empty() || lhs == "KEY_NONE" || rhs == "KEY_NONE")
        return false;
    if (lhs == rhs)
        return true;
    const FreewbKeySym lhsSym = freewb::Key::keySymFromUniqueName(lhs.c_str());
    const FreewbKeySym rhsSym = freewb::Key::keySymFromUniqueName(rhs.c_str());
    if (lhsSym == FreewbKey_None || rhsSym == FreewbKey_None)
        return false;
    return lhsSym == rhsSym;
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
    setUiTexts();

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
    ui->label_42->hide();
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
    setWindowFlags(Qt::Window);
    setFixedSize(size());
    setWindowIcon(QIcon::fromTheme("freewb"));
    setWindowTitle(_("Settings"));
    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    // setFont(freewb_candi_text_qfont(settings::instance()));

    ui->labelVersionNum->setText(FREEWB_VERSION);
    ui->label_2->setPixmap(QIcon::fromTheme("freewb").pixmap(60, 60));
    ui->label_2->setScaledContents(true);

    // 载入窗口全局UI样式表
    QFile qssFile(QSS_FILE);
    if (qssFile.open(QFile::ReadOnly))
    {
        // 仅作用于 frame 内控件，避免 QMessageBox 子窗口继承 QWidget 样式。
        ui->frame->setStyleSheet(qssFile.readAll());
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

void SettingWin::setUiTexts()
{
    ui->labelCommon->setText(_("Common options"));
    ui->ckbCodeRemind->setText(_("Enable incremental code hints"));
    ui->ckbWordThink->setText(_("Enable phrase association"));
    ui->ckbRemindExistWord->setText(_("Remind when phrase exists in lexicon"));
    ui->ckbAutoAdjustFreq->setText(_("Enable automatic frequency adjustment"));
    ui->ckbSpaceFullWhenCharHalf->setText(_("Full-width space when characters are half-width"));
    ui->ckbSmartMark->setText(_("Enable smart punctuation"));
    ui->ckbAlertWhenEmptyCode->setText(_("Sound alert on duplicate or empty code"));
    ui->labelUseAudioFile->setText(_("Use sound files"));
    ui->labelAdvance->setText(_("Advanced options"));
    ui->ckbRepeatCalib->setText(_("Duplicate commit proofreading mode"));
    ui->ckbTypeEffect->setText(_("Enable typing sound effects"));
    ui->ckbShiftCommitChar->setText(_("Shift+letter commits directly"));
    ui->ckbInputStatistic->setText(_("Enable input statistics"));
    ui->labelOthers->setText(_("Other settings"));
    ui->label_4->setText(_("Auto-switch to English strings"));
    ui->label_5->setText(_("Character"));
    ui->labelAutoEnPrompt->setText(_("Freewb switches to English when you type these strings; Enter returns to Chinese.\n"
                                     "Up to four characters, separated by spaces. Useful for browsing."));
    ui->labelAutoMarkPrompt->setText(
        _("In Chinese mode, commas and periods after digits become half-width (useful for finance)."));
    ui->ckbAutoHalfMarkAfterNum->setText(_("Half-width punctuation after digits"));
    ui->labelUi->setText(_("Interface settings"));
    ui->label_24->setText(_("Options"));
    ui->ckbAutoLocate->setText(_("Auto-position toolbar"));
    ui->labelLossLocate->setText(_("When auto-position fails"));
    ui->ckbAutoExtend->setText(_("Auto expand/collapse toolbar"));
    ui->ckbEnableUiAudioEffect->setText(_("Enable UI sound effects"));
    ui->ckbDispRealHelp->setText(_("Show realtime help"));
    ui->ckbHideToolbar->setText(_("Hide toolbar"));
    ui->label_26->setText(_("Toolbar transparency"));
    ui->labelCandidateWinUi->setText(_("Candidate window interface"));
    ui->label_28->setText(_("Candidate window style"));
    ui->label_29->setText(_("Options"));
    ui->label_30->setText(_("Index/candidate separator"));
    ui->ckbUseGradientBgColor->setText(_("Use gradient background"));
    ui->ckbUseTile->setText(_("Tile"));
    ui->ckbUseBgImage->setText(_("Use background image"));
    ui->label_31->setText(_("Corner radius"));
    ui->label_32->setText(_("Transparency"));
    ui->label_33->setText(_("Number of candidates"));
    ui->label_34->setText(_("Characters per candidate"));
    ui->label_35->setText(_("Candidate window (click labels to change font and colors)"));
    ui->btnCandiFont->setText(_("Change font"));
    ui->btnCandiBgColor0->setText(_("Gradient start"));
    ui->btnCandiBgColor1->setText(_("Gradient end"));
    ui->btnCandiBorderColor->setText(_("Border color"));
    ui->btnCandiAutoWord->setText(_("Auto phrase"));
    ui->btnCandiPrompt->setText(_("Prompt text"));
    ui->btnCandiBg->setText(_("Background color"));
    ui->labelCandidateWinOption->setText(_("Candidate window options"));
    ui->label_36->setText(_("2nd/3rd duplicate keys"));
    ui->label_37->setText(_("2nd duplicate key"));
    ui->label_38->setText(_("3rd duplicate key"));
    ui->label_39->setText(_("Page up/down keys"));
    ui->label_40->setText(_("Previous page"));
    ui->label_41->setText(_("Previous page"));
    ui->ckbCursorFollow->setText(_("Candidate window follows caret"));
    ui->ckbDispOpPrompt->setText(_("Show operation hints"));
    ui->ckbShiftSelectRecode->setText(_("Use Shift to select duplicates"));
    ui->ckbDispOpDict->setText(_("Live dictionary on candidates"));
    ui->labelShortcutKey->setText(_("Shortcut settings"));
    ui->labelCustom->setText(_("Custom shortcuts"));
    ui->label_11->setText(_("Function"));
    ui->label_12->setText(_("Shortcut"));
    ui->ckbDisableAllShortcutKey->setText(_("Disable all shortcuts"));
    ui->ckbDisableFullHalfKey->setText(_("Disable full/half width shortcut"));
    ui->label_13->setText(_("Shortcut input"));
    ui->labelEasy->setText(_("Convenience shortcuts"));
    ui->label_15->setText(_("Temporary English"));
    ui->label_16->setText(_("Temporary Pinyin\nrare characters"));
    ui->labelCnEn->setText(_("Chinese/English\nswitch"));
    ui->labelTwo->setText(_("Double-tap convenience key for symbol"));
    ui->btnRestoreShortcutKey->setText(_("Restore default shortcuts"));
    ui->labelCustomKeyChar->setText(_("Custom soft keyboard"));
    ui->labelPrompt_1->setText(_("Click the soft keyboard character to edit"));
    ui->labelCustomKeyMark->setText(_("Custom punctuation"));
    ui->labelPrompt_2->setText(_("Click the punctuation mark to edit"));
    ui->labelVersionInfo->setText(_("Version information"));
    ui->labelVersion->setText(_("Freewb Input Method"));
    ui->btnOk->setText(_("OK"));
    ui->btnCancel->setText(_("Cancel"));
    ui->ckbAutoPhrase->setText(_("Auto phrase options"));
    ui->ckbZzSpecialEncodingSymbols->setText(_("ZZ special encoding symbols"));
    ui->cmbWhenLossLocation->setItemText(0, _("Hide toolbar"));
    ui->cmbWhenLossLocation->setItemText(1, _("Top-left of desktop"));
    ui->cmbWhenLossLocation->setItemText(2, _("Top-right of desktop"));
    ui->cmbWhenLossLocation->setItemText(3, _("Bottom-left of desktop"));
    ui->cmbWhenLossLocation->setItemText(4, _("Bottom-right of desktop"));
    ui->cmbCandiWinMode->setItemText(0, _("Single row"));
    ui->cmbCandiWinMode->setItemText(1, _("Multi row"));
    ui->cmbFunction->setItemText(0, _("Reverse code lookup"));
    ui->cmbFunction->setItemText(1, _("Add word online"));
    ui->cmbFunction->setItemText(2, _("Delete word online"));
    ui->cmbFunction->setItemText(3, _("Toggle virtual keyboard"));
    ui->cmbFunction->setItemText(4, _("Switch character set"));
    ui->cmbFunction->setItemText(5, _("Switch input mode"));
    ui->cmbFunction->setItemText(6, _("Toggle simplified/traditional output"));
    ui->cmbFunction->setItemText(7, _("Open system settings"));
    ui->cmbFunction->setItemText(8, _("Show/hide status bar"));
    ui->cmbFunction->setItemText(9, _("Switch lexicon"));
    ui->cmbFunction->setItemText(10, _("Switch skin"));
    ui->cmbFunction->setItemText(11, _("Quick delete committed item"));
    ui->cmbFunction->setItemText(12, _("Auto-pair punctuation"));
    ui->cmbSwitchCnEn->setItemText(0, _("None"));
    ui->cmbSwitchCnEn->setItemText(1, _("Left Shift"));
    ui->cmbSwitchCnEn->setItemText(2, _("Right Shift"));
    ui->cmbSwitchCnEn->setItemText(3, _("Left Ctrl"));
    ui->cmbSwitchCnEn->setItemText(4, _("Right Ctrl"));
}

void SettingWin::init_mouse_hover_tips()
{
    m_tooltipsWin.setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    m_tooltipsWin.setAttribute(Qt::WA_TranslucentBackground);
    m_tooltipsLabel = new QLabel(&m_tooltipsWin);
    m_tooltipsLabel->setStyleSheet(QSS_TOOL_TIPS);

    ui->ckbCodeRemind->setToolTip(_("When you enter code 'a', besides the character for 'a', candidates starting with 'a' "
                                    "are also shown, e.g. entries like '式a 节b'."));
    ui->ckbSpaceFullWhenCharHalf->setToolTip(
        _("When editing Word documents, paragraph indents of two Chinese characters can be entered "
          "conveniently with this option."));
    ui->ckbWordThink->setToolTip(_("When enabled, after typing '中', related phrases such as '中国' and '中国共产党' are "
                                   "listed for selection."));
    ui->ckbSmartMark->setToolTip(_("When enabled, paired punctuation can be entered with the opening mark; e.g. '(' outputs "
                                   "'()' and places the cursor inside. Press Enter to finish."));
    ui->ckbRemindExistWord->setToolTip(
        _("When enabled, if a phrase exists in the lexicon but you type it character by character, "
          "you are prompted. If ignored repeatedly, the phrase may be hidden depending on Advanced "
          "settings."));
    ui->ckbAlertWhenEmptyCode->setToolTip(_("When enabled, an alert sounds when the code is empty or has duplicate candidates."));
    ui->ckbUseAudioFile->setToolTip(_("When enabled, alerts use sound files in the Freewb sound directory."));
    ui->ckbAutoAdjustFreq->setToolTip(
        _("The selected duplicate candidate is moved to the first position.\nNote: single characters "
          "are not adjusted this way; use Ctrl+number instead."));

    ui->ckbShiftCommitChar->setToolTip(
        _("When enabled, Shift+letter outputs the letter directly; otherwise temporary English mode "
          "is used and Enter commits the text."));
    ui->ckbInputStatistic->setToolTip(_("When enabled, typing speed is tracked in real time."));
    ui->ckbTypeEffect->setToolTip(_("Your computer behaves like a typewriter (useful for Wubi beginners)."));
    ui->ckbRepeatCalib->setToolTip(_("When enabled, duplicate or empty codes output the first two candidates or codes for batch "
                                     "proofreading."));
    ui->ckbAutoPhrase->setToolTip(_("When enabled, consecutive single-character commits form auto phrases; selected auto phrases "
                                    "are saved to the auto phrase lexicon."));
    ui->ckbZzSpecialEncodingSymbols->setToolTip(
        _("When enabled, the Wubi engine loads zz special encoding symbols from the mb table."));

    ui->ledtAutoToEnStr->setToolTip(_("When typing URLs such as 'www.freewb.org', entering 'www.' switches to English so browser "
                                      "autocomplete can be used."));
    ui->ledtAutoToHalf->setToolTip(_("For numbers like '12,345.9', enter half-width ',.' here to input grouped numbers without "
                                     "wrong full-width punctuation."));

    ui->cmbSkinSelect->setToolTip(_("Shows installed skins; select one to change the appearance."));
    ui->ckbAutoLocate->setToolTip(_("When enabled, the toolbar is placed at the top-right of the active window. Drag it "
                                    "elsewhere if you prefer another corner."));
    ui->cmbWhenLossLocation->setToolTip(_("Opens a drop-down menu."));
    ui->ckbAutoExtend->setToolTip(_("When enabled, hidden toolbar buttons expand on mouse hover and collapse when the pointer "
                                    "leaves."));
    ui->ckbEnableUiAudioEffect->setToolTip(_("When enabled, toolbar and candidate window actions play sound effects."));
    ui->ckbDispRealHelp->setToolTip(_("When enabled, brief help is shown when hovering toolbar buttons."));
    ui->ckbHideToolbar->setToolTip(_("Hide the toolbar in games or fullscreen apps to reduce distraction."));
    ui->spbToolbarTransparency->setToolTip(_("Adjust toolbar transparency."));

    ui->cmbCandiWinMode->setToolTip(_("Choose single-row, double-row, or multi-row candidate window layout."));
    ui->ledtSeparateChar->setToolTip(_("Change the separator between index and candidate text."));
    ui->ckbUseGradientBgColor->setToolTip(_("Use a gradient background for the candidate window."));
    ui->ckbUseBgImage->setToolTip(_("Use an image as the candidate window background."));
    ui->ckbUseTile->setToolTip(_("Tile the background image; otherwise stretch it to fill the window."));
    ui->spbCornerRadian->setToolTip(_("Set the corner radius of the candidate window."));
    ui->spbCandiTransparency->setToolTip(_("Set candidate window transparency."));
    ui->spbCandiItemNum->setToolTip(
        _("Number of candidates shown; larger values mean fewer page turns (balance with appearance)."));
    ui->spbCandiCharNum->setToolTip(_("Maximum characters per candidate; very small values show '...' for hidden text."));

    ui->btnCandiFont->setToolTip(_("Open font settings for candidate text."));
    ui->btnCandiBg->setToolTip(_("Set candidate window background color or image."));
    ui->btnCandiBgColor0->setToolTip(_("Set the gradient start color."));
    ui->btnCandiBgColor1->setToolTip(_("Set the gradient end color."));
    ui->btnCandiBorderColor->setToolTip(_("Set the candidate window border color."));
    ui->btnCandiAutoWord->setToolTip(_("Set the color for auto-generated brief codes."));
    ui->btnCandiPrompt->setToolTip(_("Set the candidate window prompt text color."));

    ui->ledt2ndRecode->setToolTip(_("Key to select the 2nd candidate, e.g. ',' for users who prefer comma."));
    ui->ledt3rdRecode->setToolTip(_("Key to select the 3rd candidate, e.g. '.' for users who prefer period."));
    ui->ckbCursorFollow->setToolTip(_("When enabled, the candidate window follows the caret; otherwise it stays at the bottom "
                                      "(or drag it anywhere)."));
    ui->ckbDispOpPrompt->setToolTip(_("Show operation hints at the bottom of the multi-row candidate window, e.g. "
                                      "'Ctrl+= add word online'."));
    ui->ckbShiftSelectRecode->setToolTip(
        _("When enabled, Left Shift selects the 2nd candidate and Right Shift the 3rd. Swap in Expert "
          "settings if needed."));
    ui->ledtPrecPage->setToolTip(_("Previous page key, e.g. ',' if you prefer comma and period for paging."));
    ui->ledtNextPage->setToolTip(_("Next page key, e.g. '.' if you prefer comma and period for paging."));
    ui->cmb23RecodeSelect->setToolTip(_("Keys for selecting 2nd and 3rd duplicate candidates."));
    ui->cmbPrevNextPage->setToolTip(_("Keys for candidate paging."));
    ui->ckbDispOpDict->setToolTip(_("Show dictionary lookup when hovering candidates."));

    ui->cmbFunction->setToolTip(_("Select a Freewb function such as reverse lookup or online word creation."));
    ui->cmbShortcutKey->setToolTip(_("Shortcut for the function selected above."));
    ui->ckbDisableAllShortcutKey->setToolTip(_("Disable all Freewb shortcuts to avoid conflicts with other applications."));
    ui->ckbDisableFullHalfKey->setToolTip(
        _("Shift+Space is the default full/half width shortcut in Fcitx; enable this to disable it."));
    ui->cmbTmpEnglish->setToolTip(_("For short English input (e.g. email), press this key then type English and press Enter to "
                                    "return to Chinese. Also used as a lead key for advanced features."));
    ui->cmbShortcutInput->setToolTip(QString(_("Press this key, then a letter to output a predefined phrase.\nCustom rules: %1\n"
                                               "(Right-click toolbar → Management tools → Edit shortcut table)."))
                                         .arg(INSTALL_DIR + "/data/quick_table.txt"));
    ui->cmbTmpPinyin->setToolTip(_("When in Wubi mode, press this key for temporary Pinyin input of unknown characters, then "
                                   "return to Wubi. With code already typed, toggles rare-character mode."));
    ui->cmbSwitchCnEn->setToolTip(_("Choose a key to switch Chinese/English without closing or switching the input method."));
}

void SettingWin::show_mouse_hover_tips(QWidget *widget)
{
    QString tips = widget->toolTip();
    if (tips.isEmpty())
    {
        return;
    }

    // ToolTip may fire repeatedly while the pointer moves (esp. on Wayland); position once like the old build.
    if (m_tooltipsWinShowFlg)
    {
        return;
    }

    m_tooltipsLabel->setText(tips);
    m_tooltipsLabel->adjustSize();
    m_tooltipsWin.adjustSize();

    QPoint position = QCursor::pos();
    QSize desktopSize = QApplication::desktop()->size();
    if (position.x() + m_tooltipsWin.width() > desktopSize.width())
    {
        position.setX(desktopSize.width() - m_tooltipsWin.width());
    }
    else
    {
        position.setX(position.x() + 10);
    }

    if (position.y() + m_tooltipsWin.height() > desktopSize.height() && m_tooltipsWin.height() < position.y())
    {
        position.setY(position.y() - m_tooltipsWin.height() - 10);
    }
    else
    {
        position.setY(position.y() + 10);
    }

    m_tooltipsWin.move(position);
    m_tooltipsWin.show();
    m_tooltipsWinShowFlg = true;
}

void SettingWin::install_evt_filter()
{
    installEventFilter(this);

    for (QWidget *widget : findChildren<QWidget *>())
    {
        if (!widget->toolTip().isEmpty())
        {
            widget->installEventFilter(this);
        }
    }
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
        QWidget *widget = qobject_cast<QWidget *>(obj);
        if (widget && !widget->toolTip().isEmpty())
        {
            show_mouse_hover_tips(widget);
            isProcessed = true;
        }
    }
    else if (event->type() == QEvent::Leave)
    {
        QWidget *widget = qobject_cast<QWidget *>(obj);
        if (widget && !widget->toolTip().isEmpty() && m_tooltipsWinShowFlg)
        {
            m_tooltipsWin.hide();
            m_tooltipsWinShowFlg = false;
        }
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

    m_listItemUi = new QListWidgetItem(_("Interface settings"), ui->listWidget);
    m_listItemCandidateWinUi = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), _("Candidate window interface"), ui->listWidget);
    m_listItemCandidateWinOption = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), _("Candidate window options"), ui->listWidget);

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
    ui->ckbSmartMark->setChecked(settings::instance().get_puncAutoPair());
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
    ui->ckbAutoPhrase->setChecked(settings::instance().get_autoPhrase());
    ui->ckbZzSpecialEncodingSymbols->setChecked(settings::instance().get_zzSpecialEncodingSymbols());
}

// 初始化其它选项设置页面
void SettingWin::init_others_page()
{
    ui->ledtAutoToEnStr->setText(toQStringUtf8(settings::instance().get_autoToEnStr()));
    // ui->ledtAutoToHalf->setText( settings::instance().get_CoustomMark() );
    ui->ledtAutoToHalf->hide();
    ui->ckbAutoHalfMarkAfterNum->setChecked(settings::instance().get_autoToHalfPuncAfterNumber());
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

    const int cnEnIdx = freewb_cn_en_switch_preset_index(settings::instance().get_cnEnSwitch());
    ui->cmbSwitchCnEn->setCurrentIndex(cnEnIdx >= 0 ? cnEnIdx : 0);
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

    Skin &skin = Skin::instance();
    skin.refreshSkinList();

    const QStringList skinIds = skin.availableSkinIds();
    for (const QString &skinId : skinIds)
    {
        ui->cmbSkinSelect->addItem(skin.skinName(skinId), skinId);
    }

    for (int i = 0; i < ui->cmbSkinSelect->count(); i++)
    {
        if (ui->cmbSkinSelect->itemData(i).toString() == toQStringUtf8(settings::instance().get_curSkinId()))
        {
            ui->cmbSkinSelect->setCurrentIndex(i);
            break;
        }
    }

    update_toolbar_preview(ui->cmbSkinSelect->currentData().toString());
}

void SettingWin::update_toolbar_preview(const QString &skinId)
{
    ui->labelToolbar->setStyleSheet(QString("border-image:url(%1);").arg(Skin::instance().toolbarPreviewPath(skinId)));
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
    QDesktopServices::openUrl(QUrl(QString(FREEWB_INSTALL_PKGDATADIR) + "/help/help.html"));
}

void SettingWin::on_ckbCodeRemind_stateChanged(int arg1)
{

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
        settings::instance().set_puncAutoPair(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_puncAutoPair(false);
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

void SettingWin::on_ckbAutoPhrase_toggled(bool checked)
{
    settings::instance().set_autoPhrase(checked);
}

void SettingWin::on_ckbZzSpecialEncodingSymbols_toggled(bool checked)
{
    settings::instance().set_zzSpecialEncodingSymbols(checked);
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
    settings::instance().set_autoToHalfPuncAfterNumber(checked);
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
    const std::string &cnEnToken = presets[static_cast<size_t>(index)].token;
    const std::string sec = settings::instance().get_secondRecodeKey();
    const std::string thi = settings::instance().get_thirdRecodeKey();

    if (tokensSharePhysicalKey(cnEnToken, sec) || tokensSharePhysicalKey(cnEnToken, thi))
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
    m_msgBox->setIcon(QMessageBox::Warning);
    m_msgBox->setText(_("Confirm to restore all shortcut keys to the default key value?"));
    m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    m_msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
    m_msgBox->button(QMessageBox::Yes)->setText(_("Yes(&Y)"));
    m_msgBox->button(QMessageBox::No)->setIcon(QIcon());
    m_msgBox->button(QMessageBox::No)->setText(_("No(&N)"));
    m_msgBox->setDefaultButton(QMessageBox::Yes);

    const int ret = m_msgBox->exec();
    delete m_msgBox;
    if (ret == QMessageBox::Yes)
    {
        settings::instance().restore_default_shortcutkey();
        init_shortcutkey_page();
    }
}

void SettingWin::slot_custom_keyboard_char_clicked(SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue)
{

    m_curSymbolKeyIdx = keyIdx;
    m_curCustomKeyValue = keyValue;

    m_customKeyDialog->setWindowTitle(_("Set keyboard characters"));
    m_customKeyDialog->set_custom_symbol(VKM_CUSTOM_CHAR, keyName, keyValue.commChar, keyValue.shiftChar);
    m_customKeyDialog->exec();
}

void SettingWin::slot_custom_keyboard_mark_clicked(SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue)
{

    m_curSymbolKeyIdx = keyIdx;
    m_curCustomKeyValue = keyValue;

    m_customKeyDialog->setWindowTitle(_("Set keyboard punctuation"));
    m_customKeyDialog->set_custom_symbol(VKM_CUSTOM_MARK, keyName, keyValue.commMark, keyValue.shiftMark);
    m_customKeyDialog->exec();
}

void SettingWin::slot_custom_btn_ok_clicked(const QString &commSymbol, const QString &shiftSymbol)
{

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
    std::string chars = settings::instance().get_CoustomChar();
    std::string marks = settings::instance().get_CoustomMark();
    if (freewb_custom_key_info_apply_to_values(chars, marks, static_cast<int>(m_curSymbolKeyIdx), KEY_SYMBOL_NUM,
                                               customKeyFromQt(m_curCustomKeyValue)))
    {
        settings::instance().set_CoustomChar(chars);
        settings::instance().set_CoustomMark(marks);
    }
}

void SettingWin::on_cmbSkinSelect_activated(const QString &)
{
    const QString skinId = ui->cmbSkinSelect->currentData().toString();
    settings::instance().set_curSkinId(fromStdUtf8(skinId));
    update_toolbar_preview(skinId);
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
    g_settingsNotifier.notifySettingDataChangedToLocal();
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
    const std::string &cnEnToken = settings::instance().get_cnEnSwitch();

    QString conflictInfo;
    if (tokensSharePhysicalKey(p.first, prev) || tokensSharePhysicalKey(p.first, next) ||
        tokensSharePhysicalKey(p.second, prev) || tokensSharePhysicalKey(p.second, next))
    {
        conflictInfo = _("The shortcut key you set will conflict with the up and down page key, confirm setting?");
    }
    else if (tokensSharePhysicalKey(cnEnToken, p.first) || tokensSharePhysicalKey(cnEnToken, p.second))
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

    if (tokensSharePhysicalKey(p.first, sec) || tokensSharePhysicalKey(p.first, thi) || tokensSharePhysicalKey(p.second, sec) ||
        tokensSharePhysicalKey(p.second, thi))
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
