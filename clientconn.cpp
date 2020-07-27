#include "clientconn.h"
#include<QVector>

Clientconn::Clientconn(int socketDescriptor, Server * server) : server(server)
{
    tcpSock = new QTcpSocket();
    tcpSock->setSocketDescriptor(socketDescriptor);
    connect(tcpSock, &QTcpSocket::readyRead, this, &Clientconn::readAnyMessage);
    qDebug() << "Someone's in";
}

Clientconn::~Clientconn(){}

void Clientconn::readAnyMessage() {

        QDataStream in(tcpSock);
        in.setVersion(QDataStream::Qt_4_0);
        int op;
        QString username;
        QString password;
        QString nickname;
        QString filename;
        QString answer;
        User user;
        Message msg;
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);

        qDebug() << "receiving some...";
        do {
            in.startTransaction();
            in >> op;
                switch(op) {
                case 'l':
                    in >> username;
                    in >> password;
                    if(in.commitTransaction()){
                        uniqueId = server->checkCredentials(username, password);
                        //qDebug() << "UniqueId = " + QString(uniqueId);
                        out << op;
                        out << uniqueId;
                        if (uniqueId != 0) {
                            isLoggedIn = true;
                            clientPro = server->getProfile(uniqueId);
                            out << server->retrieveFiles();
                        }
                        tcpSock->write(block);
                    }
                    break;
                case 's':
                    in >> username;
                    in >> password;
                    in >> nickname;
                    if(in.commitTransaction()){
                        uniqueId = server->registerUser(username, password, nickname);
                        //qDebug() << "UniqueId = " + QString(uniqueId);
                        out << op;
                        out << uniqueId;
                        if (uniqueId != 0) {
                            isLoggedIn = true;
                            clientPro = server->getProfile(uniqueId);
                            out << server->retrieveFiles();
                        }
                        tcpSock->write(block);
                    }
                    break;
                case 'o':
                    in >> filename;
                    if(in.commitTransaction())
                        workingOn = server->openFile(filename, this);
                    break;
                case 'n':
                    in >> filename;
                    if(in.commitTransaction())
                        workingOn = server->newFile(filename, this);
                    break;
                case 'u':
                    in >> user;
                    in >> username;
                    in >> password;
                    if(in.commitTransaction()) {
                        server->updateUser(user, uniqueId, username, password);
                        workingOn->relay(user);
                    }
                    break;
                case 'm':
                    in >> msg;
                    if(in.commitTransaction()) {
                        workingOn->process(msg);
                    }
                    break;
                default:
                    in.rollbackTransaction();
                }
            if(in.status() == QDataStream::Ok)
                qDebug() << "received '" << char(op) << "' message";

        } while(in.status() == QDataStream::Ok);
}

QTcpSocket* Clientconn::getTcpSock() {
    return this->tcpSock;
}
