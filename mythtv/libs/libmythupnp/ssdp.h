//////////////////////////////////////////////////////////////////////////////
// Program Name: ssdp.h
// Created     : Oct. 1, 2005
//
// Purpose     : SSDP Discovery Service Implmenetation
//
// Copyright (c) 2005 David Blain <dblain@mythtv.org>
//
// Licensed under the GPL v2 or later, see LICENSE for details
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SSDP_H
#define SSDP_H

#include <cstdint>

#include <QHostAddress>
#include <QMutex>
#include <QRegularExpression>
#include <QString>

#include "libmythbase/mthread.h"

#include "upnpexp.h"
#include "msocketdevice.h"
#include "upnputil.h"

static constexpr const char* SSDP_GROUP { "239.255.255.250" };
static constexpr uint16_t SSDP_PORT       { 1900 };

enum SSDPRequestType : std::uint8_t
{
    SSDP_Unknown        = 0,
    SSDP_MSearch        = 1,
    SSDP_MSearchResp    = 2,
    SSDP_Notify         = 3
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// SSDPThread Class Definition  (Singleton)
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class UPNP_PUBLIC SSDP : public MThread
{
    private:
        // Singleton instance used by all.
        static SSDP*        g_pSSDP;  

        QRegularExpression  m_procReqLineExp        {"\\s+"};
        MSocketDevice* m_socket                     {nullptr};

        int                 m_nPort                 {SSDP_PORT};
        int                 m_nServicePort          {0};

        class UPnpNotifyTask* m_pNotifyTask         {nullptr};
        bool                m_bAnnouncementsEnabled {false};

        bool                m_bTermRequested        {false};
        QMutex              m_lock;

    private:

        // ------------------------------------------------------------------
        // Private so the singleton pattern can be enforced.
        // ------------------------------------------------------------------

        SSDP   ();
        
    protected:

        bool    ProcessSearchRequest ( const QStringMap &sHeaders,
                                       const QHostAddress&  peerAddress,
                                       quint16       peerPort ) const;
        static bool    ProcessSearchResponse( const QStringMap &sHeaders );
        static bool    ProcessNotify        ( const QStringMap &sHeaders );

        bool    IsTermRequested      ();

        static QString GetHeaderValue    ( const QStringMap &headers,
                                    const QString    &sKey,
                                    const QString    &sDefault );

        void    ProcessData       ( MSocketDevice *pSocket );

        SSDPRequestType ProcessRequestLine( const QString &sLine );

        void    run() override; // MThread
 
    public:

        static inline const QString kBackendURI = "urn:schemas-mythtv-org:device:MasterMediaServer:1";

        static SSDP* Instance();
        static void Shutdown();

            ~SSDP() override;

        void RequestTerminate(void);

        /** @brief Send a SSDP discover multicast datagram.
        @note This needs an SSDP instance to process the replies and add to the SSDPCache.
        */
        static void PerformSearch(const QString &sST, std::chrono::seconds timeout = 2s);

        void EnableNotifications ( int nServicePort );
        void DisableNotifications();
};

#endif // SSDP_H
