#ifndef CLIENTCONN_H
#define CLIENTCONN_H
#include <QStringList>
#include <QTcpServer>
#include <QColor>
#include <QTextEdit>
#include <QFile>
#include <QtNetwork>
#include "user.h"
#include "server.h"
#include "document.h"

class Server;
class Document;
class Clientconn : public QObject {
    Q_OBJECT
public:
    Clientconn(int sd, Server * server);
    ~Clientconn();
    User clientPro;
    quint32 uniqueId;
    Server * server = 0;
    int socketDescriptor = 0;
    bool isLoggedIn = false;
    Document * workingOn = 0;
    QTcpSocket * getTcpSock();

private slots:
    void readAnyMessage();

private:
    QTcpSocket * tcpSock = 0;
};

#endif // CLIENTCONN_H
