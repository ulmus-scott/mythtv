#include <iostream>
using namespace std;

// qt
#include <qpixmap.h>
#include <qimage.h>
#include <qapplication.h>

// mythtv
#include <mythtv/mythcontext.h>
#include <mythtv/mythdialogs.h>
#include <mythtv/uitypes.h>
#include <mythtv/lcddevice.h>

// mythmusic
#include "miniplayer.h"
#include "musicplayer.h"
#include "decoder.h"

MiniPlayer::MiniPlayer(MythMainWindow *parent,
                    MusicPlayer *parentPlayer,
                    const char *name,
                    bool setsize)
            : MythThemedDialog(parent, name, setsize)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(1);
    m_parentPlayer = parentPlayer;

    m_displayTimer = new QTimer(this);
    connect(m_displayTimer, SIGNAL(timeout()), this, SLOT(timerTimeout()));

    m_infoTimer = new QTimer(this);
    connect(m_infoTimer, SIGNAL(timeout()), this, SLOT(showInfoTimeout()));

    wireupTheme();

    gPlayer->setListener(this);

    if (gPlayer->getCurrentMetadata())
    {
        m_maxTime = gPlayer->getCurrentMetadata()->Length() / 1000;
        updateTrackInfo(gPlayer->getCurrentMetadata());

        if (!gPlayer->isPlaying())
        {
            QString time_string = getTimeString(m_maxTime, 0);

            if (m_timeText)
                m_timeText->SetText(time_string);
            if (m_infoText)
                m_infoText->SetText(tr("Stopped"));
        }
    }

    m_showingInfo = false;
}

MiniPlayer::~MiniPlayer(void)
{
    gPlayer->setListener(NULL);

    m_displayTimer->deleteLater();
    m_displayTimer = NULL;

    m_infoTimer->deleteLater();
    m_infoTimer = NULL;

    if (class LCD *lcd = LCD::Get()) 
        lcd->switchToTime ();
}

void MiniPlayer::showPlayer(int showTime)
{
    m_displayTimer->start(showTime * 1000, true);
    exec();
}

void MiniPlayer::timerTimeout(void)
{
    done(Accepted);
}

void MiniPlayer::wireupTheme(void)
{
    QString theme_file = QString("music-");

    if (!loadThemedWindow("miniplayer", theme_file))
    {
        VERBOSE(VB_GENERAL, "MiniPlayer: cannot load theme!");
        done(0);
        return;
    }

    // get dialog size from player_container area
    LayerSet *container = getContainer("player_container");

    if (!container)
    {
        cerr << "MiniPlayer: cannot find the 'player_container'"
                " in your theme" << endl;
        done(0);
        return;
    }

    m_popupWidth = container->GetAreaRect().width();
    m_popupHeight = container->GetAreaRect().height();
    setFixedSize(QSize(m_popupWidth, m_popupHeight));

    int xbase, width, ybase, height;
    float wmult, hmult;
    gContext->GetScreenSettings(xbase, width, wmult, ybase, height, hmult);
    QRect tlwg = QRect(0, 0, width, height);

    QPoint newpos;

    PlayerPosition preferredPos = MP_POSTOPDIALOG;

    if (preferredPos == MP_POSTOPDIALOG)
    {
        newpos = QPoint(tlwg.width() / 2 - m_popupWidth / 2, 5);
        this->move(newpos);
    }
    else if (preferredPos == MP_POSBOTTOMDIALOG)
    {
        newpos = QPoint(tlwg.width() / 2 - m_popupWidth / 2, 
                        tlwg.height() - 5 - m_popupHeight);
        this->move(newpos);
    }
    else if (preferredPos == MP_POSCENTERDIALOG)
    {
        newpos = QPoint(tlwg.width() / 2 - m_popupWidth / 2, 
                        tlwg.height() / 2 - m_popupHeight / 2);
        this->move(newpos);
    }

    m_titleText = getUITextType("title_text");
    m_artistText = getUITextType("artist_text");
    m_timeText = getUITextType("time_text");
    m_infoText = getUITextType("info_text");
    m_albumText = getUITextType("album_text");
    m_ratingsImage = getUIRepeatedImageType("ratings_image");
    m_coverImage = getUIImageType("cover_image");
}

void MiniPlayer::show()
{
    grabKeyboard();

    MythDialog::show();
}

void MiniPlayer::hide()
{
    releaseKeyboard();

    MythDialog::hide();
}

void MiniPlayer::keyPressEvent(QKeyEvent *e)
{
    bool handled = false;
    QStringList actions;
    if (gContext->GetMainWindow()->TranslateKeyPress("Music", e, actions, false))
    {
        for (unsigned int i = 0; i < actions.size() && !handled; i++)
        {
            QString action = actions[i];
            handled = true;
            if (action == "ESCAPE")
                done(0);
            else if (action == "SELECT")
                    m_displayTimer->stop();
            else if (action == "NEXTTRACK")
                gPlayer->next();
            else if (action == "PREVTRACK")
                gPlayer->previous();
            else if (action == "FFWD")
                seekforward();
            else if (action == "RWND")
                seekback();
            else if (action == "PAUSE")
            {
                if (gPlayer->isPlaying())
                    gPlayer->pause();
                else
                {
                    if (gPlayer->isPlaying())
                        gPlayer->stop();

                    if (gPlayer->getOutput() && gPlayer->getOutput()->GetPause())
                    {
                        gPlayer->pause();
                        return;
                    }

                    gPlayer->play();
                }
            }
            else if (action == "STOP")
            {
                gPlayer->stop();

                QString time_string = getTimeString(m_maxTime, 0);

                if (m_timeText)
                    m_timeText->SetText(time_string);
                if (m_infoText)
                    m_infoText->SetText("");
            }
            else if (action == "VOLUMEDOWN")
            {
                if (gPlayer->getOutput())
                {
                    gPlayer->getOutput()->AdjustCurrentVolume(-2);
                    showVolume();
                }
            }
            else if (action == "VOLUMEUP")
            {
                if (gPlayer->getOutput())
                {
                    gPlayer->getOutput()->AdjustCurrentVolume(2);
                    showVolume();
                }
            }
            else if (action == "MUTE")
            {
                if (gPlayer->getOutput())
                {
                    gPlayer->getOutput()->ToggleMute();

                    if (m_infoText)
                    {
                        m_showingInfo = true;
                        if (gPlayer->getOutput()->GetMute())
                            m_infoText->SetText(tr("Mute: On"));
                        else
                            m_infoText->SetText(tr("Mute: Off"));

                        m_infoTimer->start(5000, true);
                    }
                }
            }
            else if (action == "THMBUP")
                increaseRating();
            else if (action == "THMBDOWN")
                decreaseRating();
            else if (action == "1")
            {
                gPlayer->toggleShuffleMode();
                showShuffleMode();
            }
            else if (action == "2")
            {
                gPlayer->toggleRepeatMode();
                showRepeatMode();
            }
            else
                handled = false;
        }
    }
}

void MiniPlayer::customEvent(QCustomEvent *event)
{

    switch ((int)event->type()) 
    {
        case OutputEvent::Playing:
        {
            if (gPlayer->getCurrentMetadata())
            {
                m_maxTime = gPlayer->getCurrentMetadata()->Length() / 1000;
                updateTrackInfo(gPlayer->getCurrentMetadata());
            }
            break;
        }

        case OutputEvent::Buffering:
        {
            break;
        }

        case OutputEvent::Paused:
        {
            break;
        }

        case OutputEvent::Info:
       {
            OutputEvent *oe = (OutputEvent *) event;

            int rs;
            m_currTime = rs = oe->elapsedSeconds();

            QString time_string = getTimeString(rs, m_maxTime);

            QString info_string;

            //  Hack around for cd bitrates
            if (oe->bitrate() < 2000)
            {
                info_string.sprintf("%d "+tr("kbps")+ "   %.1f "+ tr("kHz")+ "   %s "+ tr("ch"),
                                   oe->bitrate(), float(oe->frequency()) / 1000.0,
                                   oe->channels() > 1 ? "2" : "1");
            }
            else
            {
                info_string.sprintf("%.1f "+ tr("kHz")+ "   %s "+ tr("ch"),
                                   float(oe->frequency()) / 1000.0,
                                   oe->channels() > 1 ? "2" : "1");
            }

            if (m_timeText)
                m_timeText->SetText(time_string);
            if (m_infoText && !m_showingInfo)
                m_infoText->SetText(info_string);

            if (gPlayer->getCurrentMetadata())
            {
                if (class LCD *lcd = LCD::Get())
                {
                    float percent_heard = m_maxTime <=0 ? 0.0 :
                            ((float)rs / (float)gPlayer->getCurrentMetadata()->Length()) * 1000.0;

                    QString lcd_time_string = time_string; 

                    // if the string is longer than the LCD width, remove all spaces
                    if (time_string.length() > lcd->getLCDWidth())
                        lcd_time_string.remove(' ');

                    lcd->setMusicProgress(lcd_time_string, percent_heard);
                }
            }
            break;
        }
        case OutputEvent::Error:
        {
            break;
        }
        case DecoderEvent::Stopped:
        {
            break;
        }
        case DecoderEvent::Finished:
        {
            if (gPlayer->getRepeatMode() == MusicPlayer::REPEAT_TRACK)
               gPlayer->play();
            else 
                gPlayer->next();
            break;
        }
        case DecoderEvent::Error:
        {
            break;
        }
    }
    QObject::customEvent(event);
}

QString MiniPlayer::getTimeString(int exTime, int maxTime)
{
    QString time_string;

    int eh = exTime / 3600;
    int em = (exTime / 60) % 60;
    int es = exTime % 60;

    int maxh = maxTime / 3600;
    int maxm = (maxTime / 60) % 60;
    int maxs = maxTime % 60;

    if (maxTime <= 0) 
    {
        if (eh > 0) 
            time_string.sprintf("%d:%02d:%02d", eh, em, es);
        else 
            time_string.sprintf("%02d:%02d", em, es);
    } 
    else
    {
        if (maxh > 0)
            time_string.sprintf("%d:%02d:%02d / %02d:%02d:%02d", eh, em,
                    es, maxh, maxm, maxs);
        else
            time_string.sprintf("%02d:%02d / %02d:%02d", em, es, maxm, 
                    maxs);
    }

    return time_string;
}

void MiniPlayer::updateTrackInfo(Metadata *mdata)
{
    if (m_titleText)
        m_titleText->SetText(mdata->FormatTitle());
    if (m_artistText)
        m_artistText->SetText(mdata->FormatArtist());
    if (m_albumText)
        m_albumText->SetText(mdata->Album());
    if (m_ratingsImage)
        m_ratingsImage->setRepeat(mdata->Rating());

    if (m_coverImage)
    {
        QImage image = gPlayer->getCurrentMetadata()->getAlbumArt();
        if (!image.isNull())
        {
            m_coverImage->SetImage(
                    QPixmap(image.smoothScale(m_coverImage->GetSize(true))));
        }
        else
        {
            m_coverImage->SetImage("mm_nothumb.png");
            m_coverImage->LoadImage();
        }

        m_coverImage->refresh();
    }

    LCD *lcd = LCD::Get();
    if (lcd)
    {
        // Set the Artist and Track on the LCD
        lcd->switchToMusic(mdata->Artist(), 
                       mdata->Album(), 
                       mdata->Title());
    }
}

void MiniPlayer::seekforward(void)
{
    int nextTime = m_currTime + 5;
    if (nextTime > m_maxTime)
        nextTime = m_maxTime;
    seek(nextTime);
}

void MiniPlayer::seekback(void)
{
    int nextTime = m_currTime - 5;
    if (nextTime < 0)
        nextTime = 0;
    seek(nextTime);
}

void MiniPlayer::seek(int pos)
{
    if (gPlayer->getOutput())
    {
        gPlayer->getOutput()->Reset();
        gPlayer->getOutput()->SetTimecode(pos*1000);

        if (gPlayer->getDecoder() && gPlayer->getDecoder()->running()) 
        {
            gPlayer->getDecoder()->lock();
            gPlayer->getDecoder()->seek(pos);
            gPlayer->getDecoder()->unlock();
        }

        if (!gPlayer->isPlaying())
        {
            m_currTime = pos;
            if (m_timeText)
                m_timeText->SetText(getTimeString(pos, m_maxTime));

            //showProgressBar();

            if (class LCD *lcd = LCD::Get())
            {
                float percent_heard = m_maxTime <= 0 ? 0.0 : ((float)pos /
                                      (float)m_maxTime);

                QString lcd_time_string = getTimeString(pos, m_maxTime);

                // if the string is longer than the LCD width, remove all spaces
                if (lcd_time_string.length() > lcd->getLCDWidth())
                    lcd_time_string.remove(' ');

                lcd->setMusicProgress(lcd_time_string, percent_heard);
            }
        }
    }
}

void MiniPlayer::increaseRating(void)
{
    Metadata *curMeta = gPlayer->getCurrentMetadata();

    if (!curMeta)
        return;

    if (m_ratingsImage)
    {
        curMeta->incRating();
        curMeta->persist();
        m_ratingsImage->setRepeat(curMeta->Rating());

        // if all_music is still in scope we need to keep that in sync
        if (gMusicData->all_music)
        {
            if (gPlayer->getCurrentNode())
            {
                Metadata *mdata = gMusicData->all_music->getMetadata(gPlayer->getCurrentNode()->getInt());
                if (mdata)
                {
                    mdata->incRating();
                }
            }
        }
    }
}

void MiniPlayer::decreaseRating(void)
{
    Metadata *curMeta = gPlayer->getCurrentMetadata();

    if (!curMeta)
        return;

    if (m_ratingsImage)
    {
        curMeta->decRating();
        curMeta->persist();
        m_ratingsImage->setRepeat(curMeta->Rating());

        // if all_music is still in scope we need to keep that in sync
        if (gMusicData->all_music)
        {
            if (gPlayer->getCurrentNode())
            {
                Metadata *mdata = gMusicData->all_music->getMetadata(gPlayer->getCurrentNode()->getInt());
                if (mdata)
                {
                    mdata->decRating();
                }
            }
        }
    }
}

void MiniPlayer::showInfoTimeout(void) 
{
    m_showingInfo = false;
    LCD *lcd = LCD::Get();
    Metadata * mdata = gPlayer->getCurrentMetadata();

    if (lcd && mdata)
    {
        // Set the Artist and Track on the LCD
        lcd->switchToMusic(mdata->Artist(), 
                       mdata->Album(), 
                       mdata->Title());
    }
}

void MiniPlayer::showShuffleMode(void)
{
    if (m_infoText)
    {
        m_infoTimer->stop();
        QString msg = tr("Shuffle Mode: ");
        switch (gPlayer->getShuffleMode())
        {
            case MusicPlayer::SHUFFLE_INTELLIGENT:
                msg += tr("Smart");
                if (class LCD *lcd = LCD::Get())
                    lcd->setMusicShuffle(LCD::MUSIC_SHUFFLE_SMART);
                break;
            case MusicPlayer::SHUFFLE_RANDOM:
                msg += tr("Rand");
                if (class LCD *lcd = LCD::Get())
                    lcd->setMusicShuffle(LCD::MUSIC_SHUFFLE_RAND);
                break;
            case MusicPlayer::SHUFFLE_ALBUM:
                msg += tr("Album");
                if (class LCD *lcd = LCD::Get())
                    lcd->setMusicShuffle(LCD::MUSIC_SHUFFLE_ALBUM);
                break;
            case MusicPlayer::SHUFFLE_ARTIST:
                msg += tr("Artist");
                if (class LCD *lcd = LCD::Get())
                    lcd->setMusicShuffle(LCD::MUSIC_SHUFFLE_ARTIST);
                break;
            default:
                msg += tr("None");
                if (class LCD *lcd = LCD::Get())
                    lcd->setMusicShuffle(LCD::MUSIC_SHUFFLE_NONE);
                break;
        }

        m_showingInfo = true;
        m_infoText->SetText(msg);
        m_infoTimer->start(5000, true);
    }
}

void MiniPlayer::showRepeatMode(void)
{
    if (m_infoText)
    {
        m_infoTimer->stop();
        QString msg = tr("Repeat Mode: ");
        switch (gPlayer->getRepeatMode())
        {
            case MusicPlayer::REPEAT_ALL:
                msg += tr("All");
                if (class LCD *lcd = LCD::Get())
                    lcd->setMusicRepeat (LCD::MUSIC_REPEAT_ALL);
                break;
            case MusicPlayer::REPEAT_TRACK:
                msg += tr("Track");
                if (class LCD *lcd = LCD::Get())
                    lcd->setMusicRepeat (LCD::MUSIC_REPEAT_TRACK);
                break;
            default:
                msg += tr("None");
                if (class LCD *lcd = LCD::Get())
                    lcd->setMusicRepeat (LCD::MUSIC_REPEAT_NONE);
                break;
        }

        m_showingInfo = true;
        m_infoText->SetText(msg);
        m_infoTimer->start(5000, true);
    }
}

void MiniPlayer::showVolume(void)
{
    if (m_infoText)
    {
        m_infoTimer->stop();
        QString msg = tr("Volume: ");
        float level;

        level = (float)gPlayer->getOutput()->GetCurrentVolume();

        if (gPlayer->getOutput()->GetMute())
        {
            if (class LCD *lcd = LCD::Get())
            {
                lcd->switchToVolume("Music (muted)");
                lcd->setVolumeLevel(level / (float)100);
            }
            msg += QString::number((int) level) + "% " + tr("(muted)");
        }
        else
        {
            if (class LCD *lcd = LCD::Get())
            {
                lcd->switchToVolume("Music");
                lcd->setVolumeLevel(level / (float)100);
            }
            msg += QString::number((int) level) + "%";
        }

        m_showingInfo = true;
        m_infoText->SetText(msg);
        m_infoTimer->start(5000, true);
    }
}
