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
#include <QDebug>
#include <QFontDatabase>
#include <QMutex>

#include "commdefine.h"
#include "mainprogram.h"
#include "settings.h"

CpuType g_cpuType = CT_X86;
DesktopType g_desktopType = DT_UBUNTU;

static CpuType get_cpu_type()
{
    CpuType type = CT_X86;
    struct utsname info;

    if (uname(&info) != 0)
    {
        qWarning() << "get cpu arch failed!";
    }
    else
    {
        QString arch = QString(info.machine);
        if (arch.contains("86"))
        {
            type = CT_X86;
        }
        else if (arch.contains("aarch64"))
        {
            type = CT_ARM;
        }
    }

    return type;
}

static DesktopType get_desktop_type()
{
    DesktopType type = DT_UBUNTU;

    QString desktopUi = QString(qgetenv("DESKTOP_SESSION"));
    if (desktopUi.contains("ubuntu"))
    {
        type = DT_UBUNTU;
    }
    else if (desktopUi.contains("mate"))
    {
        type = DT_MATE;
    }
    else if (desktopUi.contains("ukui"))
    {
        type = DT_UKUI;
    }
    else if (desktopUi.contains("deepin"))
    {
        type = DT_DEEPIN;
    }

    return type;
}

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

static QFile logFile;
static QTextStream logStream;
static QMutex logMutex;

void qdebug_msg_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    logMutex.lock();

    if (!logFile.exists() && logFile.isOpen())
    {
        logFile.close();
        logFile.open(QIODevice::Append | QIODevice::Text);
    }

#define LOF_FILE_MAX_SIZE (1024 * 1024 * 8) // 8MB
    if (logFile.size() > LOF_FILE_MAX_SIZE)
    {
        logFile.resize(0);
    }

    QString lodMsgData, logType;
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type)
    {
    case QtDebugMsg:
        logType = "Debug";
        break;
    case QtInfoMsg:
        logType = "Info";
        break;
    case QtWarningMsg:
        logType = "Warning";
        break;
    case QtCriticalMsg:
        logType = "Critical";
        break;
    case QtFatalMsg:
        logType = "Fatal";
        break;
    }

    lodMsgData = QString("\n[%1][%2](%3:%4:%5) %6").arg(QDateTime::currentDateTime().toString("yyyyMMdd HH:mm:ss zzz")).arg("Debug").arg(file).arg(function).arg(context.line).arg(msg);

    logStream << lodMsgData;
    logStream.flush();

    logMutex.unlock();
}

int main(int argc, char *argv[])
{
#ifdef BUILD_IN_CAMKE
    run_as_daemon(); // 将程序初始化为后台守护进程
#endif
    if (app_is_running())
    {
        exit(1);
    }

    qSetMessagePattern("[%{time hh:mm:ss zzz}](%{function}): %{message}");
#ifdef BUILD_IN_CAMKE
    logFile.setFileName((QString(qgetenv("HOME")) + "/.local/freewb/log.txt"));
    logStream.setDevice(&logFile);
    logStream.setCodec("UTF-8");
    if (!logFile.open(QIODevice::Append | QIODevice::Text))
    {
        qWarning() << logFile.fileName() << "open failed!";
    }
    else
    {
        qInstallMessageHandler(qdebug_msg_handler);
    }
#endif
    g_cpuType = get_cpu_type();
    g_desktopType = get_desktop_type();

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName("Freewb");

    Settings::init_const_data_member();
    Settings::load_all_setting_data_from_file();

    MainProgram w;

    return app.exec();
}
