#include "videometadata.h"

#include <algorithm>
#include <cmath> // for isnan()

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include "libmythbase/mythcorecontext.h"
#include "libmythbase/mythdate.h"
#include "libmythbase/mythdb.h"
#include "libmythbase/mythlogging.h"
#include "libmythbase/mythmiscutil.h"// for FileHash
#include "libmythbase/mythsorthelper.h"
#include "libmythbase/remotefile.h"
#include "libmythbase/remoteutil.h"
#include "libmythbase/storagegroup.h"
#include "libmythbase/stringutil.h"
#include "libmythbase/ternarycompare.h"

#include "dbaccess.h"
#include "videometadatalistmanager.h"
#include "videoutils.h"

VideoMetadata::VideoMetadata(QString filename,
                QString sortFilename,
                QString hash,
                QString trailer,
                QString coverfile,
                QString screenshot,
                QString banner,
                QString fanart,
                QString title,
                QString sortTitle,
                QString subtitle,
                QString sortSubtitle,
                QString tagline,
                int year,
                const QDate releasedate,
                QString inetref,
                int collectionref,
                QString homepage,
                QString director,
                QString studio,
                QString plot,
                float userrating,
                QString rating,
                int length,
                int playcount,
                int season,
                int episode,
                QDate insertdate,
                int id,
                ParentalLevel::Level showlevel,
                int categoryID,
                int childID,
                bool browse,
                bool watched,
                QString playcommand,
                QString category,
                genre_list genres,
                country_list countries,
                cast_list cast,
                QString host,
                bool processed,
                VideoContentType contenttype)
                :
                m_title(std::move(title)),
                m_sortTitle(std::move(sortTitle)),
                m_subtitle(std::move(subtitle)),
                m_sortSubtitle(std::move(sortSubtitle)),
                m_tagline(std::move(tagline)),
                m_inetref(std::move(inetref)),
                m_collectionref(collectionref),
                m_homepage(std::move(homepage)),
                m_director(std::move(director)),
                m_studio(std::move(studio)),
                m_plot(std::move(plot)),
                m_rating(std::move(rating)),
                m_playcommand(std::move(playcommand)),
                m_category(std::move(category)),
                m_genres(std::move(genres)),
                m_countries(std::move(countries)),
                m_cast(std::move(cast)),
                m_filename(std::move(filename)),
                m_sortFilename(std::move(sortFilename)),
                m_hash(std::move(hash)),
                m_trailer(std::move(trailer)),
                m_coverfile(std::move(coverfile)),
                m_screenshot(std::move(screenshot)),
                m_banner(std::move(banner)),
                m_fanart(std::move(fanart)),
                m_host(std::move(host)),
                m_categoryID(categoryID),
                m_childID(childID),
                m_year(year),
                m_releasedate(std::move(releasedate)),
                m_length(length),
                m_playcount(playcount),
                m_season(season),
                m_episode(episode),
                m_insertdate(std::move(insertdate)),
                m_showlevel(showlevel),
                m_browse(browse),
                m_watched(watched),
                m_id(id),
                m_userrating(userrating),
                m_processed(processed),
                m_contenttype(contenttype)
{
    // Try to glean data if none provided.
    if (m_title.isEmpty() && m_subtitle.isEmpty() && m_season == 0 && m_episode == 0)
    {
        m_title = VideoMetadata::FilenameToMeta(m_filename, 1);
        m_subtitle = VideoMetadata::FilenameToMeta(m_filename, 4);
        m_season = VideoMetadata::FilenameToMeta(m_filename, 2).toInt();
        m_episode = VideoMetadata::FilenameToMeta(m_filename, 3).toInt();
    }

    VideoCategory::GetCategory().get(m_categoryID, m_category);

    ensureSortFields();
}

void VideoMetadata::ensureSortFields(void)
{
    std::shared_ptr<MythSortHelper>sh = getMythSortHelper();
    if (m_sortTitle.isEmpty() and not m_title.isEmpty())
        m_sortTitle = sh->doTitle(m_title);
    if (m_sortSubtitle.isEmpty() and not m_subtitle.isEmpty())
        m_sortSubtitle = sh->doTitle(m_subtitle);
    if (m_sortFilename.isEmpty() and not m_filename.isEmpty())
        m_sortFilename = sh->doPathname(m_filename);
}



/**
 *  \brief Returns true if the object should appear before the argument.
 */
bool VideoMetadata::sortBefore(const VideoMetadata &rhs) const
{
    int cmp = StringUtil::naturalCompare(m_sortTitle, rhs.m_sortTitle);
    if (cmp == 0)
        cmp = StringUtil::naturalCompare(m_sortFilename, rhs.m_sortFilename);
    if (cmp == 0)
        cmp = ternary_compare(m_id, rhs.m_id);
    return cmp < 0;
}

bool VideoMetadata::removeDir(const QString &dirName)
{
    QDir d(dirName);

    d.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList contents = d.entryInfoList();
    if (contents.empty())
    {
        return d.rmdir(dirName);
    }

    for (const auto& entry : std::as_const(contents))
    {
        if (entry.isDir())
        {
            QString fileName = entry.fileName();
            if (!removeDir(fileName))
                return false;
        }
        else
        {
            if (!QFile(entry.fileName()).remove())
                return false;
        }
    }
    return d.rmdir(dirName);
}

/// Deletes the file associated with a metadata entry
bool VideoMetadata::DeleteFile()
{
    bool isremoved = false;

    if (!m_host.isEmpty())
    {
        QString url = StorageGroup::generate_file_url("Videos", m_host, m_filename);
        isremoved = RemoteFile::DeleteFile(url);
    }
    else
    {
        QFileInfo fi(m_filename);
        if (fi.isDir())
        {
            isremoved = removeDir(m_filename);
        }
        else
        {
            isremoved = QFile::remove(m_filename);
        }
    }

    if (!isremoved)
    {
        LOG(VB_GENERAL, LOG_DEBUG, QString("Could not delete file: %1")
                                       .arg(m_filename));
    }

    return isremoved;
}

void VideoMetadata::Reset()
{
    using namespace k_VideoMetadata::Default;
    VideoMetadata tmp(m_filename, QString(),
                    VideoMetadata::VideoFileHash(m_filename, m_host), trailer,
                    coverfile, screenshot, banner,
                    fanart, QString(), QString(), QString(), QString(),
                    QString(), year,
                    QDate(), inetref, -1, QString(), director,
                    QString(), plot, 0.0,
                    rating, 0, 0,
                    0, 0, QDate(), m_id,
                    ParentalLevel::plLowest, 0, -1, true, false, "", "",
                    VideoMetadata::genre_list(), VideoMetadata::country_list(),
                    VideoMetadata::cast_list(), m_host, false);
    tmp.m_prefix = m_prefix;

    *this = tmp;
}

void VideoMetadata::fillGenres()
{
    m_genres.clear();
    VideoGenreMap &vgm = VideoGenreMap::getGenreMap();
    VideoGenreMap::entry genres;
    if (vgm.get(m_id, genres))
    {
        VideoGenre &vg = VideoGenre::getGenre();
        for (long value : genres.values)
        {
            // Just add empty string for no-name genres
            QString name;
            vg.get(value, name);
            m_genres.emplace_back(value, name);
        }
    }
}

void VideoMetadata::fillCountries()
{
    m_countries.clear();
    VideoCountryMap &vcm = VideoCountryMap::getCountryMap();
    VideoCountryMap::entry countries;
    if (vcm.get(m_id, countries))
    {
        VideoCountry &vc = VideoCountry::getCountry();
        for (long value : countries.values)
        {
            // Just add empty string for no-name countries
            QString name;
            vc.get(value, name);
            m_countries.emplace_back(value, name);
        }
    }
}

void VideoMetadata::fillCast()
{
    m_cast.clear();
    VideoCastMap &vcm = VideoCastMap::getCastMap();
    VideoCastMap::entry cast;
    if (vcm.get(m_id, cast))
    {
        VideoCast &vc = VideoCast::GetCast();
        for (long value : cast.values)
        {
            // Just add empty string for no-name cast
            QString name;
            vc.get(value, name);
            m_cast.emplace_back(value, name);
        }
    }
}

/// Sets metadata from a DB row
///
/// Query string in VideoMetadataListManager::loadAllFromDatabase
///
void VideoMetadata::fromDBRow(MSqlQuery &query)
{
    m_title = query.value(0).toString();
    m_director = query.value(1).toString();
    m_studio = query.value(2).toString();
    m_plot = query.value(3).toString();
    m_rating = query.value(4).toString();
    m_year = query.value(5).toInt();
    m_releasedate = query.value(6).toDate();
    m_userrating = (float)query.value(7).toDouble();
    if (std::isnan(m_userrating) || m_userrating < 0)
        m_userrating = 0.0;
    m_userrating = std::min(m_userrating, 10.0F);
    m_length = std::chrono::minutes(query.value(8).toInt());
    m_playcount = query.value(9).toInt();
    m_filename = query.value(10).toString();
    m_hash = query.value(11).toString();
    m_showlevel = ParentalLevel(query.value(12).toInt()).GetLevel();
    m_coverfile = query.value(13).toString();
    m_inetref = query.value(14).toString();
    m_collectionref = query.value(15).toUInt();
    m_homepage = query.value(16).toString();
    m_childID = query.value(17).toUInt();
    m_browse = query.value(18).toBool();
    m_watched = query.value(19).toBool();
    m_playcommand = query.value(20).toString();
    m_categoryID = query.value(21).toInt();
    m_id = query.value(22).toInt();
    m_trailer = query.value(23).toString();
    m_screenshot = query.value(24).toString();
    m_banner = query.value(25).toString();
    m_fanart = query.value(26).toString();
    m_subtitle = query.value(27).toString();
    m_tagline = query.value(28).toString();
    m_season = query.value(29).toInt();
    m_episode = query.value(30).toInt();
    m_host = query.value(31).toString();
    m_insertdate = query.value(32).toDate();
    m_processed = query.value(33).toBool();

    m_contenttype = ContentTypeFromString(query.value(34).toString());

    ensureSortFields();

    VideoCategory::GetCategory().get(m_categoryID, m_category);

    // Genres
    fillGenres();

    //Countries
    fillCountries();

    // Cast
    fillCast();
}

void VideoMetadata::SaveToDatabase()
{
    using namespace k_VideoMetadata;
    using namespace k_VideoMetadata::Default;
    if (m_title.isEmpty())
        m_title = VideoMetadata::FilenameToMeta(m_filename, 1);
    if (m_hash.isEmpty())
        m_hash = VideoMetadata::VideoFileHash(m_filename, m_host);
    if (m_subtitle.isEmpty())
        m_subtitle = VideoMetadata::FilenameToMeta(m_filename, 4);
    if (m_director.isEmpty())
        m_director = Unknown::director;
    if (m_plot.isEmpty())
        m_plot = plot;
    if (m_rating.isEmpty())
        m_rating = rating;
    ensureSortFields();

    InfoMap metadataMap;
    GetImageMap(metadataMap);
    QString coverfile   = metadataMap["coverfile"];
    QString screenshot  = metadataMap["screenshotfile"];
    QString bannerfile  = metadataMap["bannerfile"];
    QString fanartfile  = metadataMap["fanartfile"];

    if (coverfile.isEmpty() || !RemoteFile::Exists(coverfile))
        m_coverfile = coverfile;
    if (screenshot.isEmpty() || !RemoteFile::Exists(screenshot))
        m_screenshot = screenshot;
    if (bannerfile.isEmpty() || !RemoteFile::Exists(bannerfile))
        m_banner = banner;
    if (fanartfile.isEmpty() || !RemoteFile::Exists(fanartfile))
        m_fanart = fanart;
    if (m_trailer.isEmpty())
        m_trailer = trailer;
    if (m_inetref.isEmpty())
        m_inetref = inetref;
    if (std::isnan(m_userrating))
        m_userrating = 0.0;
    if (m_userrating < -10.0F || m_userrating > 10.0F)
        m_userrating = 0.0;
    if (m_releasedate.toString().isEmpty())
        m_releasedate = QDate::fromString("0000-00-00", "YYYY-MM-DD");
    if (m_contenttype == kContentUnknown)
    {
        if (m_season > 0 || m_episode > 0)
            m_contenttype = kContentTelevision;
        else
            m_contenttype = kContentMovie;
    }

    bool inserting = m_id == 0;

    MSqlQuery query(MSqlQuery::InitCon());

    if (inserting)
    {
        m_browse = true;

        m_watched = false;

        query.prepare("INSERT INTO videometadata (title,subtitle,tagline,director,studio,plot,"
                      "rating,year,userrating,length,season,episode,filename,hash,"
                      "showlevel,coverfile,inetref,homepage,browse,watched,trailer,"
                      "screenshot,banner,fanart,host,processed,contenttype) VALUES (:TITLE, :SUBTITLE, "
                      ":TAGLINE, :DIRECTOR, :STUDIO, :PLOT, :RATING, :YEAR, :USERRATING, "
                      ":LENGTH, :SEASON, :EPISODE, :FILENAME, :HASH, :SHOWLEVEL, "
                      ":COVERFILE, :INETREF, :HOMEPAGE, :BROWSE, :WATCHED, "
                      ":TRAILER, :SCREENSHOT, :BANNER, :FANART, :HOST, :PROCESSED, :CONTENTTYPE)");
    }
    else
    {
        query.prepare("UPDATE videometadata SET title = :TITLE, subtitle = :SUBTITLE, "
                      "tagline = :TAGLINE, director = :DIRECTOR, studio = :STUDIO, "
                      "plot = :PLOT, rating= :RATING, year = :YEAR, "
                      "releasedate = :RELEASEDATE, userrating = :USERRATING, "
                      "length = :LENGTH, playcount = :PLAYCOUNT, season = :SEASON, "
                      "episode = :EPISODE, filename = :FILENAME, hash = :HASH, trailer = :TRAILER, "
                      "showlevel = :SHOWLEVEL, coverfile = :COVERFILE, "
                      "screenshot = :SCREENSHOT, banner = :BANNER, fanart = :FANART, "
                      "inetref = :INETREF, collectionref = :COLLECTION, homepage = :HOMEPAGE, "
                      "browse = :BROWSE, watched = :WATCHED, host = :HOST, playcommand = :PLAYCOMMAND, "
                      "childid = :CHILDID, category = :CATEGORY, processed = :PROCESSED, "
                      "contenttype = :CONTENTTYPE WHERE intid = :INTID");

        query.bindValue(":PLAYCOMMAND", m_playcommand);
        query.bindValue(":CHILDID", m_childID);
        query.bindValue(":CATEGORY", m_categoryID);
        query.bindValue(":INTID", m_id);
    }

    query.bindValueNoNull(":TITLE", m_title);
    query.bindValueNoNull(":SUBTITLE", m_subtitle);
    query.bindValue(":TAGLINE", m_tagline);
    query.bindValueNoNull(":DIRECTOR", m_director);
    query.bindValue(":STUDIO", m_studio);
    query.bindValue(":PLOT", m_plot);
    query.bindValueNoNull(":RATING", m_rating);
    query.bindValue(":YEAR", m_year);
    query.bindValue(":RELEASEDATE", m_releasedate);
    query.bindValue(":USERRATING", m_userrating);
    query.bindValue(":LENGTH", static_cast<qint64>(m_length.count()));
    query.bindValue(":PLAYCOUNT", m_playcount);
    query.bindValue(":SEASON", m_season);
    query.bindValue(":EPISODE", m_episode);
    query.bindValue(":FILENAME", m_filename);
    query.bindValue(":HASH", m_hash);
    query.bindValueNoNull(":TRAILER", m_trailer);
    query.bindValue(":SHOWLEVEL", m_showlevel);
    query.bindValueNoNull(":COVERFILE", m_coverfile);
    query.bindValueNoNull(":SCREENSHOT", m_screenshot);
    query.bindValueNoNull(":BANNER", m_banner);
    query.bindValueNoNull(":FANART", m_fanart);
    query.bindValueNoNull(":INETREF", m_inetref);
    query.bindValue(":COLLECTION", m_collectionref);
    query.bindValueNoNull(":HOMEPAGE", m_homepage);
    query.bindValue(":BROWSE", m_browse);
    query.bindValue(":WATCHED", m_watched);
    query.bindValue(":HOST", m_host);
    query.bindValue(":PROCESSED", m_processed);
    query.bindValue(":CONTENTTYPE", ContentTypeToString(m_contenttype));

    if (!query.exec() || !query.isActive())
    {
        MythDB::DBError("video metadata update", query);
        return;
    }

    if (inserting)
    {
        // Must make sure we have 'id' filled before we call updateGenres or
        // updateCountries

        if (!query.exec("SELECT LAST_INSERT_ID()") || !query.next())
        {
            MythDB::DBError("metadata id get", query);
            return;
        }

        m_id = query.value(0).toUInt();

        if (0 == m_id)
        {
            LOG(VB_GENERAL, LOG_ERR,
                QString("%1: The id of the last inserted row to "
                        "videometadata seems to be 0. This is odd.")
                    .arg(__FILE__));
            return;
        }
    }

    updateGenres();
    updateCountries();
    updateCast();
}

bool VideoMetadata::DeleteFromDatabase()
{
    VideoGenreMap::getGenreMap().remove(m_id);
    VideoCountryMap::getCountryMap().remove(m_id);
    VideoCastMap::getCastMap().remove(m_id);

    MSqlQuery query(MSqlQuery::InitCon());
    query.prepare("DELETE FROM videometadata WHERE intid = :ID");
    query.bindValue(":ID", m_id);
    if (!query.exec())
    {
        MythDB::DBError("delete from videometadata", query);
    }

    query.prepare("DELETE FROM filemarkup WHERE filename = :FILENAME");
    query.bindValue(":FILENAME", m_filename);
    if (!query.exec())
    {
        MythDB::DBError("delete from filemarkup", query);
    }

    return true;
}

void VideoMetadata::SetCategoryID(int id)
{
    if (id == 0)
    {
        m_category = "";
        m_categoryID = id;
        return;
    }
    if (m_categoryID == id)
        return;

    if (QString cat; VideoCategory::GetCategory().get(id, cat))
    {
        m_category = cat;
        m_categoryID = id;
    }
    else
    {
        LOG(VB_GENERAL, LOG_ERR, QString("Unknown category id: %1").arg(id));
    }
}

void VideoMetadata::updateGenres()
{
    VideoGenreMap::getGenreMap().remove(m_id);

    // ensure that all genres we have are in the DB
    auto genre = m_genres.begin();
    while (genre != m_genres.end())
    {
        if (!genre->second.trimmed().isEmpty())
        {
            genre->first = VideoGenre::getGenre().add(genre->second);
            VideoGenreMap::getGenreMap().add(m_id, genre->first);
            ++genre;
        }
        else
        {
            genre = m_genres.erase(genre);
        }
    }
}

void VideoMetadata::updateCountries()
{
    // remove countries for this video
    VideoCountryMap::getCountryMap().remove(m_id);

    auto country = m_countries.begin();
    while (country != m_countries.end())
    {
        if (!country->second.trimmed().isEmpty())
        {
            country->first = VideoCountry::getCountry().add(country->second);
            VideoCountryMap::getCountryMap().add(m_id, country->first);
            ++country;
        }
        else
        {
            country = m_countries.erase(country);
        }
    }
}

void VideoMetadata::updateCast()
{
    VideoCastMap::getCastMap().remove(m_id);

    // ensure that all cast we have are in the DB
    auto cast = m_cast.begin();
    while (cast != m_cast.end())
    {
        if (!cast->second.trimmed().isEmpty())
        {
            cast->first = VideoCast::GetCast().add(cast->second);
            VideoCastMap::getCastMap().add(m_id, cast->first);
            ++cast;
        }
        else
        {
            cast = m_cast.erase(cast);
        }
    }
}

void VideoMetadata::GetImageMap(InfoMap &imageMap) const
{
    QString coverfile;
    if (IsHostSet()
        && !GetCoverFile().startsWith(u'/')
        && !GetCoverFile().isEmpty()
        && !IsDefaultCoverFile(GetCoverFile()))
    {
        coverfile = StorageGroup::generate_file_url(QStringLiteral(u"Coverart"), GetHost(),
                                      GetCoverFile());
    }
    else
    {
        coverfile = GetCoverFile();
    }

    imageMap[QStringLiteral(u"coverfile")] = coverfile;
    imageMap[QStringLiteral(u"coverart")] = coverfile;

    QString screenshotfile;
    if (IsHostSet() && !GetScreenshot().startsWith(u'/')
        && !GetScreenshot().isEmpty())
    {
        screenshotfile = StorageGroup::generate_file_url(QStringLiteral(u"Screenshots"),
                                           GetHost(), GetScreenshot());
    }
    else
    {
        screenshotfile = GetScreenshot();
    }

    imageMap[QStringLiteral(u"screenshotfile")] = screenshotfile;
    imageMap[QStringLiteral(u"screenshot")] = screenshotfile;

    QString bannerfile;
    if (IsHostSet() && !GetBanner().startsWith(u'/')
        && !GetBanner().isEmpty())
    {
        bannerfile = StorageGroup::generate_file_url(QStringLiteral(u"Banners"), GetHost(),
                                       GetBanner());
    }
    else
    {
        bannerfile = GetBanner();
    }

    imageMap[QStringLiteral(u"bannerfile")] = bannerfile;
    imageMap[QStringLiteral(u"banner")] = bannerfile;

    QString fanartfile;
    if (IsHostSet() && !GetFanart().startsWith('/')
        && !GetFanart().isEmpty())
    {
        fanartfile = StorageGroup::generate_file_url(QStringLiteral(u"Fanart"), GetHost(),
                                       GetFanart());
    }
    else
    {
        fanartfile = GetFanart();
    }

    imageMap[QStringLiteral(u"fanartfile")] = fanartfile;
    imageMap[QStringLiteral(u"fanart")] = fanartfile;

    QString smartimage = coverfile;
    if (!screenshotfile.isEmpty () && (GetSeason() > 0 || GetEpisode() > 0))
        smartimage = screenshotfile;
    imageMap[QStringLiteral(u"smartimage")] = smartimage;
}

// This should be the equivalent of GetImageMap, only the image names
// are computed one at a time as needed.
QString VideoMetadata::GetImage(const QString& name) const
{
    if ((name == QStringLiteral(u"coverfile")) ||
        (name == QStringLiteral(u"coverart")))
    {
        QString coverfile = GetCoverFile();
        if (IsHostSet()
            && !coverfile.startsWith(u'/')
            && !coverfile.isEmpty()
            && !IsDefaultCoverFile(coverfile))
            return StorageGroup::generate_file_url(QStringLiteral(u"Coverart"), GetHost(),
                                     coverfile);
        return coverfile;
    }

    if ((name == QStringLiteral(u"screenshotfile")) ||
        (name == QStringLiteral(u"screenshot")))
    {
        QString screenshot = GetScreenshot();
        if (IsHostSet() && !screenshot.startsWith(u'/')
            && !screenshot.isEmpty())
            return StorageGroup::generate_file_url(QStringLiteral(u"Screenshots"),
                                     GetHost(), screenshot);
        return screenshot;
    }

    if ((name == QStringLiteral(u"bannerfile")) ||
        (name == QStringLiteral(u"banner")))
    {
        QString bannerfile = GetBanner();
        if (IsHostSet() && !bannerfile.startsWith(u'/')
            && !bannerfile.isEmpty())
            return StorageGroup::generate_file_url(QStringLiteral(u"Banners"), GetHost(),
                                     bannerfile);
        return bannerfile;
    }

    if ((name == QStringLiteral(u"fanartfile")) ||
        (name == QStringLiteral(u"fanart")))
    {
        QString fanartfile = GetFanart();
        if (IsHostSet() && !fanartfile.startsWith('/')
            && !fanartfile.isEmpty())
            return StorageGroup::generate_file_url(QStringLiteral(u"Fanart"), GetHost(),
                                     fanartfile);
        return fanartfile;
    }

    if ((name == QStringLiteral(u"smartimage")) ||
        (name == QStringLiteral(u"buttonimage")))
    {
        if (GetSeason() > 0 || GetEpisode() > 0)
        {
            QString screenshotfile = GetImage("screenshot");
            if (!screenshotfile.isEmpty())
                return screenshotfile;
        }
        return GetImage("coverart");
    }

    return {};
}

////////////////////////////////////////
//// Metadata
////////////////////////////////////////
namespace
{
    QString eatBraces(const QString &title, const QString &left_brace,
                      const QString &right_brace)
    {
        QString ret(title);
        bool keep_checking = true;

        while (keep_checking)
        {
            int left_position = ret.indexOf(left_brace);
            int right_position = ret.indexOf(right_brace);
            if (left_position == -1 || right_position == -1)
            {
                //
                //  No matching sets of these braces left.
                //

                keep_checking = false;
            }
            else
            {
                if (left_position < right_position)
                {
                    //
                    //  We have a matching set like:  (  foo  )
                    //

                    ret = ret.left(left_position) +
                            ret.right(ret.length() - right_position - 1);
                }
                else if (left_position > right_position)
                {
                    //
                    //  We have a matching set like:  )  foo  (
                    //

                    ret = ret.left(right_position) +
                            ret.right(ret.length() - left_position - 1);
                }
            }
        }

        return ret;
    }
}

int VideoMetadata::UpdateHashedDBRecord(const QString &hash,
                                   const QString &file_name,
                                   const QString &host)
{
    MSqlQuery query(MSqlQuery::InitCon());

    query.prepare("SELECT intid,filename FROM videometadata WHERE "
                  "hash = :HASH");
    query.bindValue(":HASH", hash);

    if (!query.exec() || !query.isActive())
    {
        MythDB::DBError("Video hashed metadata update", query);
        return -1;
    }

    if (!query.next())
        return -1;

    int intid = query.value(0).toInt();
    QString oldfilename = query.value(1).toString();

    query.prepare("UPDATE videometadata SET filename = :FILENAME, "
                  "host = :HOST WHERE intid = :INTID");
    query.bindValue(":FILENAME", file_name);
    query.bindValue(":HOST", host);
    query.bindValue(":INTID", intid);

    if (!query.exec() || !query.isActive())
    {
        MythDB::DBError("Video hashed metadata update (videometadata)", query);
        return -1;
    }

    query.prepare("UPDATE filemarkup SET filename = :FILENAME "
                  "WHERE filename = :OLDFILENAME");
    query.bindValue(":FILENAME", file_name);
    query.bindValue(":OLDFILENAME", oldfilename);

    if (!query.exec() || !query.isActive())
    {
        MythDB::DBError("Video hashed metadata update (filemarkup)", query);
        return -1;
    }

    return intid;
}

QString VideoMetadata::VideoFileHash(const QString &file_name,
                           const QString &host)
{
    if (host.isEmpty())
        return FileHash(file_name);

    if (gCoreContext->IsMasterBackend() && gCoreContext->IsThisHost(host))
    {
        StorageGroup sgroup("Videos", host);
        QString fullname = sgroup.FindFile(file_name);

        return FileHash(fullname);
    }

    QString url = StorageGroup::generate_file_url("Videos", host, file_name);

    return RemoteFile::GetFileHash(url);
}

QString VideoMetadata::FilenameToMeta(const QString &file_name, int position)
{
    // position 1 returns title, 2 returns season,
    //          3 returns episode, 4 returns subtitle

    QString cleanFilename = file_name.left(file_name.lastIndexOf('.'));
    static const QRegularExpression kSpaceRE      { "%20" };
    static const QRegularExpression kUnderscoreRE { "_"   };
    static const QRegularExpression kDotRE        { "\\." };
    cleanFilename.replace(kSpaceRE,      " ");
    cleanFilename.replace(kUnderscoreRE, " ");
    cleanFilename.replace(kDotRE,        " ");

    /*: Word(s) which should be recognized as "season" when parsing a video
     * file name. To list more than one word, separate them with a '|'.
     */
    QString season_translation = tr("Season", "Metadata file name parsing");

    /*: Word(s) which should be recognized as "episode" when parsing a video
     * file name. To list more than one word, separate them with a '|'.
     */
    QString episode_translation = tr("Episode", "Metadata file name parsing");

    // Primary Regexp
    QString separator = "(?:\\s?(?:-|/)?\\s?)?";
    QString regexp = QString(
                  "^(.*[^s0-9])" // Title
                  "%1" // optional separator
                  "(?:s|(?:Season|%2))?" // season marker
                  "%1" // optional separator
                  "(\\d{1,4})" // Season
                  "%1" // optional separator
                  "(?:[ex/]|Episode|%3)" // episode marker
                  "%1" // optional separator
                  "(\\d{1,3})" // Episode
                  "%1" // optional separator
                  "(.*)$" // Subtitle
                  ).arg(separator, season_translation, episode_translation);
    static const QRegularExpression filename_parse { regexp,
        QRegularExpression::CaseInsensitiveOption|QRegularExpression::UseUnicodePropertiesOption };

    // Cleanup Regexp
    QString regexp2 = QString("(%1(?:(?:Season|%2)%1\\d*%1)*%1)$")
                             .arg(separator, season_translation);
    static const QRegularExpression title_parse {regexp2,
        QRegularExpression::CaseInsensitiveOption|QRegularExpression::UseUnicodePropertiesOption };

    auto match = filename_parse.match(cleanFilename);
    if (match.hasMatch())
    {
        // Return requested value
        if (position == 1 && !match.capturedView(1).isEmpty())
        {
            // Clean up the title
            QString title = match.captured(1);
            match = title_parse.match(title);
            if (match.hasMatch())
                title = title.left(match.capturedStart());
            title = title.right(title.length() - title.lastIndexOf('/') -1);
            return title.trimmed();
        }
        if (position == 2)
            return match.captured(2).trimmed();
        if (position == 3)
            return match.captured(3).trimmed();
        if (position == 4)
            return match.captured(4).trimmed();
    }
    else if (position == 1)
    {
        QString title = cleanFilename;

        // Clean up the title
        title = title.right(title.length() -
                     title.lastIndexOf('/') -1);

        // Allow parentheses "()", but remove content inside other braces
        title = eatBraces(title, "[", "]");
        title = eatBraces(title, "{", "}");
        return title.trimmed();
    }
    else if (position == 2 || position == 3)
    {
        return {"0"};
    }

    return {};
}

void VideoMetadata::toMap(InfoMap &metadataMap) const
{
    GetImageMap(metadataMap);

    metadataMap[QStringLiteral(u"filename")] = GetFilename();
    metadataMap[QStringLiteral(u"sortfilename")] = GetSortFilename();
    metadataMap[QStringLiteral(u"title")] = GetTitle();
    metadataMap[QStringLiteral(u"sorttitle")] = GetSortTitle();
    metadataMap[QStringLiteral(u"subtitle")] = GetSubtitle();
    metadataMap[QStringLiteral(u"sortsubtitle")] = GetSortSubtitle();
    metadataMap[QStringLiteral(u"tagline")] = GetTagline();
    metadataMap[QStringLiteral(u"director")] = GetDirector();
    metadataMap[QStringLiteral(u"studio")] = GetStudio();
    metadataMap[QStringLiteral(u"description0")] = metadataMap[QStringLiteral(u"description")] = GetPlot();
    metadataMap[QStringLiteral(u"genres")] = GetDisplayGenres(*this);
    metadataMap[QStringLiteral(u"countries")] = GetDisplayCountries(*this);
    metadataMap[QStringLiteral(u"cast")] = GetDisplayCast(*this).join(", ");
    metadataMap[QStringLiteral(u"rating")] = GetDisplayRating(GetRating());
    metadataMap[QStringLiteral(u"length")] = GetDisplayLength(GetLength());
    metadataMap[QStringLiteral(u"playcount")] = QString::number(GetPlayCount());
    metadataMap[QStringLiteral(u"year")] = GetDisplayYear(GetYear());

    metadataMap[QStringLiteral(u"releasedate")] = MythDate::toString(
        GetReleaseDate(), MythDate::kDateFull | MythDate::kAddYear);

    metadataMap[QStringLiteral(u"userrating")] = GetDisplayUserRating(GetUserRating());

    if (GetSeason() > 0 || GetEpisode() > 0)
    {
        metadataMap[QStringLiteral(u"season")] = StringUtil::intToPaddedString(GetSeason(), 1);
        metadataMap[QStringLiteral(u"episode")] = StringUtil::intToPaddedString(GetEpisode(), 1);
        QString usingSE = QStringLiteral(u"s%1e%2")
            .arg(StringUtil::intToPaddedString(GetSeason(), 2),
                 StringUtil::intToPaddedString(GetEpisode(), 2));
        metadataMap[QStringLiteral(u"s##e##")] = metadataMap[QStringLiteral(u"s00e00")] = usingSE;
        QString usingX = QStringLiteral(u"%1x%2")
            .arg(StringUtil::intToPaddedString(GetSeason(), 1),
                 StringUtil::intToPaddedString(GetEpisode(), 2));
        metadataMap[QStringLiteral(u"##x##")] = metadataMap[QStringLiteral(u"00x00")] = usingX;
    }
    else
    {
        metadataMap[QStringLiteral(u"s##e##")] = metadataMap[QStringLiteral(u"##x##")] = QString();
        metadataMap[QStringLiteral(u"s00e00")] = metadataMap[QStringLiteral(u"00x00")] = QString();
        metadataMap[QStringLiteral(u"season")] = metadataMap[QStringLiteral(u"episode")] = QString();
    }

    GetStateMap(metadataMap);

    metadataMap[QStringLiteral(u"insertdate")] = MythDate::toString(
        GetInsertdate(), MythDate::kDateFull | MythDate::kAddYear);
    metadataMap[QStringLiteral(u"inetref")] = GetInetRef();
    metadataMap[QStringLiteral(u"homepage")] = GetHomepage();
    metadataMap[QStringLiteral(u"child_id")] = QString::number(GetChildID());
    metadataMap[QStringLiteral(u"browseable")] = GetDisplayBrowse(GetBrowse());
    metadataMap[QStringLiteral(u"watched")] = GetDisplayWatched(GetWatched());
    metadataMap[QStringLiteral(u"processed")] = GetDisplayProcessed(GetProcessed());
    metadataMap[QStringLiteral(u"category")] = GetCategory();
}

// This should be the equivalent of toMap, only the text strings
// are computed one at a time as needed.
QString VideoMetadata::GetText(const QString& name) const
{
    if (name == QStringLiteral(u"filename"))
        return GetFilename();
    if (name == QStringLiteral(u"sortfilename"))
        return GetSortFilename();
    if (name == QStringLiteral(u"title"))
        return GetTitle();
    if (name == QStringLiteral(u"sorttitle"))
        return GetSortTitle();
    if (name == QStringLiteral(u"subtitle"))
        return GetSubtitle();
    if (name == QStringLiteral(u"sortsubtitle"))
        return GetSortSubtitle();
    if (name == QStringLiteral(u"tagline"))
        return GetTagline();
    if (name == QStringLiteral(u"director"))
        return GetDirector();
    if (name == QStringLiteral(u"studio"))
        return GetStudio();
    if ((name == QStringLiteral(u"description")) ||
        (name == QStringLiteral(u"description0")))
        return GetPlot();
    if (name == QStringLiteral(u"genres"))
        return GetDisplayGenres(*this);
    if (name == QStringLiteral(u"countries"))
        return GetDisplayCountries(*this);
    if (name == QStringLiteral(u"cast"))
        return GetDisplayCast(*this).join(", ");
    if (name == QStringLiteral(u"rating"))
        return GetDisplayRating(GetRating());
    if (name == QStringLiteral(u"length"))
        return GetDisplayLength(GetLength());
    if (name == QStringLiteral(u"playcount"))
        return QString::number(GetPlayCount());
    if (name == QStringLiteral(u"year"))
        return GetDisplayYear(GetYear());

    if (name == QStringLiteral(u"releasedate"))
        return MythDate::toString(GetReleaseDate(), MythDate::kDateFull | MythDate::kAddYear);

    if (name == QStringLiteral(u"userrating"))
        return GetDisplayUserRating(GetUserRating());

    if (GetSeason() > 0 || GetEpisode() > 0)
    {
        if (name == QStringLiteral(u"season"))
            return StringUtil::intToPaddedString(GetSeason(), 1);
        if (name == QStringLiteral(u"episode"))
            return StringUtil::intToPaddedString(GetEpisode(), 1);
        if ((name == QStringLiteral(u"s##e##")) ||
            (name == QStringLiteral(u"s00e00")))
        {
            return QStringLiteral(u"s%1e%2")
                .arg(StringUtil::intToPaddedString(GetSeason(), 2),
                     StringUtil::intToPaddedString(GetEpisode(), 2));
        }
        if ((name == QStringLiteral(u"##x##")) ||
            (name == QStringLiteral(u"00x00")))
        {
            return QStringLiteral(u"%1x%2")
                .arg(StringUtil::intToPaddedString(GetSeason(), 1),
                     StringUtil::intToPaddedString(GetEpisode(), 2));
        }
    }

    if (name == QStringLiteral(u"insertdate"))
        return MythDate::toString(
            GetInsertdate(), MythDate::kDateFull | MythDate::kAddYear);
    if (name == QStringLiteral(u"inetref"))
        return GetInetRef();
    if (name == QStringLiteral(u"homepage"))
        return GetHomepage();
    if (name == QStringLiteral(u"child_id"))
        return QString::number(GetChildID());
    if (name == QStringLiteral(u"browseable"))
        return GetDisplayBrowse(GetBrowse());
    if (name == QStringLiteral(u"watched"))
        return GetDisplayWatched(GetWatched());
    if (name == QStringLiteral(u"processed"))
        return GetDisplayProcessed(GetProcessed());
    if (name == QStringLiteral(u"category"))
        return GetCategory();
    return {};
}

void VideoMetadata::GetStateMap(InfoMap &stateMap) const
{
    stateMap[QStringLiteral(u"trailerstate")] = TrailerToState(GetTrailer());
    stateMap[QStringLiteral(u"userratingstate")] =
            QString::number((int)(GetUserRating()));
    stateMap[QStringLiteral(u"watchedstate")] = WatchedToState(GetWatched());
    stateMap[QStringLiteral(u"videolevel")] = ParentalLevelToState(GetShowLevel());
}

// This should be the equivalent of GetStateMap, only the state strings
// are computed one at a time as needed.
QString VideoMetadata::GetState(const QString& name) const
{
    if (name == QStringLiteral(u"trailerstate"))
        return TrailerToState(GetTrailer());
    if (name == QStringLiteral(u"userratingstate"))
        return QString::number((int)(GetUserRating()));
    if (name == QStringLiteral(u"watchedstate"))
        return WatchedToState(GetWatched());
    if (name == QStringLiteral(u"videolevel"))
        return ParentalLevelToState(GetShowLevel());
    return {};
}

void ClearMap(InfoMap &metadataMap)
{
    metadataMap["coverfile"] = "";
    metadataMap["screenshotfile"] = "";
    metadataMap["bannerfile"] = "";
    metadataMap["fanartfile"] = "";
    metadataMap["filename"] = "";
    metadataMap["sortfilename"] = "";
    metadataMap["title"] = "";
    metadataMap["sorttitle"] = "";
    metadataMap["subtitle"] = "";
    metadataMap["sortsubtitle"] = "";
    metadataMap["tagline"] = "";
    metadataMap["director"] = "";
    metadataMap["studio"] = "";
    metadataMap["description"] = "";
    metadataMap["description0"] = "";
    metadataMap["genres"] = "";
    metadataMap["countries"] = "";
    metadataMap["cast"] = "";
    metadataMap["rating"] = "";
    metadataMap["length"] = "";
    metadataMap["playcount"] = "";
    metadataMap["year"] = "";
    metadataMap["releasedate"] = "";
    metadataMap["userrating"] = "";
    metadataMap["season"] = "";
    metadataMap["episode"] = "";
    metadataMap["s##e##"] = "";
    metadataMap["##x##"] = "";
    metadataMap["trailerstate"] = "";
    metadataMap["userratingstate"] = "";
    metadataMap["watchedstate"] = "";
    metadataMap["videolevel"] = "";
    metadataMap["insertdate"] = "";
    metadataMap["inetref"] = "";
    metadataMap["homepage"] = "";
    metadataMap["child_id"] = "";
    metadataMap["browseable"] = "";
    metadataMap["watched"] = "";
    metadataMap["category"] = "";
    metadataMap["processed"] = "";
}

QString VideoMetadata::MetadataGetTextCb(const QString& name, void *data)
{
    if (data == nullptr)
        return {};
    auto *metadata = static_cast<VideoMetadata *>(data);
    QString result = metadata->GetText(name);
    if (!result.isEmpty())
        return result;
    result = metadata->GetImage(name);
    if (!result.isEmpty())
        return result;
    return metadata->GetState(name);
}

QString VideoMetadata::MetadataGetImageCb(const QString& name, void *data)
{
    if (data == nullptr)
        return {};
    auto *metadata = static_cast<VideoMetadata *>(data);
    return metadata->GetImage(name);
}

QString VideoMetadata::MetadataGetStateCb(const QString& name, void *data)
{
    if (data == nullptr)
        return {};
    auto *metadata = static_cast<VideoMetadata *>(data);
    return metadata->GetState(name);
}


#if 0
bool VideoMetadata::fillDataFromID(const VideoMetadataListManager &cache)
{
    if (m_imp->getID() == 0)
        return false;

    VideoMetadataListManager::VideoMetadataPtr mp = cache.byID(m_imp->getID());
    if (mp.get())
    {
        *this = *mp;
        return true;
    }

    return false;
}
#endif

bool VideoMetadata::FillDataFromFilename(const VideoMetadataListManager &cache)
{
    if (GetFilename().isEmpty())
        return false;

    if (auto mp = cache.byFilename(GetFilename()))
    {
        *this = *mp;
        return true;
    }

    return false;
}


bool operator==(const VideoMetadata& a, const VideoMetadata& b)
{
    return a.GetFilename() == b.GetFilename();
}

bool operator!=(const VideoMetadata& a, const VideoMetadata& b)
{
    return a.GetFilename() != b.GetFilename();
}
