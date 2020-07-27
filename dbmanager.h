#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QSqlDatabase>

class DBManager
{
public:
    DBManager();
    bool connOpen();
    bool connClose();
    QSqlDatabase getDb();
private:
    QSqlDatabase db;
};

#endif // DBMANAGER_H
