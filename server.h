#ifndef SERVER_H
#define SERVER_H
#include <QStringList>
#include <QTcpServer>
#include <QColor>
#include <QTextEdit>
#include <QFile>
#include <QtNetwork>
#include <QSqlDatabase>
#include "document.h"
#include "clientconn.h"
#include "account.h"
#include "message.h"
#include "user.h"

class Document;
class ClientConn;
class Server : public QTcpServer
{
    Q_OBJECT
public:

    Server(QObject *parent = 0);
    quint32 checkCredentials(QString username, QString password);
    quint32 registerUser(QString username, QString password, QString nickname);
    QStringList retrieveFiles();
    Document* openFile(QString fname, Clientconn* client);
    Document* newFile(QString fname, Clientconn* client);
    void updateUser(User user, quint32 uid, QString username, QString password);
    User getProfile(quint32);
    bool getDb();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QStringList _filenames;
    QMap<QString, Document *> _opendocs;
    QVector<Account> _accounts;
    QMap<quint32, User> _profiles;
    QThread m_thread;
    QList<QTcpSocket*> _sockets;
    QSqlDatabase db;

};
template<class T>
QDataStream &operator<<(QDataStream& stream, const std::vector<T>& val){
    stream << static_cast<quint32>(val.size());
    for(auto& singleVal : val)
        stream << singleVal;
    return stream;
}

template<class T>
QDataStream &operator>>(QDataStream& stream, std::vector<T>& val){
    quint32 vecSize;
    val.clear();
    stream >> vecSize;
    val.reserve(vecSize);
    T tempVal;
    while(vecSize--){
        stream >> tempVal;
        val.push_back(tempVal);
    }
    return stream;
}
#endif
