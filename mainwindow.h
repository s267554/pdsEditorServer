#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include "mythread.h"
#include <QTcpServer>
#include <QSqlQuery>
#include <QThread>

namespace Ui {
class MainWindow;
}

class FileWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit FileWindow(QWidget *parent = 0 );
    void incomingConnection(qintptr socketDescriptor); // messo
    ~FileWindow();
    bool setUpConnection();


 signals:
    void error(QTcpSocket::SocketError socketerror);
    void quitWork(void);
    void readyRead();

  public slots:
    void onNewConnection();
    void disconnected();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void workerResult(QTcpSocket* socketToRead);


private:
    Ui::MainWindow *ui;
    QTcpServer _server;
    QTcpSocket* server_socket;
    QTcpServer* server_to_rec;
    QTcpSocket* client_socket_to_rec;
    QList<QTcpSocket*> _sockets;
    QHash<QTcpSocket*, QByteArray*> buffers;
    QHash<QTcpSocket*, qint32*> sizes;
    QTcpSocket* socketID;
    AbstractWorker *m_worker;
    QThread m_thread;
    QSqlDatabase db;

};

#endif // MAINWINDOW_H
