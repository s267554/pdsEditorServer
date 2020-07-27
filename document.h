#ifndef DOCUMENT_H
#define DOCUMENT_H
#include <QStringList>
#include <QTcpServer>
#include <QColor>
#include <QTextEdit>
#include <QFile>
#include <QtNetwork>
#include "user.h"
#include "server.h"
#include <QVector>
#include "symbol.h"
#include "clientconn.h"
#include "message.h"

class Symbol;
class Clientconn;
class Document : public QObject{
    Q_OBJECT
public:
    Document(QString fname, Clientconn* client);
    QVector<Symbol> _symbols;
    QMap<quint32, Clientconn *> _subs;
    void process(const Message&);
    void relay(const User&);
    void newSub(Clientconn* sub);
    int fractcmp(Symbol s1, Symbol s2);
private:
    QFile file;
    QTimer *timer = nullptr;
    bool isChanged = true;
private slots:
    void autoSave();
    void byeUser(quint32 uid);
};

#endif // DOCUMENT_H
