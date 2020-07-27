#include<QDataStream>
#include "symbol.h"
#include "user.h"
#include "message.h"
#include "account.h"
#include "symbol.h"

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
