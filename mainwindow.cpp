#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QFile>
#include<QStorageInfo>
#include<QThread>
#include<QRegularExpression>
#include<QTimer>
#include<sys/sysinfo.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    connect(timer, &QTimer::timeout,
            this, &MainWindow::updateProgress);

    // Démarrer le timer (toutes les 1000 ms)
    timer->start(1000);
    updateProgress();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::updateProgress()
{

    int ram = getRamUsage() ;
    int cpu = getCpuUsage() ;
    int disk = getDiskUsage() ;
    ui->ramProgress->setValue(ram);
    ui->diskProgress->setValue(disk) ;
    ui->cpuProgress->setValue(cpu) ;

}

int MainWindow::getCpuUsage()
{
    static unsigned long long prevTotal = 0, prevIdle = 0;
    QFile file("/proc/stat");
    if(!file.open(QIODevice::ReadOnly)) return 0;

    QByteArray line = file.readLine();
    file.close();

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(line.data(), "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    unsigned long long idleTime = idle + iowait;

    if(prevTotal == 0) {
        prevTotal = total;
        prevIdle = idleTime;
        return 0;
    }

    unsigned long long totalDiff = total - prevTotal;
    unsigned long long idleDiff = idleTime - prevIdle;
    prevTotal = total;
    prevIdle = idleTime;

    if(totalDiff == 0) return 0;
    return (int)(100.0 * (totalDiff - idleDiff) / totalDiff);
}

int MainWindow::getRamUsage()
{
    QFile file("/proc/meminfo");
    if(!file.open(QIODevice::ReadOnly)) return 0;

    QString data = file.readAll();
    file.close();

    QRegularExpression memTotalRe("MemTotal:\\s+(\\d+)");
    QRegularExpression memAvailableRe("MemAvailable:\\s+(\\d+)");

    qlonglong total = memTotalRe.match(data).captured(1).toLongLong();
    qlonglong available = memAvailableRe.match(data).captured(1).toLongLong();

    if(total == 0) return 0;
    return (int)(100 - (available * 100.0 / total));
}

int MainWindow::getDiskUsage()
{
    QStorageInfo storage = QStorageInfo::root();
    qint64 total = storage.bytesTotal();
    qint64 used = total - storage.bytesFree();

    if(total == 0) return 0;
    return (int)((used * 100) / total);
}
