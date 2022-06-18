#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <stdio.h>
#include "Logger.h"
#include "MainWindow.h"

void printHelp()
{
    printf("Usage:  XMonitor -f ~/config.yml -d\n");
    printf("\t-f: Config File Path\n");
    printf("\t-a: Account\n");
    printf("\t-d: log debug mode, print debug log\n");
    printf("\t-h: print help infomartion\n");
}

int main(int argc, char *argv[])
{
    std::string configPath = "./XMonitor.yml";
    int ch;
    bool debug = false;
    while ((ch = getopt(argc, argv, "f:a:dh")) != -1)
    {
        switch (ch)
        {
        case 'f':
            configPath = optarg;
            break;
        case 'a':
            break;
        case 'd':
            debug = true;
            break;
        case 'h':
        case '?':
        case ':':
        default:
            printHelp();
            exit(-1);
            break;
        }
    }
    std::string app_log_path;
    char* p = getenv("APP_LOG_PATH");
    if(p == NULL)
    {
        app_log_path = "./log/";
    }
    else
    {
        app_log_path = p;
    }
    std::string cmd;
    for(int i = 0; i < argc; i++)
    {
        cmd += (std::string(argv[i]) + " ");
    }
    Utils::gLogger = Utils::Singleton<Utils::Logger>::GetInstance();
    Utils::gLogger->setLogPath(app_log_path, "XMonitor");
    Utils::gLogger->Init();
    Utils::gLogger->setDebugLevel(debug);
    Utils::gLogger->Log->info(cmd);
    Utils::gLogger->Log->info("XMonitor AppCommitID:{} BranchName:{}", APP_COMMIT_ID, APP_BRANCH_NAME);

    QApplication a(argc, argv);
    MainWindow w(configPath);
    qApp->setPalette(QPalette(QColor("#19232D")));
    w.show();

    return a.exec();
}
