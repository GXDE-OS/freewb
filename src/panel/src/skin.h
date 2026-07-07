#ifndef SKIN_H
#define SKIN_H

#include <QObject>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>

// 文字输入框界面皮肤配置数据
struct SkinInputWin
{
    int bgTopImageHeight = 0;
    int bgBottomImageHeight = 0;
    int bgLeftImageWidth = 0;
    int bgRightImageWidth = 0;
    QString bgCenterImagePath;
    QString bgTopImagePath;
    QString bgBottomImagePath;
    QString bgLeftImagePath;
    QString bgRightImagePath;
    QString fullIcoPath;
    QString halfIcoPath;
    QString cnMarkIcoPath;
    QString enMarkIcoPath;
    QString prev0PageIcoPath;
    QString prev1PageIcoPath;
    QString next0PageIcoPath;
    QString next1PageIcoPath;
    QString logoIcoPath;
};

// 工具条界面按钮
struct SkinToolBarLogoBtn
{
    bool isExist = false;
    QRect rect;
    QString icoPath;
};

struct SkinToolBarMenuExtendBtn
{
    bool isExist = false;
    QRect rect;
    QString openIcoPath;
    QString closeIcoPath;
};

struct SkinToolBarModeBtn
{
    bool isExist = false;
    QRect rect;
    QString wbFontIcoPath;
    QString wbPinyinIcoPath;
    QString stdPinyinIcoPath;
    QString englishIcoPath;
    QString capsIcoPath;
};

struct SkinToolBarFullHalfBtn
{
    bool isExist = false;
    QRect rect;
    QString fullIcoPath;
    QString halfIcoPath;
};

struct SkinToolBarCnEnMarkBtn
{
    bool isExist = false;
    QRect rect;
    QString cnMarkIcoPath;
    QString enMarkIcoPath;
};

struct SkinToolBarSettingBtn
{
    bool isExist = false;
    QRect rect;
    QString icoPath;
};

struct SkinToolBarGenerateBtn
{
    bool isExist = false;
    QRect rect;
    QString icoPath;
};

struct SkinToolBarSearchBtn
{
    bool isExist = false;
    QRect rect;
    QString icoPath;
};

struct SkinToolBarCharFontBtn
{
    bool isExist = false;
    QRect rect;
    QString simpIcoPath;
    QString tradIcoPath;
};

struct SkinToolBarCharSetBtn
{
    bool isExist = false;
    QRect rect;
    QString gbIcoPath;
    QString gbkIcoPath;
};

struct SkinToolBarKeyboardBtn
{
    bool isExist = false;
    QRect rect;
    QString icoPath;
};

struct SkinToolBar
{
    QSize size0;
    QSize size1;
    QString bg0ImagePath;
    QString bg1ImagePath;

    SkinToolBarLogoBtn stbLogoBtn;
    SkinToolBarMenuExtendBtn stbMenuExtendBtn;
    SkinToolBarModeBtn stbModeBtn;
    SkinToolBarFullHalfBtn stbFullHalfBtn;
    SkinToolBarCnEnMarkBtn stbCnEnMarkBtn;
    SkinToolBarSettingBtn stbSettingBtn;
    SkinToolBarGenerateBtn stbGenerateBtn;
    SkinToolBarSearchBtn stbSearchBtn;
    SkinToolBarCharFontBtn stbCharFontBtn;
    SkinToolBarCharSetBtn stbCharSetBtn;
    SkinToolBarKeyboardBtn stbKeyboardBtn;
};

class Skin : public QObject
{
    Q_OBJECT

public:
    static Skin &instance();

    /** 扫描皮肤目录，返回有效皮肤 ID 列表（文件夹名）。 */
    QStringList availableSkinIds() const;
    /** 皮肤显示名称：读取 skin.ini [Skin] name；未配置时 default 为「经典皮肤」，其余返回 skinId。 */
    QString skinName(const QString &skinId) const;

    QString currentSkinId() const;
    const SkinInputWin &inputWin() const;
    const SkinToolBar &toolbar() const;

    /** 候选窗是否使用九宫格边框图片（由 skin.ini 边框尺寸决定）。 */
    bool inputWinUsesImageBorder() const;
    /** 候选窗 frameBg 样式：非九宫格皮肤时从 settings 生成，否则为空。 */
    QString inputWinFrameBgStyle(int radius) const;

    /** 从 skin.ini 加载指定皮肤配置到内存。 */
    bool load(const QString &skinId);
    /** 从 settings 同步当前皮肤。 */
    bool loadFromSettings();
    /** 切换皮肤：加载配置、可选写入 settings 并发出 skinChanged。 */
    bool switchTo(const QString &skinId, bool persist = true);

    QString toolbarPreviewPath(const QString &skinId) const;

    static QString skinRootDir();
    static QString skinFolder(const QString &skinId);
    static bool isValidSkin(const QString &skinId);

    void refreshSkinList();

signals:
    void skinChanged(const QString &skinId);

private:
    explicit Skin(QObject *parent = nullptr);

    void loadInputWinSkin(class QSettings &settings, const QString &skinFolder);
    void loadToolbarSkin(class QSettings &settings, const QString &skinFolder);

private:
    QString m_curSkinId;
    SkinInputWin m_inputWin;
    SkinToolBar m_toolbar;
    QStringList m_skinIds;
};

#endif
