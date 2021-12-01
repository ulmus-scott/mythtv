#ifndef MYTH_DATETIME_H
#define MYTH_DATETIME_H

#include <cstdint>
#include <chrono>
using namespace std::chrono_literals;
#include <utility>

#include <QDateTime>
#include <QString>

#include "mythbaseexp.h"

/**
@brief wrapper class around QDateTime.

The static functions use UTC by default instead of Qt's default of local time.

This also provides overloads of + and - to add or subtract a std::chrono::duration
from a datetime as syntactic sugar.  Function parameters of type std::chrono::duration
allow the compiler to automatically convert coarser durations into the desired one.
*/
class MBASE_PUBLIC MythDateTime
{
  public:
    enum Format
    {
        ISODate        = Qt::ISODate,               ///< Default UTC
        kFilename      = 0x000100,                  ///< Default UTC, "yyyyMMddhhmmss"
        kDateFull      = 0x000200,                  ///< Default local time
        kDateShort     = 0x000400,                  ///< Default local time
        kDateEither    = kDateFull  | kDateShort,   ///< Default local time
        kTime          = 0x000800,                  ///< Default local time
        kDateTimeFull  = kDateFull  | kTime,        ///< Default local time
        kDateTimeShort = kDateShort | kTime,        ///< Default local time
        kAddYear       = 0x001000,      ///< Add year to string if not included
        kSimplify      = 0x002000,      ///< Do Today/Yesterday/Tomorrow transform
        kDatabase      = 0x004000,      ///< Default UTC, database format
        kAutoYear      = 0x008000,      ///< Add year only if different from current year
        kScreenShotFilename = 0x010000, ///< "yyyy-MM-ddThh-mm-ss.zzz"
        kRFC822        = 0x020000,      ///< HTTP Date format
        kOverrideUTC   = 0x100000,      ///< Present date/time in UTC
        kOverrideLocal = 0x200000,      ///< Present date/time in localtime
    };

    /// Returns current Date and Time in UTC.
    /// \param truncateToSeconds if true, time is set to the beginning of the current second, i.e. milliseconds = 0
    static MythDateTime current(bool truncateToSeconds = false)
    {
        QDateTime dt = QDateTime::currentDateTimeUtc();
        if (truncateToSeconds)
            dt = dt.addMSecs(-dt.time().msec());
        return dt;
    }

    /**
    \brief Returns the current Date and Time in UTC as an ISO 8601 extended format string.
    \param no_ms if true, the string does not include milliseconds

    The string format is (no_ms) ? "yyyy-MM-ddTHH:mm:ssZ" : "yyyy-MM-ddTHH:mm:ss.zzzZ"
    */
    static QString current_iso_string(bool no_ms = false)
    {
        return no_ms ? QDateTime::currentDateTimeUtc().toString(Qt::ISODate) :
                       QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    }

    /// Returns copy of QDateTime with TimeSpec set to UTC.
    /// This reinterprets (changes) the time represented.
    static QDateTime as_utc(const QDateTime &old_dt)
    {
        QDateTime dt(old_dt);
        dt.setTimeSpec(Qt::UTC);
        return dt;
    }

    /// Converts kFilename && kISODate formats to MythDateTime
    static MythDateTime fromString(const QString &dtstr)
    {
        QDateTime dt;
        if (dtstr.isEmpty())
            return as_utc(dt); // null and invalid

        if (!dtstr.contains("-") && dtstr.length() == 14)
        {
            // must be in yyyyMMddhhmmss format
            dt = QDateTime::fromString(dtstr, "yyyyMMddhhmmss");
        }
        else
        {
            dt = QDateTime::fromString(dtstr, Qt::ISODate);
        }

        return as_utc(dt);
    }

    /// Converts a UTC string with format to a UTC MythDateTime
    static MythDateTime fromString(const QString &str, const QString &format)
    {
        return as_utc(QDateTime::fromString(str, format));
    }

    /**
     *  This function takes the number of seconds since the start of the
     *  epoch and returns a QDateTime with the equivalent value.
     *
     *  Note: This function returns a QDateTime set to UTC, whereas the
     *  QDateTime::fromSecsSinceEpoch function returns a value set to
     *  localtime.
     *
     *  Note: QDateTime does not account for leap seconds.  Neither does Unix time,
     *  effectively, since it specifies a day is always exactly 86 400 seconds.
     *
     *  \param seconds  The number of seconds since the start of the epoch
     *                  at Jan 1 1970 at 00:00:00.
     *  \return A QDateTime.
     */
    static QDateTime fromSecsSinceEpoch(int64_t seconds)
    {
        return QDateTime::fromSecsSinceEpoch(seconds, Qt::UTC);
    }

    /** \brief Returns the total number of seconds since midnight of the supplied QTime
     *
     *  \param time     The QTime object to use
     */
    static std::chrono::seconds toSeconds(QTime time)
    {
        if (!time.isValid())
            return 0s;

        std::chrono::seconds secs = std::chrono::hours(time.hour());
        secs += std::chrono::minutes(time.minute());
        secs += std::chrono::seconds(time.second());

        return secs;
    }

    static std::chrono::milliseconds currentMSecsSinceEpochAsDuration()
    {
        return std::chrono::milliseconds(QDateTime::currentMSecsSinceEpoch());
    }

    static std::chrono::seconds secsInPast(const QDateTime& past)
    {
        return std::chrono::seconds(past.secsTo(current()));
    }

    static std::chrono::seconds secsInFuture(const QDateTime& future)
    {
        return std::chrono::seconds(current().secsTo(future));
    }

    /**
     * \brief Format a milliseconds time value
     *
     * Convert a millisecond time value into a textual representation of the value.
     *
     * \param msecs The time value in milliseconds. Since the type of this
     *     field is std::chrono::duration, any duration of a larger
     *     interval can be passed to this function and the compiler will
     *     convert it to milliseconds.
     *
     * \param fmt A formatting string specifying how to output the time.
     *     See QTime::toString for the a definition fo valid formatting
     *     characters.
     */
    static QString formatTime(std::chrono::milliseconds msecs, const QString& fmt)
    {
        return QTime::fromMSecsSinceStartOfDay(msecs.count()).toString(fmt);
    }

    explicit MythDateTime(QDateTime datetime) : m_datetime(std::move(datetime)) {}

    QDateTime getQDateTime() const { return m_datetime; }

    MythDateTime& operator+=(const std::chrono::milliseconds rhs)
    {
        m_datetime = m_datetime.addMSecs(rhs.count());
        return *this;
    }

    MythDateTime& operator-=(const std::chrono::milliseconds rhs)
    {
        m_datetime = m_datetime.addMSecs(-rhs.count());
        return *this;
    }

    friend MythDateTime operator+(MythDateTime lhs, const std::chrono::milliseconds rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend MythDateTime operator-(MythDateTime lhs, const std::chrono::milliseconds rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    /// Returns formatted string representing the time.
    QString toString(const QDateTime &datetime, uint format = Format::kDateTimeFull);

    /// Warning: this function does not convert to and from UTC
    QString toString(QDate date, uint format = Format::kDateFull);

  private:
    QDateTime m_datetime;

};

/**
\brief balanced ternary (three way) comparison
This is equivalent to C++20's operator <=>. See also ternarycompare.h.

Less than means earlier and greater than means later.

Since Qt 5.14 invalid QDateTimes compare equal and are less than all valid QDateTimes.
*/
inline int ternary_compare(const QDateTime& a, const QDateTime& b)
{
    if (a < b) return -1;
    if (a > b) return +1;
    return 0; // a == b
}

inline int ternary_compare(const MythDateTime& a, const MythDateTime& b)
{
    return ternary_compare(a.getQDateTime(), b.getQDateTime());
}

#endif // MYTH_DATETIME_H
