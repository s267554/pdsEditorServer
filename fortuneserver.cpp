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

#include "fortuneserver.h"
#include <stdlib.h>

#include <QtNetwork>
#include <QRandomGenerator>
#include <QTimer>


QDataStream &operator<<(QDataStream& out, const Symbol& sen){
    QVector<int> qvect;
    return out << sen.c << sen.count << sen.format << sen.siteid << qvect.fromStdVector(sen.fract);
}
QDataStream &operator>>(QDataStream& in, Symbol& rec){
    QVector<int> qvect;
    in >> rec.c >> rec.count >> rec.format >> rec.siteid >> qvect;
    rec.fract = qvect.toStdVector();
    return in;
}

QDataStream &operator<<(QDataStream& out, const Message& sen){
    return out << sen.totAdd << sen.totRem << sen.genFrom << sen.symToAdd << sen.symToRem;
}
QDataStream &operator>>(QDataStream& in, Message& rec){
    return in >> rec.totAdd >> rec.totRem >> rec.genFrom >> rec.symToAdd >> rec.symToRem;
}


QDataStream &operator<<(QDataStream& out, const User& sen){
    return out << sen.uid << sen.icon << sen.nick << sen.color << sen.startCursor;
}
QDataStream &operator>>(QDataStream& in, User& rec){
    return in >> rec.uid >> rec.icon >> rec.nick >> rec.color >> rec.startCursor;
}

QDataStream &operator<<(QDataStream& out, const Account& sen){
    return out << sen.password << sen.uinqueId << sen.username;
}
QDataStream &operator>>(QDataStream& in, Account& rec){
    return in >> rec.password >> rec.uinqueId >> rec.username;
}

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


FortuneServer::FortuneServer(QObject *parent)
    : QTcpServer(parent)
{
    QFile accounts("accounts");
    if(accounts.open(QIODevice::ReadOnly)){
        QDataStream in(&accounts);
        in.setVersion(QDataStream::Qt_4_0);
        in >> _accounts;
    }
    accounts.close();

    QFile profiles("profiles");
    if(profiles.open(QIODevice::ReadOnly)){
        QDataStream in(&profiles);
        in.setVersion(QDataStream::Qt_4_0);
        in >> _profiles;
    }
    profiles.close();

    _filenames = retrieveFiles();
}

void FortuneServer::incomingConnection(qintptr socketDescriptor)
{
    ClientConn * client = new ClientConn(socketDescriptor, this);
    // next pending connection LOOK QT DOCS
}

quint32 FortuneServer::checkCredentials(QString username, QString password)
{
    for (auto acc : _accounts) {
        if (acc.username == username && acc.password == password) {        
            return acc.uinqueId;
        }
    }

    return 0;
}

quint32 FortuneServer::registerUser(QString username, QString password)
{
    quint32 uid = QRandomGenerator::global()->generate();

    for (auto acc : _accounts) {
        if (acc.username == username) {
            return 0;
        }
        while (acc.uinqueId==uid && uid!=0) {
            uid = QRandomGenerator::global()->generate();
        }
    }

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
               QString(QRandomGenerator::global()->generate()),
               QColor(QRandomGenerator::global()->generate() % 128 +128,QRandomGenerator::global()->generate() % 128 +128,QRandomGenerator::global()->generate() % 128 +128),
               0);

    _profiles.insert(uid,randU);

    QFile profiles("profiles");
    profiles.open(QIODevice::WriteOnly);
    QDataStream out2(&profiles);
    out2.setVersion(QDataStream::Qt_4_0);

    out2 << _profiles;
    profiles.close();

    return uid;
}

QStringList FortuneServer::retrieveFiles()
{
    QDir docs("documents");
    return docs.entryList();
}

Document * FortuneServer::openFile(QString fname, ClientConn* client)
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

Document * FortuneServer::newFile(QString fname, ClientConn* client)
{
    Document * doc = new Document(fname, client);
    _opendocs.insert(fname, doc);

    _filenames.append(fname);

    return doc;

}

void FortuneServer::updateUser(User user, quint32 uid)
{
    if(_profiles.contains(uid)){
        _profiles.insert(uid, user);
    }
    else {
        _profiles.find(uid).value() = user;
    }

    QFile profiles("profiles");
    profiles.open(QIODevice::WriteOnly);
    QDataStream out(&profiles);
    out.setVersion(QDataStream::Qt_4_0);

    out << _profiles;
    profiles.close();
}

User FortuneServer::getProfile(quint32 uid)
{
    return _profiles.find(uid).value();
}


ClientConn::ClientConn(int socketDescriptor, FortuneServer * server) : server(server)
{
    tcpSock = new QTcpSocket();
    tcpSock->setSocketDescriptor(socketDescriptor);

    connect(tcpSock, &QTcpSocket::readyRead, this, &ClientConn::readAnyMessage);
    qDebug() << "Someone's in";


}

ClientConn::~ClientConn()
{

}

void ClientConn::readAnyMessage()
{
    QDataStream in(tcpSock);
    in.setVersion(QDataStream::Qt_4_0);

    int op;

    QString username;
    QString password;

    QString filename;
    QString answer;

    User user;

//    NotifyCursor nfy;
    Message msg;

    qDebug() << "receiving some...";
    do {
        in.startTransaction();
        in >> op;
        switch(op) {
        case 'l':
            in.startTransaction();
            in >> username;
            in >> password;
            if(in.commitTransaction()){
                uniqueId = server->checkCredentials(username, password);
                if (uniqueId != 0) {
                    isLoggedIn = true;
                    clientPro = server->getProfile(uniqueId);

                    QByteArray block;
                    QDataStream out(&block, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_0);

                    out << op;
                    out << uniqueId;
                    out << server->retrieveFiles();

                    tcpSock->write(block);
                }
                else {
                    QByteArray block;
                    QDataStream out(&block, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_0);
                    out << op;

                    out << uniqueId;
                    tcpSock->write(block);
                }
            }
            qDebug() << "received '" << char(op) << "' message";
            break;
        case 's':
            in.startTransaction();
            in >> username;
            in >> password;
            if(in.commitTransaction()){
                uniqueId = server->registerUser(username, password);
                if (uniqueId != 0) {
                    isLoggedIn = true;
                    clientPro = server->getProfile(uniqueId);

                    QByteArray block;
                    QDataStream out(&block, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_0);

                    out << op;
                    out << uniqueId;
                    out << server->retrieveFiles();

                    tcpSock->write(block);
                }
                else {
                    QByteArray block;
                    QDataStream out(&block, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_0);

                    out << op;
                    out << uniqueId;
                    tcpSock->write(block);
                }
            }
            qDebug() << "received '" << (char) op << "' message";
            break;
        case 'o':
            in.startTransaction();
            in >> filename;
            if(in.commitTransaction())
                workingOn = server->openFile(filename, this);
            qDebug() << "received '" << (char) op << "' message";
            break;
        case 'n':
            in.startTransaction();
            in >> filename;
            if(in.commitTransaction())
                workingOn = server->newFile(filename, this);
            qDebug() << "received '" << (char) op << "' message";
            break;
        case 'u':
            in.startTransaction();
            in >> user;
            if(in.commitTransaction())
                server->updateUser(user, uniqueId);
            qDebug() << "received '" << (char) op << "' message";
            break;
        case 'm':
            in.startTransaction();
            in >> msg;
            if(in.commitTransaction()) {
                qDebug() << "received '" << (char) op << "' message";
                workingOn->process(msg);
            }
            break;
//        case 'c':
//            in >> nfy;
//            workingOn->relay(nfy);
//            break;
        }

    } while(in.commitTransaction());

}


Document::Document(QString fname, ClientConn* client)
{
    file.setFileName(QString("documents/").append(fname));
    file.open(QFile::ReadWrite);

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_0);
    in >> _symbols;
    newSub(client);
    file.close();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Document::autoSave);
    timer->start(1000 * 1);     // every 1000 ms = every 1s
}

void Document::newSub(ClientConn *sub)
{
    quint32 uid = sub->uniqueId;

    _subs.insert(uid, sub);

    connect(sub->tcpSock, &QTcpSocket::disconnected, [this, uid]()
    {
        byeUser(uid);
    });

    // send content to new sub
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    qDebug() << "sendin: " << _symbols.size() << " to new user";
    out << 't';
    out << _symbols;
    sub->tcpSock->write(block);

    // send new sub's User to every sub
    QByteArray blockU;
    QDataStream outU(&blockU, QIODevice::WriteOnly);
    outU.setVersion(QDataStream::Qt_4_0);
    outU << 'u';
    outU << sub->clientPro;

    for(auto any: _subs){
        any->tcpSock->write(blockU);
    }

    // send any sub's User to new sub

    for(auto any: _subs){
        if(any->uniqueId!=uid){
            QByteArray blockN;
            QDataStream outN(&blockN, QIODevice::WriteOnly);
            outN.setVersion(QDataStream::Qt_4_0);
            outN << 'u';
            outN << any->clientPro;
            sub->tcpSock->write(blockN);
        }
    }


}

void Document::autoSave()
{
    if(!isChanged)
        return;

    qDebug() << "Saving...";

    file.open(QFile::WriteOnly | QFile::Truncate);
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_0);

    out << _symbols;

    isChanged = false;

    file.close();
}

void Document::byeUser(quint32 uid)
{
    for(auto client : _subs){
        if(client->uniqueId != uid){
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out << 'd';
            out << uid;
            client->tcpSock->write(block);
        }
    }
}

int Document::fractcmp(Symbol s1, Symbol s2){
    int digit = 0;
    int cmp;

    auto v1 = s1.fract;
    auto v2 = s2.fract;

    while (v1.size() > digit && v2.size() > digit)
    {
        cmp = v1.at(digit) - v2.at(digit);
        if(cmp!=0)
            return cmp;
        digit++;
    }

    // until now vectors are equal but one may continue
    if(v1.size() > digit && v1.at(digit) > 0)
            return 1;

    if(v2.size() > digit && v2.at(digit) == 0)
            return -1;

    cmp = s1.siteid - s2.siteid;
    if(cmp!=0)
        return cmp;

    cmp = s1.count - s2.count;
    if(cmp!=0)
        return cmp;

    // exactly the same
    return cmp; // that is 0
}


void Document::process(const Message& m)
{
    for(auto client : _subs){
        if(client->uniqueId != m.genFrom){
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out << 'm';

            out << m;
            client->tcpSock->write(block);
        }
    }

    //binary search
    int lowbound;
    int upbound;
    int index;
    Symbol curr;

    /* let's look for syms to erase */
    for(auto mi = m.symToRem.begin(); mi != m.symToRem.end(); mi++){

        lowbound = 0;
        upbound = _symbols.size();

        while(lowbound < upbound){
            index = (upbound+lowbound) /2;
            curr = _symbols.at(index);

            /* to check if it's the sym I'm looking for siteid & count are enough */
            if (curr.siteid == mi->siteid && curr.count == mi->count) {

                _symbols.erase(_symbols.begin()+index);


                break;
            }

            if(fractcmp(curr, *mi) > 0)
                upbound = index;
            else
                lowbound = index +1;
        }

        //then it was already removed
    }

    /* let's look for syms to add */
    for(auto mi = m.symToAdd.begin(); mi != m.symToAdd.end(); mi++){

        lowbound = 0;
        upbound = _symbols.size();

        while(lowbound<upbound){

            index = (upbound+lowbound) /2;
            curr = _symbols.at(index);

            if(fractcmp(curr, *mi) > 0)
                upbound = index;
            else
                lowbound = index+1;
        }

        _symbols.insert(_symbols.begin() + upbound, *mi);

    }


//    QString textA;
//    for(auto s : m.symToAdd){
//        textA.append(s.c);
//    }
//    qDebug() << "add is: " << textA;

//    QString textR;
//    for(auto s : m.symToRem){
//        textR.append(s.c);
//    }
//    qDebug() << "rem is: " << textR;

    isChanged = true;
}

//void Document::relay(const NotifyCursor & nfy)
//{
//    for(auto client : _subs){
//        if(client->uniqueId != nfy.uid){
//            QByteArray block;
//            QDataStream out(&block, QIODevice::WriteOnly);
//            out.setVersion(QDataStream::Qt_4_0);
//            out << 'c';

//            out << nfy;
//            client->tcpSock->write(block);
//        }
//    }
//}
