#include <QDataStream>
#include "account.h"

QDataStream &operator<<(QDataStream& out, const Account& sen){
    return out << sen.password << sen.uinqueId << sen.username;
}
QDataStream &operator>>(QDataStream& in, Account& rec){
    return in >> rec.password >> rec.uinqueId >> rec.username;
}
