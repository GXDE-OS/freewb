/****************************************************************************************
** @作者：lcj
**
** @说明：
**      SettingWin是一个从QWidget类继承而来的UI类，其对应的UI设计文件为settingwin.ui，该类设计目的
**      为被MainProgram类所包含实例化，当用户点击桌面工具条上的设置按钮或通过右键菜单选择图形化设置按钮
**      时会弹出该类所描绘的输入法配置界面。
*************************************************************************************×**/

#ifndef SETTINGWIN_H
#define SETTINGWIN_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QDebug>
#include <QFile>
#include <QDesktopWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QDateTime>
#include <QEvent>
#include <QDesktopServices>

#include "keyboard.h"
#include "customkeydialog.h"

namespace Ui {
class SettingWin;
}

class SettingWin : public QWidget
{
    Q_OBJECT

public:
    explicit SettingWin( QWidget *parent = nullptr );
    ~SettingWin();

public slots:
    void slot_open_win();
    void slot_init_all_setting_page();
    void slot_show_version_info();
    void slot_open_advanced_settting_page();

    void slot_custom_keyboard_char_clicked( SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue );
    void slot_custom_keyboard_mark_clicked( SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue );
    void slot_custom_btn_ok_clicked( const QString &commSymbol, const QString &shiftSymbol );

protected:
    void init_window_appearance();
    void init_member_data();
    void init_mouse_hover_tips();
    void install_evt_filter();

    void init_common_page();
    void init_advance_page();
    void init_others_page();
    void init_shortcutkey_page();
    void update_custom_shortkey_cmb();
    void update_listwidget_item();
    void update_tmp_engish_cmb();
    void update_short_input_cmb();
    void update_tmp_pinyin_cmb();
    void init_ui_setting_page();
    void update_toolbar_preview( const QString &skinId );
    void init_skin_select_cmb();
    void init_candidate_ui_page();
    void ckb_useGradientBgColor_updated( bool checked );
    void ckb_useBgImage_updated( bool checked );
    void update_fram_candidate_win();
    void init_candidate_option_page();


protected:
    //重载函数
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    bool eventFilter( QObject *obj, QEvent *event );

private slots:
    /****************** 主界面 ***************/
    void on_btnSettingOption_clicked();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    void on_btnHelp_clicked();

    void on_listWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );

    /****************** 常用选项界面 ***************/
    void on_ckbCodeRemind_stateChanged(int arg1);
    void on_ckbSpaceFullWhenCharHalf_toggled(bool checked);
    void on_ckbWordThink_toggled(bool checked);
    void on_ckbSmartMark_stateChanged(int arg1);
    void on_ckbRemindExistWord_toggled(bool checked);
    void on_ckbAlertWhenEmptyCode_toggled(bool checked);
    void on_ckbUseAudioFile_stateChanged(int arg1);
    void on_ckbAutoAdjustFreq_toggled(bool checked);

    /****************** 高级选项设置 ***************/
    void on_ckbShiftCommitChar_toggled(bool checked);
    void on_ckbInputStatistic_toggled(bool checked);
    void on_ckbTypeEffect_toggled( bool checked );
    void on_ckbRepeatCalib_toggled(bool checked);
    void on_ckbAutoWordGroup_activated(int index);

    /****************** 其他选项设置 ***************/
    void on_ledtAutoToEnStr_textChanged(const QString &arg1);
    void on_ledtAutoToHalf_textChanged(const QString &arg1);
    void on_ckbAutoHalfMarkAfterNum_toggled(bool checked);

    /****************** UI设置界面 ***************/
    void on_cmbSkinSelect_activated(const QString &arg1);
    void on_ckbAutoLocate_toggled(bool checked);
    void on_ckbAutoExtend_toggled(bool checked);
    void on_ckbEnableUiAudioEffect_toggled(bool checked);
    void on_ckbDispRealHelp_toggled(bool checked);
    void on_ckbHideToolbar_toggled(bool checked);
    void on_spbToolbarTransparency_valueChanged(int arg1);

    /****************** 候选窗界面 ***************/
    void on_cmbCandiWinMode_activated(int index);
    void on_ledtSeparateChar_textChanged(const QString &arg1);
    void on_ckbUseGradientBgColor_toggled(bool checked);
    void on_ckbUseBgImage_toggled(bool checked);
    void on_ckbUseTile_toggled(bool checked);
    void on_spbCandiItemNum_valueChanged(int arg1);
    void on_spbCornerRadian_valueChanged(int arg1);
    void on_spbCandiTransparency_valueChanged(int arg1);
    void on_spbCandiCharNum_valueChanged(int arg1);
    void on_btnCandiFont_clicked();
    void on_btnCandiBg_clicked();
    void on_btnCandiBgColor0_clicked();
    void on_btnCandiBgColor1_clicked();
    void on_btnCandiBorderColor_clicked();
    void on_btnCandiAutoWord_clicked();
    void on_btnCandiPrompt_clicked();


    /****************** 候选窗选项 ***************/
//    void on_ledt2ndRecode_textChanged(const QString &arg1);
//    void on_ledt3rdRecode_textChanged(const QString &arg1);
    void on_ckbShiftSelectRecode_toggled(bool checked);
    void on_ckbCursorFollow_stateChanged(int arg1);
    void on_ckbHideCandiChinese_stateChanged(int arg1);
    void on_ckbDispOpPrompt_toggled(bool checked);
    void on_ckbDispOpDict_toggled(bool checked);

    void on_cmb23RecodeSelect_activated(int index);
    void on_cmbPrevNextPage_activated(int index);

    /****************** 快捷键界面 ***************/
    void on_cmbFunction_activated(int index);
    void on_cmbShortcutKey_activated(int index);
    void on_ckbDisableAllShortcutKey_stateChanged(int arg1);
    void on_ckbDisableFullHalfKey_toggled(bool checked);
    void on_cmbSwitchCnEn_activated(int index);
    void on_cmbTmpEnglish_activated(const QString &arg1);
    void on_cmbShortcutInput_activated(const QString &arg1);
    void on_cmbTmpPinyin_activated(const QString &arg1);
    void on_btnRestoreShortcutKey_clicked();


private:
    Ui::SettingWin *ui;

    //用于窗口拖动计算
    bool  m_mouseIsPressed;
    QPoint m_mouseLastPosition;

    QPoint m_defaultPopPosition;

    QMessageBox *m_msgBox;
    QWidget m_tooltipsWin;
    QLabel *m_tooltipsLabel;
    bool m_tooltipsWinShowFlg;
    QMap<QWidget*, QString> m_tipsTextMap;


    QListWidgetItem *m_listItemCommon;//常用选项
    QListWidgetItem *m_listItemAdvance;//高级选项
    QListWidgetItem *m_listItemOthers;//其他设置
    QListWidgetItem *m_listItemUi;//界面设置
    QListWidgetItem *m_listItemCandidateWinUi;//候选窗界面
    QListWidgetItem *m_listItemCandidateWinOption;//候选窗选项
    QListWidgetItem *m_listItemShortcutKey;//快捷键设置
    QListWidgetItem *m_listItemCustomKeyChar;//定义软键盘
    QListWidgetItem *m_listItemCustomKeyMark;//自定义标点
    QListWidgetItem *m_listItemVersionInfo;//输入法版本信息
    QListWidgetItem *m_listItemBug;//Bug信息反馈

    Keyboard *m_kbCustomKeyChar;//自定义按键字符设置页面的键盘
    Keyboard *m_kbCustomKeyMark;//自定义按键标点设置页面的键盘
    CustomKeyDialog *m_customKeyDialog;//自定义按键对话框

    SymbolKeyIdx m_curSymbolKeyIdx;//当前正在自定义的按键
    CustomKeyValue m_curCustomKeyValue;//当前正在自定义的按键值

};

#endif
