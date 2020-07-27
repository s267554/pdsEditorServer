
#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QThread>
#include <QDebug>
#include <QTcpSocket>
#include "abstractworker.h"
#include "workerlogin.h"

class MyThread : public QThread
{
    Q_OBJECT
    public:
    explicit MyThread();
    explicit MyThread( QTcpSocket* ID , QObject *parent = nullptr);
    explicit MyThread(int iID, QObject *parent = 0);
    void run();
    virtual ~MyThread();
    void stop();
    QString name;


 public slots:
    void onReadyRead();
    void onDisconnected();
    virtual void startVariable();

  signals:
    void error(QTcpSocket::SocketError socketerror);
    void _resultLogin();
    void readyRead();
    void start();

 private:
    QTcpSocket *socket;
    QTcpSocket *socketAttivi;
    qintptr socketDescriptor;
    QThread *m_thread;
    WorkerLogin *m_worker;

    /** variabile che ci dice se il thread e' acceduto da altri **/
    volatile bool stopped;
    volatile bool started;
};
#endif
