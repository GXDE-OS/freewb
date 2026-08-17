#include <QApplication>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "config.h"
#include "log.h"
#include "mainprogram.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(FREEWB_TEXT_DOMAIN, FREEWB_INSTALL_LOCALEDIR);
    bind_textdomain_codeset(FREEWB_TEXT_DOMAIN, "UTF-8");
    textdomain(FREEWB_TEXT_DOMAIN);

    FreewbLog log("/tmp/freewb-ui-panel.log");
    FREEWB_DEBUG("panel started");

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    QApplication app(argc, argv);
    app.setApplicationName("freewb");
    app.setQuitOnLastWindowClosed(false);

    QTranslator qtTranslator;
    const QString qtTrPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    const QString localeName = QLocale::system().name();
    // Qt 5.15+ 拆分为 qtbase_*.qm；Qt 5.12简体中文仍在 qt_zh_CN.qm，无 qtbase_zh_CN.qm
    if (qtTranslator.load("qtbase_" + localeName, qtTrPath) || qtTranslator.load("qt_" + localeName, qtTrPath))
    {
        FREEWB_DEBUG("loaded qt translator: %s", localeName.toStdString().c_str());
        app.installTranslator(&qtTranslator);
    }
    else
    {
        FREEWB_WARN("failed to load qt translator: %s", localeName.toStdString().c_str());
    }

    settings::instance().reload();

    MainProgram w;

    return app.exec();
}
