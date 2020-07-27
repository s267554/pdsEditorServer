#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QStringList>
class Account {
public:
    QString username;
    QString password;
    quint32 uinqueId;

private:
    friend QDataStream &operator<<(QDataStream& out, const Account& sen);
   friend  QDataStream &operator>>(QDataStream& in, Account& rec);
};

#endif // ACCOUNT_H
