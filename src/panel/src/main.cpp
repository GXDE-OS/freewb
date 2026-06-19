#include <QApplication>

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

    QApplication app(argc, argv);
    app.setApplicationName("freewb");

    settings::instance().reload();

    MainProgram w;

    return app.exec();
}
