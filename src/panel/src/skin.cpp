#include "skin.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QtGlobal>

#include "config.h"
#include "settings.h"
#include "settingshelper.h"

namespace
{

void setSkinIniCodec(QSettings &settings)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    settings.setIniCodec("UTF-8");
#endif
}

QColor skinQColorFromSpec(const std::string &spec)
{
    return QColor(toQStringUtf8(spec));
}

} // namespace

Skin &Skin::instance()
{
    static Skin s_skin;
    return s_skin;
}

Skin::Skin(QObject *parent) : QObject(parent)
{
    refreshSkinList();
    loadFromSettings();
}

QString Skin::skinRootDir()
{
    return QString(FREEWB_INSTALL_PKGDATADIR) + "/skin/";
}

QString Skin::skinFolder(const QString &skinId)
{
    return skinRootDir() + skinId + "/";
}

bool Skin::isValidSkin(const QString &skinId)
{
    return QFile(skinFolder(skinId) + "skin.ini").exists();
}

void Skin::refreshSkinList()
{
    m_skinIds.clear();

    const QString skinDir = skinRootDir();
    QStringList dirList = QDir(skinDir).entryList(QDir::Dirs);
    dirList.removeOne(".");
    dirList.removeOne("..");

    for (const QString &skinId : dirList)
    {
        if (isValidSkin(skinId))
        {
            m_skinIds << skinId;
        }
    }

    std::vector<std::string> skinVec;
    skinVec.reserve(static_cast<size_t>(m_skinIds.size()));
    for (const QString &id : m_skinIds)
    {
        skinVec.push_back(fromStdUtf8(id));
    }
    freewb_runtime_set_skin_list(skinVec);
}

QStringList Skin::availableSkinIds() const
{
    return m_skinIds;
}

QString Skin::skinName(const QString &skinId) const
{
    const QString iniPath = skinFolder(skinId) + "skin.ini";
    if (!QFile(iniPath).exists())
    {
        return skinId;
    }

    QSettings settings(iniPath, QSettings::IniFormat);
    setSkinIniCodec(settings);
    settings.beginGroup("Skin");
    const QString name = settings.value("name").toString();
    settings.endGroup();

    if (!name.isEmpty())
    {
        return name;
    }
    if (skinId == QLatin1String("default"))
    {
        return QStringLiteral("经典皮肤");
    }
    return skinId;
}

QString Skin::currentSkinId() const
{
    return m_curSkinId;
}

const SkinInputWin &Skin::inputWin() const
{
    return m_inputWin;
}

const SkinToolBar &Skin::toolbar() const
{
    return m_toolbar;
}

bool Skin::inputWinUsesImageBorder() const
{
    return m_inputWin.bgTopImageHeight > 0 || m_inputWin.bgBottomImageHeight > 0 || m_inputWin.bgLeftImageWidth > 0 ||
           m_inputWin.bgRightImageWidth > 0;
}

QString Skin::inputWinFrameBgStyle(int radius) const
{
    if (inputWinUsesImageBorder())
    {
        return {};
    }

    const QColor borderColor = skinQColorFromSpec(settings::instance().get_borderColor());
    const QColor bgColor = skinQColorFromSpec(settings::instance().get_bgColor());
    const QColor gradientColor0 = skinQColorFromSpec(settings::instance().get_gradientColor0());
    const QColor gradientColor1 = skinQColorFromSpec(settings::instance().get_gradientColor1());

    const QString borderColorStyle =
        QString("border-color:rgb(%1,%2,%3);").arg(borderColor.red()).arg(borderColor.green()).arg(borderColor.blue());

    if (settings::instance().get_useGradientColor())
    {
        const QString gradientColorStyle = QString("background-color:qlineargradient(spread:pad,x1:0, y1:0, x2:0, y2:1,stop:0 "
                                                   "rgb(%1,%2,%3),stop:1 rgb(%4,%5,%6));")
                                               .arg(gradientColor0.red())
                                               .arg(gradientColor0.green())
                                               .arg(gradientColor0.blue())
                                               .arg(gradientColor1.red())
                                               .arg(gradientColor1.green())
                                               .arg(gradientColor1.blue());
        return QString("#frameBg{"
                       "border-width:1px;"
                       "border-style:solid;"
                       "border-radius:%1px;"
                       "%2"
                       "%3"
                       "}")
            .arg(radius)
            .arg(borderColorStyle)
            .arg(gradientColorStyle);
    }

    if (settings::instance().get_useBgImage())
    {
        const QString bgImageStyle = QString("%1:url(%2);")
                                         .arg(settings::instance().get_enableTiled() ? "background-image" : "border-image")
                                         .arg(toQStringUtf8(settings::instance().get_bgImage()));
        return QString("#frameBg{"
                       "border-width:1px;"
                       "border-style:solid;"
                       "border-radius:%1px;"
                       "%2"
                       "%3"
                       "}")
            .arg(radius)
            .arg(borderColorStyle)
            .arg(bgImageStyle);
    }

    return QString("#frameBg{"
                   "border-width:1px;"
                   "border-style:solid;"
                   "border-radius:%1px;"
                   "%2"
                   "background-color:rgb(%3,%4,%5);"
                   "}")
        .arg(radius)
        .arg(borderColorStyle)
        .arg(bgColor.red())
        .arg(bgColor.green())
        .arg(bgColor.blue());
}

QString Skin::toolbarPreviewPath(const QString &skinId) const
{
    return skinFolder(skinId) + "toolbar.png";
}

void Skin::loadInputWinSkin(QSettings &settings, const QString &skinFolder)
{
    settings.beginGroup("CandidateWin");
    m_inputWin.bgTopImageHeight = settings.value("bgTopImgHeight").toInt();
    m_inputWin.bgBottomImageHeight = settings.value("bgBottomImgHeight").toInt();
    m_inputWin.bgLeftImageWidth = settings.value("bgLeftImgWidth").toInt();
    m_inputWin.bgRightImageWidth = settings.value("bgRightImgWidth").toInt();
    m_inputWin.bgCenterImagePath = skinFolder + settings.value("bgCenterImg").toString();
    m_inputWin.bgTopImagePath = skinFolder + settings.value("bgTopImg").toString();
    m_inputWin.bgBottomImagePath = skinFolder + settings.value("bgBottomImg").toString();
    m_inputWin.bgLeftImagePath = skinFolder + settings.value("bgLeftImg").toString();
    m_inputWin.bgRightImagePath = skinFolder + settings.value("bgRightImg").toString();
    m_inputWin.fullIcoPath = skinFolder + settings.value("fullIco").toString();
    m_inputWin.halfIcoPath = skinFolder + settings.value("halfIco").toString();
    m_inputWin.cnMarkIcoPath = skinFolder + settings.value("cnMarkIco").toString();
    m_inputWin.enMarkIcoPath = skinFolder + settings.value("enMarkIco").toString();
    m_inputWin.logoIcoPath = QString(FREEWB_INSTALL_PKGDATADIR) + "/skin/freewb.png";
    m_inputWin.prev0PageIcoPath = skinFolder + settings.value("prev0PageIco").toString();
    m_inputWin.prev1PageIcoPath = skinFolder + settings.value("prev1PageIco").toString();
    m_inputWin.next0PageIcoPath = skinFolder + settings.value("next0PageIco").toString();
    m_inputWin.next1PageIcoPath = skinFolder + settings.value("next1PageIco").toString();
    settings.endGroup();
}

void Skin::loadToolbarSkin(QSettings &settings, const QString &skinFolder)
{
    settings.beginGroup("ToolbarBg");
    m_toolbar.size0 = settings.value("size0").toSize();
    m_toolbar.size1 = settings.value("size1").toSize();
    m_toolbar.bg0ImagePath = skinFolder + settings.value("bgImg0").toString();
    m_toolbar.bg1ImagePath = skinFolder + settings.value("bgImg1").toString();
    settings.endGroup();

    settings.beginGroup("ToolbarBtn");

    m_toolbar.stbMenuExtendBtn.isExist = settings.value("btnExtMenuFlg").toInt();
    m_toolbar.stbMenuExtendBtn.rect = settings.value("btnExtMenuGeometry").toRect();
    m_toolbar.stbMenuExtendBtn.openIcoPath = skinFolder + settings.value("btnExtMenuOpenImg").toString();
    m_toolbar.stbMenuExtendBtn.closeIcoPath = skinFolder + settings.value("btnExtMenuClosedImg").toString();

    m_toolbar.stbLogoBtn.isExist = settings.value("btnLogoFlg").toInt();
    m_toolbar.stbLogoBtn.rect = settings.value("btnLogoGeometry").toRect();
    m_toolbar.stbLogoBtn.icoPath = skinFolder + settings.value("btnLogoImg").toString();

    m_toolbar.stbModeBtn.isExist = settings.value("btnModeFlg").toInt();
    m_toolbar.stbModeBtn.rect = settings.value("btnModeGeometry").toRect();
    m_toolbar.stbModeBtn.wbFontIcoPath = skinFolder + settings.value("btnModewbFontImg").toString();
    m_toolbar.stbModeBtn.wbPinyinIcoPath = skinFolder + settings.value("btnModewbPyImg").toString();
    m_toolbar.stbModeBtn.stdPinyinIcoPath = skinFolder + settings.value("btnModeStdPyImg").toString();
    m_toolbar.stbModeBtn.englishIcoPath = skinFolder + settings.value("btnModeEnglishImg").toString();
    m_toolbar.stbModeBtn.capsIcoPath = skinFolder + settings.value("btnModeCapsImg").toString();

    m_toolbar.stbFullHalfBtn.isExist = settings.value("btnCharWidthFlg").toInt();
    m_toolbar.stbFullHalfBtn.rect = settings.value("btnCharWidthGeometry").toRect();
    m_toolbar.stbFullHalfBtn.fullIcoPath = skinFolder + settings.value("btnCharWidthFullImg").toString();
    m_toolbar.stbFullHalfBtn.halfIcoPath = skinFolder + settings.value("btnCharWidthHalfImg").toString();

    m_toolbar.stbCnEnMarkBtn.isExist = settings.value("btnMarkFlg").toInt();
    m_toolbar.stbCnEnMarkBtn.rect = settings.value("btnMarkGeometry").toRect();
    m_toolbar.stbCnEnMarkBtn.cnMarkIcoPath = skinFolder + settings.value("btnMarkCnImg").toString();
    m_toolbar.stbCnEnMarkBtn.enMarkIcoPath = skinFolder + settings.value("btnMarkEnImg").toString();

    m_toolbar.stbSettingBtn.isExist = settings.value("btnSettingFlg").toInt();
    m_toolbar.stbSettingBtn.rect = settings.value("btnSettingGeometry").toRect();
    m_toolbar.stbSettingBtn.icoPath = skinFolder + settings.value("btnSettingCnImg").toString();

    m_toolbar.stbGenerateBtn.isExist = settings.value("btnGenerateFlg").toInt();
    m_toolbar.stbGenerateBtn.rect = settings.value("btnGenerateGeometry").toRect();
    m_toolbar.stbGenerateBtn.icoPath = skinFolder + settings.value("btnGenerateImg").toString();

    m_toolbar.stbSearchBtn.isExist = settings.value("btnSearchFlg").toInt();
    m_toolbar.stbSearchBtn.rect = settings.value("btnSearchGeometry").toRect();
    m_toolbar.stbSearchBtn.icoPath = skinFolder + settings.value("btnSearchImg").toString();

    m_toolbar.stbCharFontBtn.isExist = settings.value("btnCharFontFlg").toInt();
    m_toolbar.stbCharFontBtn.rect = settings.value("btnCharFontGeometry").toRect();
    m_toolbar.stbCharFontBtn.simpIcoPath = skinFolder + settings.value("btnCharFontSimpImg").toString();
    m_toolbar.stbCharFontBtn.tradIcoPath = skinFolder + settings.value("btnCharFontTradImg").toString();

    m_toolbar.stbCharSetBtn.isExist = settings.value("btnCharSetFlg").toInt();
    m_toolbar.stbCharSetBtn.rect = settings.value("btnCharSetGeometry").toRect();
    m_toolbar.stbCharSetBtn.gbIcoPath = skinFolder + settings.value("btnCharSetGbImg").toString();
    m_toolbar.stbCharSetBtn.gbkIcoPath = skinFolder + settings.value("btnCharSetGbkImg").toString();

    m_toolbar.stbKeyboardBtn.isExist = settings.value("btnKeyboardFlg").toInt();
    m_toolbar.stbKeyboardBtn.rect = settings.value("btnKeyboardGeometry").toRect();
    m_toolbar.stbKeyboardBtn.icoPath = skinFolder + settings.value("btnKeyboardImg").toString();

    settings.endGroup();
}

bool Skin::load(const QString &skinId)
{
    if (m_curSkinId == skinId && isValidSkin(skinId))
    {
        return true;
    }

    const QString folder = skinFolder(skinId);
    if (!QFile(folder + "skin.ini").exists())
    {
        return false;
    }

    QSettings settings(folder + "skin.ini", QSettings::IniFormat);
    setSkinIniCodec(settings);
    loadInputWinSkin(settings, folder);
    loadToolbarSkin(settings, folder);

    m_curSkinId = skinId;
    return true;
}

bool Skin::loadFromSettings()
{
    return load(toQStringUtf8(settings::instance().get_curSkinId()));
}

bool Skin::switchTo(const QString &skinId, bool persist)
{
    if (!load(skinId))
    {
        return false;
    }

    if (persist)
    {
        settings::instance().set_curSkinId(fromStdUtf8(skinId));
    }

    emit skinChanged(skinId);
    return true;
}
