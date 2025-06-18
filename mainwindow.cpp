#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

static constexpr const char* HOST_IP   = "127.0.0.1";
static constexpr quint16     HOST_PORT = 6000;

// ----------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(new QTcpSocket(this))
    , reconnectTimer(new QTimer(this))
{
    ui->setupUi(this);

    connect(socket, &QTcpSocket::connected,    this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::attemptReconnect);
    connect(socket, &QTcpSocket::readyRead,    this, &MainWindow::readData);
    connect(socket, &QTcpSocket::errorOccurred,this, &MainWindow::onSocketError);

    reconnectTimer->setInterval(3000);
    connect(reconnectTimer, &QTimer::timeout,  this, &MainWindow::connectToServer);

    connectToServer();
}

MainWindow::~MainWindow()
{
    if (socket) {
        socket->disconnect(this);
        socket->close();
    }
    delete ui;
}


void MainWindow::connectToServer()
{
    if (socket->state() != QAbstractSocket::UnconnectedState) return;
    qDebug() << u8"🔄 bağlanılıyor:" << HOST_IP << HOST_PORT;
    socket->connectToHost(HOST_IP, HOST_PORT);
}

void MainWindow::onConnected()
{
    reconnectTimer->stop();
    qDebug() << u8"✅ TCP bağlandı";
}

void MainWindow::attemptReconnect()
{
    qDebug() << u8"⚠️  bağlantı koptu — 3 sn sonra tekrar denenecek";
    reconnectTimer->start();
}

void MainWindow::onSocketError(QAbstractSocket::SocketError)
{
    qDebug() << u8"❌ Socket hatası:" << socket->errorString();
}


void MainWindow::readData()
{
    buffer.append(socket->readAll());

    while (true) {
        int idx = buffer.indexOf('\n');
        if (idx < 0) break;

        QByteArray line = buffer.left(idx).trimmed();
        buffer.remove(0, idx + 1);

        double lat = 0, lon = 0, alt = 0;
        double gspd = 0, vspd = 0, vbatt = 0;

        const QList<QByteArray> parts = line.split(';');
        for (const QByteArray& p : parts) {
            if      (p.startsWith("lat="))   lat   = p.mid(4).toDouble();
            else if (p.startsWith("lon="))   lon   = p.mid(4).toDouble();
            else if (p.startsWith("alt="))   alt   = p.mid(4).toDouble();
            else if (p.startsWith("gspd="))  gspd  = p.mid(5).toDouble();
            else if (p.startsWith("vspd="))  vspd  = p.mid(5).toDouble();
            else if (p.startsWith("vbatt=")) vbatt = p.mid(6).toDouble();
        }

        ui->labelLatitude      ->setText(QString("LAT:  %1").arg(lat,  0,'f',7));
        ui->labelLongitude     ->setText(QString("LON:  %1").arg(lon,  0,'f',7));
        ui->labelAltitude      ->setText(QString("ALT:  %1 m").arg(alt,0,'f',1));
        ui->labelGroundSpeed   ->setText(QString("GSPD: %1 m/s").arg(gspd,0,'f',2));
        ui->labelVerticalSpeed ->setText(QString("VSPD: %1 m/s").arg(vspd,0,'f',2));
        ui->labelBattery       ->setText(QString("VBAT: %1 V").arg(vbatt,0,'f',2));
    }
}
