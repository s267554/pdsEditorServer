#ifndef SYMBOL_H
#define SYMBOL_H
#include <QStringList>
#include <QTextEdit>

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

private:
    friend QDataStream &operator<<(QDataStream& out, const Symbol& sen);
   friend  QDataStream &operator>>(QDataStream& in, Symbol& rec);
};


#endif // SYMBOL_H
