#ifndef VIDEOMETADATA_H_
#define VIDEOMETADATA_H_

#include <utility> // for std::pair
#include <vector>

#include <QString>
#include <QDate>
#include <QCoreApplication>

#include "parentalcontrols.h"
#include "mythmetaexp.h"
#include "metadatacommon.h"

class MSqlQuery;
class VideoMetadataListManager;

using MetadataMap = QHash<QString,QString>;

class META_PUBLIC VideoMetadata
{
    Q_DECLARE_TR_FUNCTIONS(VideoMetadata);

  public:
    using genre_entry = std::pair<int, QString>;
    using country_entry = std::pair<int, QString>;
    using cast_entry = std::pair<int, QString>;
    using genre_list = std::vector<genre_entry>;
    using country_list = std::vector<country_entry>;
    using cast_list = std::vector<cast_entry>;

    static constexpr int k_DefaultYear = 1895;

    static int UpdateHashedDBRecord(const QString &hash, const QString &file_name,
                                    const QString &host);
    static QString VideoFileHash(const QString &file_name, const QString &host);
    static QString FilenameToMeta(const QString &file_name, int position);
    static QString TrimTitle(const QString &title, bool ignore_case);

    explicit VideoMetadata(QString filename = QString(),
             QString sortFilename = QString(),
             QString hash = QString(),
             QString trailer = QString(),
             QString coverfile = QString(),
             QString screenshot = QString(),
             QString banner = QString(),
             QString fanart = QString(),
             QString title = QString(),
             QString sortTitle = QString(),
             QString subtitle = QString(),
             QString sortSubtitle = QString(),
             QString tagline = QString(),
             int year = VideoMetadata::k_DefaultYear,
             QDate releasedate = QDate(),
             QString inetref = QString(),
             int collectionref = 0,
             QString homepage = QString(),
             QString director = QString(),
             QString studio = QString(),
             QString plot = QString(),
             float userrating = 0.0,
             QString rating = QString(),
             int length = 0,
             int playcount = 0,
             int season = 0,
             int episode = 0,
             QDate insertdate = QDate(),
             int id = 0,
             ParentalLevel::Level showlevel = ParentalLevel::plLowest,
             int categoryID = 0,
             int childID = -1,
             bool browse = true,
             bool watched = false,
             QString playcommand = QString(),
             QString category = QString(),
             genre_list genres = genre_list(),
             country_list countries = country_list(),
             cast_list cast = cast_list(),
             QString host = "",
             bool processed = false,
             VideoContentType contenttype = kContentUnknown);
    explicit VideoMetadata(MSqlQuery &query)
    {
        fromDBRow(query);
        ensureSortFields();
    }

    bool sortBefore(const VideoMetadata &rhs) const;

    void toMap(InfoMap &metadataMap) const;
    QString GetText(const QString& name) const;
    void GetStateMap(InfoMap &stateMap) const;
    QString GetState(const QString& name) const;
    void GetImageMap(InfoMap &imageMap) const;
    QString GetImage(const QString& name) const;

    static QString MetadataGetTextCb(const QString& name, void *data);
    static QString MetadataGetImageCb(const QString& name, void *data);
    static QString MetadataGetStateCb(const QString& name, void *data);

    const QString &GetPrefix() const { return m_prefix; }
    void SetPrefix(const QString &prefix) { m_prefix = prefix; }

    const QString &GetTitle() const { return m_title; }
    const QString &GetSortTitle() const { return m_sortTitle; }
    void SetTitle(const QString& title, const QString& sortTitle = "")
    {
        m_title = title;
        m_sortTitle = sortTitle;
        ensureSortFields();
    }

    const QString &GetSubtitle() const { return m_subtitle; }
    const QString &GetSortSubtitle() const { return m_sortSubtitle; }
    void SetSubtitle(const QString &subtitle, const QString &sortSubtitle = "")
    {
        m_subtitle = subtitle;
        m_sortSubtitle = sortSubtitle;
        ensureSortFields();
    }

    const QString &GetTagline() const { return m_tagline; }
    void SetTagline(const QString &tagline) { m_tagline = tagline; }

    int GetYear() const { return m_year; }
    void SetYear(int year) { m_year = year; }

    QDate GetReleaseDate() const { return m_releasedate; }
    void SetReleaseDate(QDate releasedate) { m_releasedate = releasedate; }

    const QString &GetInetRef() const { return m_inetref; }
    void SetInetRef(const QString &inetRef) { m_inetref = inetRef; }

    int GetCollectionRef() const { return m_collectionref; }
    void SetCollectionRef(int collectionref) { m_collectionref = collectionref; }

    const QString &GetHomepage() const { return m_homepage; }
    void SetHomepage(const QString &homepage) { m_homepage = homepage; }

    const QString &GetDirector() const { return m_director; }
    void SetDirector(const QString &director) { m_director = director; }

    const QString &GetStudio() const { return m_studio; }
    void SetStudio(const QString &studio) { m_studio = studio; }

    const QString &GetPlot() const { return m_plot; }
    void SetPlot(const QString &plot) { m_plot = plot; }

    const QString &GetRating() const { return m_rating; }
    void SetRating(const QString &rating) { m_rating = rating; }

    float GetUserRating() const { return m_userrating; }
    void SetUserRating(float userRating) { m_userrating = userRating; }

    std::chrono::minutes GetLength() const { return m_length; }
    void SetLength(std::chrono::minutes length) { m_length = length; }

    int GetSeason() const { return m_season; }
    void SetSeason(int season) { m_season = season; }

    int GetEpisode() const { return m_episode; }
    void SetEpisode(int episode) { m_episode = episode; }

    QDate GetInsertdate() const { return m_insertdate;}
    void SetInsertdate(QDate date) { m_insertdate = date;}

    unsigned int GetID() const { return m_id; }
    void SetID(int id) { m_id = id; }

    int GetChildID() const { return m_childID; }
    void SetChildID(int childID) { m_childID = childID; }

    bool GetBrowse() const { return m_browse; }
    void SetBrowse(bool browse) { m_browse = browse; }

    bool GetWatched() const { return m_watched; }
    void SetWatched(bool watched) { m_watched = watched; }

    bool GetProcessed() const { return m_processed; }
    void SetProcessed(bool processed) { m_processed = processed; }

    VideoContentType GetContentType() const { return m_contenttype; }
    void SetContentType(VideoContentType contenttype) { m_contenttype = contenttype; }

    const QString &GetPlayCommand() const { return m_playcommand; }
    void SetPlayCommand(const QString &playCommand)
    {
        m_playcommand = playCommand;
    }

    unsigned int GetPlayCount() const { return m_playcount; }
    void SetPlayCount(int playcount) { m_playcount = playcount; }

    ParentalLevel::Level GetShowLevel() const { return m_showlevel; }
    void SetShowLevel(ParentalLevel::Level showLevel)
    {
        m_showlevel = ParentalLevel(showLevel).GetLevel();
    }

    const QString &GetHost() const { return m_host; }
    void SetHost(const QString &host) { m_host = host; }

    const QString &GetFilename() const { return m_filename; }
    const QString &GetSortFilename() const { return m_sortFilename; }
    void SetFilename(const QString &filename, const QString &sortFilename = "")
    {
        m_filename = filename;
        m_sortFilename = sortFilename;
        ensureSortFields();
    }

    const QString &GetHash() const { return m_hash; }
    void SetHash(const QString &hash) { m_hash = hash; }

    const QString &GetTrailer() const { return m_trailer; }
    void SetTrailer(const QString &trailer) { m_trailer = trailer; }

    const QString &GetCoverFile() const { return m_coverfile; }
    void SetCoverFile(const QString &coverFile) { m_coverfile = coverFile; }

    const QString &GetScreenshot() const { return m_screenshot; }
    void SetScreenshot(const QString &screenshot) { m_screenshot = screenshot; }

    const QString &GetBanner() const { return m_banner; }
    void SetBanner(const QString &banner) { m_banner = banner; }

    const QString &GetFanart() const { return m_fanart; }
    void SetFanart(const QString &fanart) { m_fanart = fanart; }

    const QString &GetCategory() const { return m_category; }
//    void SetCategory(const QString &category) { m_category = category; }

    const genre_list &GetGenres() const { return m_genres; }
    void SetGenres(const genre_list &genres) { m_genres = genres; }

    const country_list &GetCountries() const { return m_countries; }
    void SetCountries(const country_list &countries)
    {
        m_countries = countries;
    }

    const cast_list &GetCast() const { return m_cast; }
    void SetCast(const cast_list &cast) { m_cast = cast; }

    int GetCategoryID() const
    {
        return m_categoryID;
    }
    void SetCategoryID(int id);

    void SaveToDatabase();
    [[deprecated("Use SaveToDatabase instead")]]
    void UpdateDatabase() { SaveToDatabase(); }
    bool DeleteFromDatabase(); ///< drops the metadata from the DB

//    bool fillDataFromID(const VideoMetadataListManager &cache);
    bool FillDataFromFilename(const VideoMetadataListManager &cache);

    bool DeleteFile(); ///< If you aren't VideoList don't call this

    void Reset(); ///< Resets to default metadata

    bool IsHostSet() const { return !m_host.isEmpty(); }

  private:
    void ensureSortFields();
    void fillCountries();
    void updateCountries();
    void fillGenres();
    void fillCast();
    void updateGenres();
    void updateCast();
    bool removeDir(const QString &dirName);
    void fromDBRow(MSqlQuery &query);

    // members
    QString              m_title;
    QString              m_sortTitle;
    QString              m_subtitle;
    QString              m_sortSubtitle;
    QString              m_tagline;
    QString              m_inetref;
    int                  m_collectionref {0};
    QString              m_homepage;
    QString              m_director;
    QString              m_studio;
    QString              m_plot;
    QString              m_rating;
    QString              m_playcommand;
    QString              m_category;
    genre_list           m_genres;
    country_list         m_countries;
    cast_list            m_cast;
    QString              m_filename;
    QString              m_sortFilename;
    QString              m_hash;
    QString              m_trailer;
    QString              m_coverfile;
    QString              m_screenshot;
    QString              m_banner;
    QString              m_fanart;
    QString              m_host;

    int                  m_categoryID    {0};
    int                  m_childID       {-1};
    int                  m_year          {VideoMetadata::k_DefaultYear};
    QDate                m_releasedate;
    std::chrono::minutes m_length        {0min};
    int                  m_playcount     {0};
    int                  m_season        {0};
    int                  m_episode       {0};
    QDate                m_insertdate;
    ParentalLevel::Level m_showlevel     {ParentalLevel::plNone};
    bool                 m_browse        {true};
    bool                 m_watched       {false};
    unsigned int         m_id            {0};  // videometadata.intid
    float                m_userrating    {0.0};
    bool                 m_processed     {false};
    VideoContentType     m_contenttype   {kContentUnknown};

    // not in DB
    QString              m_prefix;
};

META_PUBLIC void ClearMap(InfoMap &metadataMap);

META_PUBLIC bool operator==(const VideoMetadata &a, const VideoMetadata &b);
META_PUBLIC bool operator!=(const VideoMetadata &a, const VideoMetadata &b);

Q_DECLARE_METATYPE(VideoMetadata*)

#endif
