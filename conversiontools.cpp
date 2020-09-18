#include <QDebug>

#include "conversiontools.h"

// Converts decimal or hex number to integer. Returns -1 on error
uint64_t ConversionTools::numberStringToInteger(const QString str)
{
    uint64_t res = 0;
    int idx=0;
    bool isHex = false;
    if(str.startsWith("0x")) {
        idx=2;
        isHex = true;
    }
    int maxidx = str.size();
    if(str.endsWith(("h"))) {
        maxidx--;
        isHex = true;
    }
    while (idx<maxidx) {
        const QChar c = str.at(idx++);
        if(!c.isDigit() &&
             (!isHex ||
                 (
                   isHex &&
                   !(c >= 'a' && c <= 'f') &&
                   !(c >= 'A' && c <= 'F')
                 )
              )
                ) {
            qDebug() << "Number format incorrect:" << str;
            return -1;
        }
        if (isHex)
            res <<= 4;
        else
            res *= 10;
        unsigned long ch = c.toLatin1();
        if(c.isDigit())
            ch -= '0';
        else if(c <= 'F')
            ch -= ('A'-10);
        else
            ch -= ('a'-10);
        res += reinterpret_cast<uint64_t>(ch);
    }
    return res;
}

const QString ConversionTools::int64ToHexString(const uint64_t n)
{
    QString res("0x");
    res = res.append(QString::number(n,16));
    return res;
}
