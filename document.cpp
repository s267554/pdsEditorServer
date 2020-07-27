#include "document.h"
#include "clientconn.h"
#include"server.h"
#include <stdlib.h>

Document::Document(QString fname, Clientconn* client)
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

void Document::newSub(Clientconn *sub)
{
    quint32 uid = sub->uniqueId;
    _subs.insert(uid, sub);
    connect(sub->getTcpSock(), &QTcpSocket::disconnected, [this, uid](){ byeUser(uid); });

    // send new sub's User to every sub
    QByteArray blockU;
    QDataStream outU(&blockU, QIODevice::WriteOnly);
    outU.setVersion(QDataStream::Qt_4_0);
    outU << 'u';
    outU << sub->clientPro;

    for(auto any: _subs){ any->getTcpSock()->write(blockU); }

    // send any sub's User to new sub
    for(auto any: _subs){
        if(any->uniqueId!=uid){
            QByteArray blockN;
            QDataStream outN(&blockN, QIODevice::WriteOnly);
            outN.setVersion(QDataStream::Qt_4_0);
            outN << 'u';
            outN << any->clientPro;
            sub->getTcpSock()->write(blockN);
        }
    }

    // send content to new sub
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    qDebug() << "sendin: " << _symbols.size() << " to new user";
    out << 't';
    out << _symbols;
    sub->getTcpSock()->write(block);

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
            client->getTcpSock()->write(block);
        }
    }
}

int Document::fractcmp(Symbol s1, Symbol s2){
    unsigned int digit = 0;
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

void Document::relay(const User & u)
{
    for(auto client : _subs){
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out << 'u';
            out << u;
            client->getTcpSock()->write(block);
    }
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
            client->getTcpSock()->write(block);
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
    isChanged = true;
}
