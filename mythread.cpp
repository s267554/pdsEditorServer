#include "mythread.h"
#include <QThreadPool>
#include <QDataStream>

MyThread::MyThread()
{

}

MyThread::MyThread(int ID, QObject *parent)
{
    this->socketDescriptor = ID;
}

MyThread::~MyThread()
{
    m_thread->quit();
    m_thread->wait();
    m_thread->deleteLater();
}


void MyThread::stop()
{
 stopped = true;
}


void MyThread::startVariable()
{
  started = true;
}

void MyThread::run() {
    if (!stopped) {
        qDebug() << "Running " + this->name + "From worker thread: " <<currentThreadId();
   }
}

void MyThread::onReadyRead() {
    QByteArray Data = socket ->readAll();
    qDebug() << socketDescriptor << " Data in: " << Data;
    socket->write(Data);
}

void MyThread::onDisconnected() {
    qDebug() << socketDescriptor << " Disconnected: ";
    socket->deleteLater();
}

