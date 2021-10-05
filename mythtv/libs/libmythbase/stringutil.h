#ifndef CODEC_UTIL_H
#define CODEC_UTIL_H

#if __has_include(<format>) // C++20
#include <format>
#endif

#include <QByteArray>
#include <QString>
#include <QStringList>

#include "mythbaseexp.h"

namespace MBASE_PUBLIC StringUtil
{

bool isValidUTF8(const QByteArray &data); // used by libupnp/websocket.cpp

bool hasUtf8(const char *str); // unused
//#define M_QSTRING_UNICODE(str) hasUtf8(str) ? QString::fromUtf8(str) : str

/**
Creates a zero padded string representation of an integer
\param n integer to convert
\param width minimum string length including sign, if any

\note per the QString::arg() documentation the padding may prefix the sign of a negative number
*/
#ifdef __cpp_lib_format // C++20 with <format>
inline std::string intToPaddedString(int n, int width = 2)
{
    return std::format("{:0{}d}", n, width);
}
#else
inline QString intToPaddedString(int n, int width = 2)
{
    return QString("%1").arg(n, width, 10, QChar('0'));
}
#endif // __cpp_lib_format

inline QString indentSpaces(uint level, uint size = 4)
{
    return QString(level * size, ' ');
}

int naturalCompare(const QString &_a, const QString &_b,
                   Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);

} // namespace StringUtil
#endif // CODEC_UTIL_H
