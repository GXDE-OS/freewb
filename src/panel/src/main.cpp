#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#include <QApplication>
#include <QFontDatabase>
#include <QMutex>

#include "config.h"
#include "log.h"
#include "mainprogram.h"
#include "settings.h"

static void run_as_daemon()
{
    typedef void (*freewb_sighandler_t)(int);

    pid_t pid;
    char workPath[1024];
    Q_UNUSED(getcwd(workPath, 1024));

    if ((pid = fork()) > 0)
    {
        waitpid(pid, nullptr, 0);
        exit(0);
    }

    setsid();

    freewb_sighandler_t oldint = signal(SIGINT, SIG_IGN);
    freewb_sighandler_t oldhup = signal(SIGHUP, SIG_IGN);
    freewb_sighandler_t oldquit = signal(SIGQUIT, SIG_IGN);
    freewb_sighandler_t oldpipe = signal(SIGPIPE, SIG_IGN);
    freewb_sighandler_t oldttou = signal(SIGTTOU, SIG_IGN);
    freewb_sighandler_t oldttin = signal(SIGTTIN, SIG_IGN);
    freewb_sighandler_t oldchld = signal(SIGCHLD, SIG_IGN);

    if (fork() > 0)
    {
        exit(0);
    }

    Q_UNUSED(chdir(workPath));

    signal(SIGINT, oldint);
    signal(SIGHUP, oldhup);
    signal(SIGQUIT, oldquit);
    signal(SIGPIPE, oldpipe);
    signal(SIGTTOU, oldttou);
    signal(SIGTTIN, oldttin);
    signal(SIGCHLD, oldchld);
}

static bool app_is_running()
{
    char lockFile[] = "/tmp/fcitx-freewb.pid";
    char buf[128];
    struct flock fl;

    int fd = open(lockFile, O_RDWR | O_CREAT, 0);
    if (fd < 0)
    {
        sprintf(buf, "can not open %s", lockFile);
        perror(buf);
        fflush(stderr);
        exit(1);
    }

    chmod(lockFile, 0777);

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    if (fcntl(fd, F_SETLK, &fl) < 0)
    {
        perror("app is running!");
        fflush(stderr);
        return true;
    }

    Q_UNUSED(ftruncate(fd, 0));
    sprintf(buf, "%d\n", getpid());
    Q_UNUSED(write(fd, buf, strlen(buf)));

    return false;
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(FREEWB_TEXT_DOMAIN, FREEWB_INSTALL_LOCALEDIR);
    bind_textdomain_codeset(FREEWB_TEXT_DOMAIN, "UTF-8");
    textdomain(FREEWB_TEXT_DOMAIN);

    FreewbLog log("/tmp/freewb-ui-panel.log");
    FREEWB_DEBUG("panel started");

    run_as_daemon();
    if (app_is_running())
    {
        exit(1);
    }

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName("freewb");

    settings::instance().reload();

    MainProgram w;

    return app.exec();
}
