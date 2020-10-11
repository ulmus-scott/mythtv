#ifndef MYTHCORECONTEXT_H_
#define MYTHCORECONTEXT_H_

#include <vector>

#include <QHostAddress>
#include <QMetaMethod>
#include <QObject>
#include <QString>

#include "mythdb.h"
#include "mythbaseexp.h"
#include "mythobservable.h"
#include "mythsocket_cb.h"
#include "mythlogging.h"
#include "mythlocale.h"
#include "mythsession.h"

#define MYTH_APPNAME_MYTHBACKEND "mythbackend"
#define MYTH_APPNAME_MYTHJOBQUEUE "mythjobqueue"
#define MYTH_APPNAME_MYTHFRONTEND "mythfrontend"
#define MYTH_APPNAME_MYTHTV_SETUP "mythtv-setup"
#define MYTH_APPNAME_MYTHFILLDATABASE "mythfilldatabase"
#define MYTH_APPNAME_MYTHCOMMFLAG "mythcommflag"
#define MYTH_APPNAME_MYTHCCEXTRACTOR "mythccextractor"
#define MYTH_APPNAME_MYTHPREVIEWGEN "mythpreviewgen"
#define MYTH_APPNAME_MYTHTRANSCODE "mythtranscode"
#define MYTH_APPNAME_MYTHWELCOME "mythwelcome"
#define MYTH_APPNAME_MYTHSHUTDOWN "mythshutdown"
#define MYTH_APPNAME_MYTHLCDSERVER "mythlcdserver"
#define MYTH_APPNAME_MYTHAVTEST "mythavtest"
#define MYTH_APPNAME_MYTHMEDIASERVER "mythmediaserver"
#define MYTH_APPNAME_MYTHMETADATALOOKUP "mythmetadatalookup"
#define MYTH_APPNAME_MYTHUTIL "mythutil"
#define MYTH_APPNAME_MYTHSCREENWIZARD "mythscreenwizard"
#define MYTH_APPNAME_MYTHFFPROBE "mythffprobe"
#define MYTH_APPNAME_MYTHEXTERNRECORDER "mythexternrecorder"

class MDBManager;
class MythCoreContextPrivate;
class MythSocket;
class MythScheduler;
class MythPluginManager;

class MythCoreContext;
using CoreWaitSigFn = void (MythCoreContext::*)(void);
struct CoreWaitInfo {
    const char *name;
    CoreWaitSigFn fn;
};

/** \class MythCoreContext
 *  \brief This class contains the runtime context for MythTV.
 *
 *   This class can be used to query for and set global and host
 *   settings, and is used to communicate between the frontends
 *   and backends. It also contains helper functions for theming
 *   and for getting system defaults, parsing the command line, etc.
 *   It also contains support for database error printing, and
 *   database message logging.
 */
class MBASE_PUBLIC MythCoreContext : public QObject, public MythObservable, public MythSocketCBs
{
    Q_OBJECT
  public:
    MythCoreContext(const QString &binversion, QObject *guiContext);
    ~MythCoreContext() override;

    bool Init(void);

    void SetLocalHostname(const QString &hostname);
    void SetServerSocket(MythSocket *serverSock);
    void SetEventSocket(MythSocket *eventSock);
    void SetScheduler(MythScheduler *sched);

    bool SafeConnectToMasterServer(bool blockingClient = true,
                                   bool openEventSocket = true);
    bool ConnectToMasterServer(bool blockingClient = true,
                               bool openEventSocket = true);

    MythSocket *ConnectCommandSocket(const QString &hostname, int  port,
                                     const QString &announcement,
                                     bool *proto_mismatch = nullptr,
                                     int maxConnTry = -1,
                                     int setup_timeout = -1);

    MythSocket *ConnectEventSocket(const QString &hostname, int port);

    bool SetupCommandSocket(MythSocket *serverSock, const QString &announcement,
                            uint timeout_in_ms, bool &proto_mismatch);

    bool CheckProtoVersion(MythSocket *socket,
                           uint timeout_ms = kMythSocketLongTimeout,
                           bool error_dialog_desired = false);

    static QString GenMythURL(const QString& host = QString(), int port = 0,
                              QString path = QString(),
                              const QString& storageGroup = QString());

    QString GetMasterHostPrefix(const QString &storageGroup = QString(),
                                const QString &path = QString());
    QString GetMasterHostName(void);
    QString GetHostName(void);
    QString GetFilePrefix(void);

    bool IsConnectedToMaster(void);
    void SetAsBackend(bool backend);
    bool IsBackend(void) const;        ///< is this process a backend process
    void SetAsFrontend(bool frontend);
    bool IsFrontend(void) const;  ///< is this process a frontend process
    bool IsFrontendOnly(void);   ///< is there a frontend, but no backend,
                                 ///<  running on this host
    bool IsMasterHost(void);     ///< is this the same host as the master
    bool IsMasterHost(const QString &host); ///< is specified host the master
    bool IsMasterBackend(void);  ///< is this the actual MBE process
    static bool BackendIsRunning(void); ///< a backend process is running on this host

    bool IsThisBackend(const QString &addr);    ///< is this address mapped to this backend host
    bool IsThisHost(const QString &addr); ///< is this address mapped to this host
    bool IsThisHost(const QString &addr, const QString &host);

    void BlockShutdown(void);
    void AllowShutdown(void);
    bool IsBlockingClient(void) const; ///< is this client blocking shutdown

    void SetWOLAllowed(bool allow);
    bool IsWOLAllowed() const;

    bool SendReceiveStringList(QStringList &strlist, bool quickTimeout = false,
                               bool block = true);
    void SendMessage(const QString &message);
    void SendEvent(const MythEvent &event);
    void SendSystemEvent(const QString &msg);
    void SendHostSystemEvent(const QString &msg, const QString &hostname,
                             const QString &args);

    void SetGUIObject(QObject *gui);
    QObject *GetGUIObject(void);
    QObject *GetGUIContext(void);
    bool HasGUI(void) const;
    bool IsUIThread(void);

    MythDB *GetDB(void);
    MDBManager *GetDBManager(void);
    MythScheduler *GetScheduler(void);

    bool IsDatabaseIgnored(void) const;
    DatabaseParams GetDatabaseParams(void)
        { return GetDB()->GetDatabaseParams(); }

    void SaveSetting(const QString &key, int newValue);
    void SaveSetting(const QString &key, const QString &newValue);
    QString GetSetting(const QString &key, const QString &defaultval = "");
    bool SaveSettingOnHost(const QString &key, const QString &newValue,
                           const QString &host);
    void SaveBoolSetting(const QString &key, bool newValue)
        { SaveSetting(key, static_cast<int>(newValue)); }

    // Convenience setting query methods
    bool GetBoolSetting(const QString &key, bool defaultval = false);
    int GetNumSetting(const QString &key, int defaultval = 0);
    int GetBoolSetting(const QString &key, int defaultval) = delete;
    bool GetNumSetting(const QString &key, bool defaultvalue) = delete;
    double GetFloatSetting(const QString &key, double defaultval = 0.0);
    void GetResolutionSetting(const QString &type, int &width, int &height,
                              double& forced_aspect, double &refresh_rate,
                              int index=-1);
    void GetResolutionSetting(const QString &type, int &width, int &height,
                              int index=-1);

    QString GetSettingOnHost(const QString &key, const QString &host,
                             const QString &defaultval = "");
    bool GetBoolSettingOnHost(const QString &key, const QString &host,
                             bool defaultval = false);
    int GetNumSettingOnHost(const QString &key, const QString &host,
                            int defaultval = 0);
    int GetBoolSettingOnHost(const QString &key, const QString &host,
                             int defaultval) = delete;
    bool GetNumSettingOnHost(const QString &key, const QString &host,
                            bool defaultval = false) = delete;
    double GetFloatSettingOnHost(const QString &key, const QString &host,
                                 double defaultval = 0.0);

    QString GetBackendServerIP(void);
    QString GetBackendServerIP(const QString &host);
    QString GetBackendServerIP4(void);
    QString GetBackendServerIP4(const QString &host);
    QString GetBackendServerIP6(void);
    QString GetBackendServerIP6(const QString &host);
    QString GetMasterServerIP(void);
    static int GetMasterServerPort(void);
    int GetMasterServerStatusPort(void);
    int GetBackendServerPort(void);
    int GetBackendServerPort(const QString &host);
    int GetBackendStatusPort(void);
    int GetBackendStatusPort(const QString &host);

    bool GetScopeForAddress(QHostAddress &addr) const;
    void SetScopeForAddress(const QHostAddress &addr);
    void SetScopeForAddress(const QHostAddress &addr, int scope);
    enum ResolveType { ResolveAny = -1, ResolveIPv4 = 0, ResolveIPv6 = 1 };
    QString resolveSettingAddress(const QString &name,
                                  const QString &host = QString(),
                                  ResolveType type = ResolveAny,
                                  bool keepscope = false);
    static QString resolveAddress(const QString &host,
                                  ResolveType type = ResolveAny,
                                  bool keepscope = false) ;
    bool CheckSubnet(const QAbstractSocket *socket);
    bool CheckSubnet(const QHostAddress &peer);

    void ClearSettingsCache(const QString &myKey = QString(""));
    void ActivateSettingsCache(bool activate = true);
    void OverrideSettingForSession(const QString &key, const QString &value);
    void ClearOverrideSettingForSession(const QString &key);

    void dispatch(const MythEvent &event);

    void InitPower(bool Create = true);
    void InitLocale(void);
    void ReInitLocale(void);
    MythLocale *GetLocale(void) const;
    QLocale GetQLocale(void);
    void SaveLocaleDefaults(void);
    QString GetLanguage(void);
    QString GetLanguageAndVariant(void);
    void ResetLanguage(void);
    void ResetSockets(void);

    void RegisterForPlayback(QObject *sender, const char *method);
    void UnregisterForPlayback(QObject *sender);
    void WantingPlayback(QObject *sender);
    bool InWantingPlayback(void);
    void TVInWantingPlayback(bool b);

    MythSessionManager *GetSessionManager(void);

    // Plugin related methods
    static bool TestPluginVersion(const QString &name, const QString &libversion,
                          const QString &pluginversion);

    void SetPluginManager(MythPluginManager *pmanager);
    MythPluginManager *GetPluginManager(void);

    // Set when QEventLoop has been stopped and about to exit application
    void SetExiting(bool exiting = true);
    bool IsExiting(void);

    void RegisterFileForWrite(const QString &file, uint64_t size = 0LL);
    void UnregisterFileForWrite(const QString &file);
    bool IsRegisteredFileForWrite(const QString &file);

    // Test Harness help
    void setTestIntSettings(QMap<QString,int>& overrides);
    void setTestFloatSettings(QMap<QString,double>& overrides);
    void setTestStringSettings(QMap<QString,QString>& overrides);

    // signal related methods
    void WaitUntilSignals(std::vector<CoreWaitInfo> & sigs) const;
    void emitTVPlaybackStarted(void)            { emit TVPlaybackStarted(); }
    void emitTVPlaybackStopped(void)            { emit TVPlaybackStopped(); }
    void emitTVPlaybackSought(qint64 position)  { emit TVPlaybackSought(position);
                                                  emit TVPlaybackSought();}
    void emitTVPlaybackPaused(void)             { emit TVPlaybackPaused(); }
    void emitTVPlaybackUnpaused(void)           { emit TVPlaybackUnpaused(); }
    void emitTVPlaybackAborted(void)            { emit TVPlaybackAborted(); }
    void emitTVPlaybackPlaying(void)            { emit TVPlaybackPlaying(); }


  signals:
    void TVPlaybackStarted(void);
    //// TVPlaybackStopped signal should be used in combination with
    //// InWantingPlayback() and treat it accordingly
    void TVPlaybackStopped(void);
    void TVPlaybackSought(qint64 position);
    void TVPlaybackSought(void);
    void TVPlaybackPaused(void);
    void TVPlaybackUnpaused(void);
    void TVPlaybackAborted(void);
    void TVPlaybackAboutToStart(void);
    void TVPlaybackPlaying(void);

  private:
    Q_DISABLE_COPY(MythCoreContext)
    MythCoreContextPrivate *d {nullptr}; // NOLINT(readability-identifier-naming)

    void connected(MythSocket *sock) override { (void)sock; } //MythSocketCBs
    void connectionFailed(MythSocket *sock) override { (void)sock; } //MythSocketCBs
    void connectionClosed(MythSocket *sock) override; // MythSocketCBs
    void readyRead(MythSocket *sock) override; // MythSocketCBs

    QMap<QString,int>     m_testOverrideInts    {};
    QMap<QString,double>  m_testOverrideFloats  {};
    QMap<QString,QString> m_testOverrideStrings {};
};

/// This global variable contains the MythCoreContext instance for the app
extern MBASE_PUBLIC MythCoreContext *gCoreContext;

#endif

/* vim: set expandtab tabstop=4 shiftwidth=4: */
