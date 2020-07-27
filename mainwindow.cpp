#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <workerfile.h>

#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtSql>
#include <QStandardItemModel>
#include <QFileInfo>

static inline qint32 ArrayToInt(QByteArray source);



FileWindow::FileWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _server(this)
{



    ui->setupUi(this);
    qDebug() << " Server is Listen to host: ";
    _server.listen(QHostAddress::Any, 4242);


    /** TcpServer -> dispone del metodo newConncetion() **/
    connect(&_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

  }




bool FileWindow::setUpConnection() {
    bool dbExist = false;
    qDebug() << " SetUp connection: ";
    db.QSqlDatabase::addDatabase("QSQLITE");
    //db.setHostName("bigblue");
    // db.setUserName("acarlson");
    //db.setPassword("1uTbSbAs");
    db.setDatabaseName("C:/sqlLite/testDd.sqlite");

    if (db.isOpen()) {
        qDebug() << " Start DB ";
        dbExist = true;
    } else {
        qDebug() << "Cant Find DB";
        dbExist = false;
    }

    return dbExist;
}



FileWindow::~FileWindow()
{
    delete ui;
}




void FileWindow::onNewConnection()
{

    qDebug() << "main threadID: inside constructor: " << m_thread.currentThreadId();

    /** Arriva Client i-esimo **/
    client_socket_to_rec = _server.nextPendingConnection();
    while(!client_socket_to_rec) {
        qDebug() << "Client try to Connect to Server";
        client_socket_to_rec =_server.nextPendingConnection();
    }


    qDebug() << "Create a new Socket Connection to Host: Socket "<< client_socket_to_rec<<" created" ;
    /** 0) Lista di attesa di socket senza worker **/


    /** 1) HANDSAKE
     * Scorro la lista di tutti i nuovi Socket: Invio saluto a tutti i Client **/
    _sockets.push_back(client_socket_to_rec);
    for (QTcpSocket* socket : _sockets) {

        /** Sending New Client Name **/
        socket -> write(QByteArray::fromStdString(client_socket_to_rec->
                                                  peerAddress().toString().toStdString() +
                                                  " Hello, Client is ON !\n"));
    }


    /** 2) Gestione del login: socket messi in una lista di richieste login
            * crea -> lista loggati NB:
            * Due thread non devono accedere contemporaneamente sulla stessa risorsa
         altrimenti vanno in Deadlock **/


        MyThread* thread = new MyThread();
        thread->name = "Login thread";

        MyThread* thread2 = new MyThread();
        thread2->name = "Word thread";


        WorkerLogin* loginWorker = new WorkerLogin();
        WorkerFile* fileWorker = new WorkerFile();


        loginWorker->_setDbConnection(db);
        loginWorker->moveToThread(thread->thread());
        loginWorker->socketDescriptor = client_socket_to_rec->socketDescriptor();
        loginWorker->setSocket(client_socket_to_rec);
        connect(thread, SIGNAL(started()), loginWorker, SLOT(_loginManager()));


        /** 4.1) Attende la risposta del client WorkerEditor -> un file scelto in particolare
                    * lista soket editor **/
        connect(thread, SIGNAL(_resultLogin()), loginWorker, SLOT(_invioFileSystem()));


        /** 4.2) Attivo Worker CRDT -> uno per socket
                * lista soket per file -> un file per socket **/
        connect(thread2, SIGNAL(readyRead()), fileWorker, SLOT(readData()));


        connect(loginWorker, SIGNAL( finished()), loginWorker, SLOT(deleteLater()));
        connect(loginWorker, SIGNAL(finished()), thread, SLOT(quit()));

        connect(fileWorker, SIGNAL( finished()), fileWorker, SLOT(deleteLater()));
        connect(fileWorker, SIGNAL(finished()), thread2, SLOT(quit()));

        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(thread2, SIGNAL(finished()), thread, SLOT(deleteLater()));



    thread->start(QThread::LowestPriority);
    thread2->start(QThread::HighestPriority);
    qDebug() << "start thread ";


}






