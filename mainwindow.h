#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnected();
    void attemptReconnect();
    void onSocketError(QAbstractSocket::SocketError);
    void readData();

private:
    void connectToServer();

    Ui::MainWindow *ui {};
    QTcpSocket     *socket {};
    QTimer         *reconnectTimer {};
    QByteArray      buffer;
};

#endif
