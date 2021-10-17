#include "stringutil.h"

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QTextCodec>
#else
#include <QStringDecoder>
#endif

#include "ternarycompare.h"

namespace StringUtil {

bool isValidUTF8(const QByteArray& data)
{
    // NOTE: If you have a better way to determine this, then please update this
    // Any chosen method MUST be able to identify UTF-8 encoded text without
    // using the BOM Byte-Order Mark as that will probably not be present.
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    const QString text = codec->toUnicode(data.constData(), data.size(), &state);
    if (state.invalidChars > 0)
        return false;
#else
    auto toUtf16 = QStringDecoder(QStringDecoder::Utf8);
    QString text = toUtf16.decode(data);
    if (toUtf16.hasError())
        return false;
#endif

    Q_UNUSED(text);
    return true;
}

/**
 * \brief Guess whether a string is UTF-8
 *
 * \note  This does not attempt to \e validate the whole string.
 *        It just checks if it has any UTF-8 sequences in it.
 *
 * \todo FIXME? skips first byte, not sure the second to last if statement makes sense
 */
bool hasUtf8(const char *str)
{
    const uchar *c = (uchar *) str;

    while (*c++)
    {
        // ASCII is < 0x80.
        // 0xC2..0xF4 is probably UTF-8.
        // Anything else probably ISO-8859-1 (Latin-1, Unicode)

        if (*c > 0xC1 && *c < 0xF5)
        {
            int bytesToCheck = 2;  // Assume  0xC2-0xDF (2 byte sequence)

            if (*c > 0xDF)         // Maybe   0xE0-0xEF (3 byte sequence)
                ++bytesToCheck;
            if (*c > 0xEF)         // Matches 0xF0-0xF4 (4 byte sequence)
                ++bytesToCheck;

            while (bytesToCheck--)
            {
                ++c;

                if (! *c)                    // String ended in middle
                    return false;            // Not valid UTF-8

                if (*c < 0x80 || *c > 0xBF)  // Bad UTF-8 sequence
                    break;                   // Keep checking in outer loop
            }

            if (!bytesToCheck)  // Have checked all the bytes in the sequence
                return true;    // Hooray! We found valid UTF-8!
        }
    }

    return false;
}

/**
This method chops the input a and b into pieces of
digits and non-digits (a1.05 becomes a | 1 | . | 05)
and compares these pieces of a and b to each other
(first with first, second with second, ...).

This is based on the natural sort order code code by Martin Pool
http://sourcefrog.net/projects/natsort/
Martin Pool agreed to license this under LGPL or GPL.

\todo FIXME: Using toLower() to implement case insensitive comparison is
sub-optimal, but is needed because we compare strings with
localeAwareCompare(), which does not know about case sensitivity.
A task has been filled for this in Qt Task Tracker with ID 205990.
http://trolltech.com/developer/task-tracker/index_html?method=entry&id=205990
Dead link.  QCollate might be of relevance.
*/
int naturalCompare(const QString &_a, const QString &_b, Qt::CaseSensitivity caseSensitivity)
{
    QString a;
    QString b;

    if (caseSensitivity == Qt::CaseSensitive)
    {
        a = _a;
        b = _b;
    }
    else
    {
        a = _a.toLower();
        b = _b.toLower();
    }

    const QChar* currA = a.unicode(); // iterator over a
    const QChar* currB = b.unicode(); // iterator over b

    if (currA == currB)
    {
        return 0;
    }

    while (!currA->isNull() && !currB->isNull())
    {
        const QChar* begSeqA = currA; // beginning of a new character sequence of a
        const QChar* begSeqB = currB;

        if (currA->unicode() == QChar::ObjectReplacementCharacter)
        {
            return 1;
        }

        if (currB->unicode() == QChar::ObjectReplacementCharacter)
        {
            return -1;
        }

        if (currA->unicode() == QChar::ReplacementCharacter)
        {
            return 1;
        }

        if (currB->unicode() == QChar::ReplacementCharacter)
        {
            return -1;
        }

        // find sequence of characters ending at the first non-character
        while (!currA->isNull() && !currA->isDigit() && !currA->isPunct() &&
               !currA->isSpace())
        {
            ++currA;
        }

        while (!currB->isNull() && !currB->isDigit() && !currB->isPunct() &&
               !currB->isSpace())
        {
            ++currB;
        }

        // compare these sequences
        const QString& subA(a.mid(begSeqA - a.unicode(), currA - begSeqA));
        const QString& subB(b.mid(begSeqB - b.unicode(), currB - begSeqB));
        int cmp = QString::localeAwareCompare(subA, subB);

        if (cmp != 0)
        {
            return cmp < 0 ? -1 : +1;
        }

        if (currA->isNull() || currB->isNull())
        {
            break;
        }

        // find sequence of characters ending at the first non-character
        while ((currA->isPunct() || currA->isSpace()) &&
               (currB->isPunct() || currB->isSpace()))
        {
            cmp = ternary_compare(*currA, *currB);
            if (cmp != 0)
            {
                return cmp;
            }
            ++currA;
            ++currB;
            if (currA->isNull() || currB->isNull())
            {
                break;
            }
        }

        // now some digits follow...
        if ((*currA == QLatin1Char('0')) || (*currB == QLatin1Char('0')))
        {
            // one digit-sequence starts with 0 -> assume we are in a fraction part
            // do left aligned comparison (numbers are considered left aligned)
            while (true)
            {
                if (!currA->isDigit() && !currB->isDigit())
                {
                    break;
                }
                if (!currA->isDigit())
                {
                    return +1;
                }
                if (!currB->isDigit())
                {
                    return -1;
                }
                if (*currA < *currB)
                {
                    return -1;
                }
                if (*currA > *currB)
                {
                    return + 1;
                }
                ++currA;
                ++currB;
            }
        }
        else
        {
            // No digit-sequence starts with 0 -> assume we are looking at some integer
            // do right aligned comparison.
            //
            // The longest run of digits wins. That aside, the greatest
            // value wins, but we can't know that it will until we've scanned
            // both numbers to know that they have the same magnitude.

            bool isFirstRun = true;
            int weight = 0;

            while (true)
            {
                if (!currA->isDigit() && !currB->isDigit())
                {
                    if (weight != 0)
                    {
                        return weight;
                    }
                    break;
                }
                if (!currA->isDigit())
                {
                    if (isFirstRun)
                    {
                        return *currA < *currB ? -1 : +1;
                    }
                    return -1;
                }
                if (!currB->isDigit())
                {
                    if (isFirstRun)
                    {
                        return *currA < *currB ? -1 : +1;
                    }
                    return +1;
                }
                if ((*currA < *currB) && (weight == 0))
                {
                    weight = -1;
                }
                else if ((*currA > *currB) && (weight == 0))
                {
                    weight = + 1;
                }
                ++currA;
                ++currB;
                isFirstRun = false;
            }
        }
    }

    if (currA->isNull() && currB->isNull())
    {
        return 0;
    }

    return currA->isNull() ? -1 : + 1;
}


} // namespace StringUtil
