#include<QDataStream>
#include"user.h"

QDataStream &operator<<(QDataStream& out, const User& sen){
    return out << sen.uid << sen.icon << sen.nick << sen.color << sen.startCursor;
}
QDataStream &operator>>(QDataStream& in, User& rec){
    return in >> rec.uid >> rec.icon >> rec.nick >> rec.color >> rec.startCursor;
}
