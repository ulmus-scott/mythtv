#include "recordingstatus.h"

#include "mythdate.h"

QString RecStatus::toUIState(RecStatus::Type recstatus)
{
    if (recstatus == RecStatus::Recorded ||
        recstatus == RecStatus::WillRecord ||
        recstatus == RecStatus::Pending)
        return "normal";

    if (recstatus == RecStatus::Recording ||
        recstatus == RecStatus::Tuning)
        return "running";

    if (recstatus == RecStatus::Conflict ||
        recstatus == RecStatus::Offline ||
        recstatus == RecStatus::TunerBusy ||
        recstatus == RecStatus::Failed ||
        recstatus == RecStatus::Aborted ||
        recstatus == RecStatus::Missed ||
        recstatus == RecStatus::Failing)
    {
        return "error";
    }

    if (recstatus == RecStatus::Repeat ||
        recstatus == RecStatus::NeverRecord ||
        recstatus == RecStatus::DontRecord   ||
        (recstatus != RecStatus::DontRecord &&
         recstatus <= RecStatus::EarlierShowing))
    {
        return "disabled";
    }

    return "warning";
}

/// \brief Converts "recstatus" into a short (unreadable) string.
QString RecStatus::toString(RecStatus::Type recstatus, uint id)
{
    return toString(recstatus, QString::number(id));
}

/// \brief Converts "recstatus" into a short (unreadable) string.
QString RecStatus::toString(RecStatus::Type recstatus, const QString &name)
{
    QString ret = "-";
    switch (recstatus)
    {
        case RecStatus::Aborted:
            ret = tr("A"); //: RecStatusChar RecStatus::Aborted
            break;
        case RecStatus::Recorded:
            ret = tr("R"); //: RecStatusChar RecStatus::Recorded
            break;
        case RecStatus::Recording:
        case RecStatus::Tuning:
        case RecStatus::Failing:
        case RecStatus::WillRecord:
        case RecStatus::Pending:
            ret = name;
            break;
        case RecStatus::DontRecord:
            ret = tr("X"); //: RecStatusChar RecStatus::DontRecord
            break;
        case RecStatus::PreviousRecording:
            ret = tr("P"); //: RecStatusChar RecStatus::PreviousRecording
            break;
        case RecStatus::CurrentRecording:
            ret = tr("R"); //: RecStatusChar RecStatus::CurrentRecording
            break;
        case RecStatus::EarlierShowing:
            ret = tr("E"); //: RecStatusChar RecStatus::EarlierShowing
            break;
        case RecStatus::TooManyRecordings:
            ret = tr("T"); //: RecStatusChar RecStatus::TooManyRecordings
            break;
        case RecStatus::Cancelled:
            ret = tr("c"); //: RecStatusChar RecStatus::Cancelled
            break;
        case RecStatus::MissedFuture:
        case RecStatus::Missed:
            ret = tr("M"); //: RecStatusChar RecStatus::Missed
            break;
        case RecStatus::Conflict:
            ret = tr("C"); //: RecStatusChar RecStatus::Conflict
            break;
        case RecStatus::LaterShowing:
            ret = tr("L"); //: RecStatusChar RecStatus::LaterShowing
            break;
        case RecStatus::Repeat:
            ret = tr("r"); //: RecStatusChar RecStatus::Repeat
            break;
        case RecStatus::Inactive:
            ret = tr("x"); //: RecStatusChar RecStatus::Inactive
            break;
        case RecStatus::LowDiskSpace:
            ret = tr("K"); //: "RecStatusChar RecStatus::LowDiskSpace"
            break;
        case RecStatus::TunerBusy:
            ret = tr("B"); //: RecStatusChar RecStatus::TunerBusy
            break;
        case RecStatus::Failed:
            ret = tr("f"); //: RecStatusChar RecStatus::Failed
            break;
        case RecStatus::NotListed:
            ret = tr("N"); //: RecStatusChar RecStatus::NotListed
            break;
        case RecStatus::NeverRecord:
            ret = tr("V"); //: RecStatusChar RecStatus::NeverRecord
            break;
        case RecStatus::Offline:
            ret = tr("F"); //: RecStatusChar RecStatus::Offline
            break;
        case RecStatus::Unknown:
            break;
    }

    return (ret.isEmpty()) ? QString("-") : ret;
}

/// \brief Converts "recstatus" into a human readable string
QString RecStatus::toString(RecStatus::Type recstatus, RecordingType rectype)
{
    if (recstatus == RecStatus::Unknown && rectype == kNotRecording)
        return tr("Not Recording");

    switch (recstatus)
    {
        case RecStatus::Aborted:
            return tr("Aborted");
        case RecStatus::Recorded:
            return tr("Recorded");
        case RecStatus::Recording:
            return tr("Recording");
        case RecStatus::Tuning:
            return tr("Tuning");
        case RecStatus::Failing:
            return tr("Failing");
        case RecStatus::WillRecord:
            return tr("Will Record");
        case RecStatus::Pending:
            return tr("Pending");
        case RecStatus::DontRecord:
            return tr("Don't Record");
        case RecStatus::PreviousRecording:
            return tr("Previously Recorded");
        case RecStatus::CurrentRecording:
            return tr("Currently Recorded");
        case RecStatus::EarlierShowing:
            return tr("Earlier Showing");
        case RecStatus::TooManyRecordings:
            return tr("Max Recordings");
        case RecStatus::Cancelled:
            return tr("Manual Cancel");
        case RecStatus::MissedFuture:
        case RecStatus::Missed:
            return tr("Missed");
        case RecStatus::Conflict:
            return tr("Conflicting");
        case RecStatus::LaterShowing:
            return tr("Later Showing");
        case RecStatus::Repeat:
            return tr("Repeat");
        case RecStatus::Inactive:
            return tr("Inactive");
        case RecStatus::LowDiskSpace:
            return tr("Low Disk Space");
        case RecStatus::TunerBusy:
            return tr("Tuner Busy");
        case RecStatus::Failed:
            return tr("Recorder Failed");
        case RecStatus::NotListed:
            return tr("Not Listed");
        case RecStatus::NeverRecord:
            return tr("Never Record");
        case RecStatus::Offline:
            return tr("Recorder Off-Line");
        case RecStatus::Unknown:
            return tr("Unknown");
    }

    return tr("Unknown");
}

/// \brief Converts "recstatus" into a long human readable description.
QString RecStatus::toDescription(RecStatus::Type recstatus, RecordingType rectype,
                      const QDateTime &recstartts)
{
    if (recstatus == RecStatus::Unknown && rectype == kNotRecording)
        return tr("This showing is not scheduled to record");

    QString message;
    QDateTime now = MythDate::current();

    if (recstatus <= RecStatus::WillRecord)
    {
        switch (recstatus)
        {
            case RecStatus::WillRecord:
                message = tr("This showing will be recorded.");
                break;
            case RecStatus::Pending:
                message = tr("This showing is about to record.");
                break;
            case RecStatus::Recording:
                message = tr("This showing is being recorded.");
                break;
            case RecStatus::Tuning:
                message = tr("The showing is being tuned.");
                break;
            case RecStatus::Failing:
                message = tr("The showing is failing to record "
                                      "because of errors.");
                break;
            case RecStatus::Recorded:
                message = tr("This showing was recorded.");
                break;
            case RecStatus::Aborted:
                message = tr("This showing was recorded but was "
                                      "aborted before completion.");
                break;
            case RecStatus::Missed:
            case RecStatus::MissedFuture:
                message = tr("This showing was not recorded because "
                                      "the master backend was not running.");
                break;
            case RecStatus::Cancelled:
                message = tr("This showing was not recorded because "
                                      "it was manually cancelled.");
                break;
            case RecStatus::LowDiskSpace:
                message = tr("This showing was not recorded because "
                                      "there wasn't enough disk space.");
                break;
            case RecStatus::TunerBusy:
                message = tr("This showing was not recorded because "
                                      "the recorder was already in use.");
                break;
            case RecStatus::Failed:
                message = tr("This showing was not recorded because "
                                      "the recorder failed.");
                break;
            default:
                message = tr("The status of this showing is unknown.");
                break;
        }

        return message;
    }

    if (recstartts > now)
        message = tr("This showing will not be recorded because ");
    else
        message = tr("This showing was not recorded because ");

    switch (recstatus)
    {
        case RecStatus::DontRecord:
            message += tr("it was manually set to not record.");
            break;
        case RecStatus::PreviousRecording:
            message += tr("this episode was previously recorded "
                                   "according to the duplicate policy chosen "
                                   "for this title.");
            break;
        case RecStatus::CurrentRecording:
            message += tr("this episode was previously recorded and "
                                   "is still available in the list of "
                                   "recordings.");
            break;
        case RecStatus::EarlierShowing:
            message += tr("this episode will be recorded at an "
                                   "earlier time instead.");
            break;
        case RecStatus::TooManyRecordings:
            message += tr("too many recordings of this program have "
                                   "already been recorded.");
            break;
        case RecStatus::Conflict:
            message += tr("another program with a higher priority "
                                   "will be recorded.");
            break;
        case RecStatus::LaterShowing:
            message += tr("this episode will be recorded at a "
                                   "later time instead.");
            break;
        case RecStatus::Repeat:
            message += tr("this episode is a repeat.");
            break;
        case RecStatus::Inactive:
            message += tr("this recording rule is inactive.");
            break;
        case RecStatus::NotListed:
            message += tr("this rule does not match any showings in "
                                   "the current program listings.");
            break;
        case RecStatus::NeverRecord:
            message += tr("it was marked to never be recorded.");
            break;
        case RecStatus::Offline:
            message += tr("the required recorder is off-line.");
            break;
        default:
            if (recstartts > now)
                message = tr("This showing will not be recorded.");
            else
                message = tr("This showing was not recorded.");
            break;
    }

    return message;
}
