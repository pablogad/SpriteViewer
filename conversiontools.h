#ifndef CONVERSIONTOOLS_H
#define CONVERSIONTOOLS_H

#include <QString>

#include <inttypes.h>

class ConversionTools
{
public:
    static uint64_t numberStringToInteger(const QString str);
    static const QString int64ToHexString(const uint64_t n);
};

#endif // CONVERSIONTOOLS_H
