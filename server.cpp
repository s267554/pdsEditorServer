#include "server.h"
#include "clientconn.h"
#include"document.h"
#include <QDataStream>
#include <QMessageBox>
#include <QSqlQuery>

#define Path_to_DB "/Users/deusex/Documents/Github/PDSEditorServer/textedit_db.db"

Server::Server(QObject *parent)
    : QTcpServer(parent)
{
    //bool connected = db.connOpen();
    //if ( !connected) { throw QException(); }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(Path_to_DB);
    QFileInfo checkFile(Path_to_DB);

    if (checkFile.isFile()) {
        db.open();
        if ( db.isOpen() ) {
            qDebug() << " Start DB ";
            // Informazioni aggiuntive
            // db.setHostName("");
            // db.setUserName("");
            // db.setPassword("");
            //return true;

        } else {  throw QException(); }
    } else {
        qDebug() << "Cant Find DB";
        throw QException();
    }

    /*QFile accounts("accounts");
    if(accounts.open(QIODevice::ReadWrite)){
        QDataStream in(&accounts);
        in.setVersion(QDataStream::Qt_4_0);
        in >> _accounts;
    }
    accounts.close();

    QFile profiles("profiles");
    if(profiles.open(QIODevice::ReadWrite)){
        QDataStream in(&profiles);
        in.setVersion(QDataStream::Qt_4_0);
        in >> _profiles;
    }
    profiles.close();*/

}

bool Server::getDb() { return db.isOpen();}

quint32 Server::checkCredentials(QString username, QString password)
{

        if(!db.isOpen()){
            qDebug() << "Connessione al database persa";
            return 1;
        }

    qint32 uid=0;

        QSqlQuery qry("SELECT id,nickname,color FROM User WHERE username='" + username +
                      "'AND password='" + password + "'"); // Nickname?

            if(qry.next()){
                uid = qry.value(0).toInt();
                QString nickname = qry.value(1).toString();
                QString color = qry.value(2).toString();
                User logging(uid,
                           nickname.toLower(),
                           QColor(color),
                           128);

                _profiles.insert(uid,logging);
            }
            qDebug() << "Uid = " + QString(uid);
            return uid;



    /*for (auto acc : _accounts) {
        if (acc.username == username && acc.password == password) {
            return acc.uinqueId;
        }
    }
    return 0;*/
}


quint32 Server::registerUser(QString username, QString password, QString nickname)
{
    quint32 uid = QRandomGenerator::global()->generate();

    /*for (auto acc : _accounts) {
        if (acc.username == username) {
            return 0;
        }
        while (acc.uinqueId==uid && uid!=0) {
            uid = QRandomGenerator::global()->generate();
        }
    }*/

        if(!db.isOpen()){
            qDebug() << "Connessione al database persa";
            return 1;
        }

     int i;
     for (i=0; i++; i<20 && uid!=0){ // evito loop infinito, limite i arbitratio
            uid = QRandomGenerator::global()->generate();
            QSqlQuery qry("SELECT uid FROM User WHERE uid='" + QString(uid) +"'");

            if(!qry.next())
                break;
        }

        QSqlQuery qry("SELECT username,password,nickname FROM User WHERE username='" + username +
                      "'");

            if(qry.next() || i==20){
                return 0;
            } else {
                QColor color (QRandomGenerator::global()->generate() % 128 +128,QRandomGenerator::global()->generate() % 128 +128,QRandomGenerator::global()->generate() % 128 +128);
                QSqlQuery query;
                   query.prepare("INSERT INTO User (id, username, password, nickname, color) "
                                 "VALUES (:id, :username, :password, :nickname, :color)");
                                 // Serve uid

                   query.bindValue(":id", uid);
                   query.bindValue(":username", username);
                   query.bindValue(":password", password);
                   query.bindValue(":nickname", nickname.isEmpty() ? "void" : nickname);
                   query.bindValue(":color", color.name());
                   query.exec();

                   qry.exec();

                   if(qry.next()){
                       User randU(uid,
                                  nickname.toLower(),
                                  color,
                                  128);

                       _profiles.insert(uid,randU);
                       return uid;
                   } else
                       qDebug() << "Nuovo utente non trovato";
                   return 0;
            }
/*
    Account a;

    a.password = password;                      // create decent constructor here
    a.username = username;
    a.uinqueId = uid;

    _accounts.append(a);

    QFile accounts("accounts");
    accounts.open(QIODevice::WriteOnly);
    QDataStream out1(&accounts);
    out1.setVersion(QDataStream::Qt_4_0);

    out1 << _accounts;
    accounts.close();

    // generate random profile
    User randU(uid,
               username.toLower(),
               QColor(QRandomGenerator::global()->generate() % 128 +128,QRandomGenerator::global()->generate() % 128 +128,QRandomGenerator::global()->generate() % 128 +128),
               128);

    _profiles.insert(uid,randU);

    QFile profiles("profiles");
    profiles.open(QIODevice::WriteOnly);
    QDataStream out2(&profiles);
    out2.setVersion(QDataStream::Qt_4_0);

    out2 << _profiles;
    profiles.close();
    return uid;
*/
}

User Server::getProfile(quint32 uid) { return _profiles.find(uid).value(); }

void Server::updateUser(User user, quint32 uid, QString username, QString password)
{
    if(_profiles.contains(uid)){
        _profiles.insert(uid, user);
    }
    else {
        _profiles.find(uid).value() = user;
    }

        if(!db.isOpen()){
            qDebug() << "Connessione al database persa";
            return;
        }

    QSqlQuery qry;
    qry.prepare("UPDATE User SET username=:uname, password=:password, nickname=:nick, color=:color, image=:image WHERE uid=:uid'");

                   qry.bindValue(":username", username);
                   qry.bindValue(":password", password);
                   qry.bindValue(":nickname", user.nick.isEmpty() ? "void" : user.nick);
                   qry.bindValue(":color", user.color.name());
                   qry.bindValue(":image", user.icon.bits());

    qry.exec();


    QFile profiles("profiles");
    profiles.open(QIODevice::WriteOnly);
    QDataStream out(&profiles);
    out.setVersion(QDataStream::Qt_4_0);
    out << _profiles;
    profiles.close();
}

QStringList Server::retrieveFiles()
{
    QDir::current().mkdir("documents");
    QDir docs("documents");
    docs.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    return docs.entryList();
}

Document * Server::openFile(QString fname, Clientconn* client)
{
    if(_opendocs.contains(fname)){
        _opendocs.find(fname).value()->newSub(client);
        return _opendocs.find(fname).value();
    }
    else {
        Document * doc = new Document(fname, client);
        _opendocs.insert(fname, doc);
        return doc;
    }
}

Document * Server::newFile(QString fname, Clientconn* client)
{
    Document * doc = new Document(fname, client);
    _opendocs.insert(fname, doc);
    _filenames.append(fname);
    return doc;

}

void Server::incomingConnection(qintptr socketDescriptor)
{

    qDebug() << "main threadID: inside constructor: " << m_thread.currentThreadId();
    qDebug() << "incoming connection";

    Clientconn *client_socket_to_rec = new Clientconn(socketDescriptor, this);
    _sockets.push_back(client_socket_to_rec->getTcpSock());
    for (QTcpSocket* socket : _sockets) {

        /** Sending New Client Name **/
        if ( socket != nullptr ) {
            socket -> write(QByteArray::fromStdString(""));
        }
    }
}

