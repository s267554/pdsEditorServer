#ifndef MESSAGE_H
#define MESSAGE_H
#include "symbol.h"
#include <QStringList>

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

private:
    friend QDataStream &operator<<(QDataStream& out, const Message& sen);
   friend  QDataStream &operator>>(QDataStream& in, Message& rec);

};
#endif // MESSAGE_H
