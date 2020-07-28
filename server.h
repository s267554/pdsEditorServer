/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <QStringList>
#include <QTcpServer>
#include <QColor>
#include <QTextEdit>
#include <QFile>
#include <QtNetwork>

//#include <QSql>

class ClientConn;
class Document;


class User{
public:
    User(quint32 u, QString n, QColor col, int tc): uid(u), nick(n), color(col), startCursor(tc){}
    User(){}
    quint32 uid = 0;                    // se faccio map<int, user> non serve, la uso come chiave
    QString nick = "";
    //QString uname ="";
    QColor color = QColor();
    int startCursor = 0;
    QImage icon =  QImage();
};

//class NotifyCursor{
//public:
//    NotifyCursor(int curs, quint32 u): cursPos(curs), uid(u){}
//    NotifyCursor(){}
//    int cursPos = 0;
//    quint32 uid = 0;
//};

class Symbol {
public:
    Symbol(QChar i, quint32 i1, int i2, std::vector<int>& vector, QTextCharFormat qtcf):
        c(i), siteid(i1), count(i2), fract(vector), format(qtcf){}
    Symbol(){

    }
    QChar c = 0;
    quint32 siteid = 0;
    int count = 0;
    std::vector<int> fract;
    QTextCharFormat format = QTextCharFormat();
};

class Message {
public:
    Message(int totA, int totR, quint32 fromUid, QList<Symbol>& symAdd, QList<Symbol>& symRem):
        totAdd(totA), totRem(totR), genFrom(fromUid), symToAdd(symAdd), symToRem(symRem){}
    Message(){}
    int totAdd = 0;
    int totRem = 0;
    quint32 genFrom = 0;

    QList<Symbol> symToAdd = {};
    QList<Symbol> symToRem = {};

};

// waste of a class this one, others too
class Account {
public:
    QString username;
    QString password;
    quint32 uinqueId;
};


class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(QObject *parent = 0);
    quint32 checkCredentials(QString username, QString password);
    quint32 registerUser(QString username, QString password);
    QStringList retrieveFiles();
    Document* openFile(QString fname, ClientConn* client);
    Document* newFile(QString fname, ClientConn* client);
    void updateUser(User user, quint32 uid);
    User getProfile(quint32);
protected:
    void incomingConnection(qintptr socketDescriptor) override;
private:
    QMap<QString, Document *> _opendocs;
    QVector<Account> _accounts;
    QMap<quint32, User> _profiles;
    //QSqlDatabase MyDB;
};

class ClientConn : public QObject {
    Q_OBJECT
public:
    ClientConn(int sd, Server * server);
    ~ClientConn();
    User clientPro;
    quint32 uniqueId;
    Server * server = 0;
    int socketDescriptor = 0;
    QTcpSocket * tcpSock = 0;
    bool isLoggedIn = false;
    Document * workingOn = 0;
private slots:
    void readAnyMessage();
};


class Document : public QObject{
    Q_OBJECT
public:
    Document(QString fname, ClientConn* client);
    std::vector<Symbol> _symbols;
    QMap<quint32, ClientConn *> _subs;
    void process(const Message&);
    void relay(const User&);
    void newSub(ClientConn* sub);
    int fractcmp(Symbol s1, Symbol s2);
private:
    QFile file;
    QTimer *timer = nullptr;
    bool isChanged = true;
private slots:
    void autoSave();
    void byeUser(quint32 uid);
};
#endif
