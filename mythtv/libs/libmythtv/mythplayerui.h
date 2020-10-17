﻿#ifndef MYTHPLAYERUI_H
#define MYTHPLAYERUI_H

// MythTV
#include "mythplayervisualiserui.h"
#include "mythvideoscantracker.h"
#include "DetectLetterbox.h"
#include "jitterometer.h"
#include "mythplayer.h"

class MTV_PUBLIC MythPlayerUI : public MythPlayerVisualiserUI, public MythVideoScanTracker
{
    Q_OBJECT

  public slots:
    void ChangeOSDDebug();

  protected slots:
    void UpdateOSDDebug();

  public:
    MythPlayerUI(MythMainWindow* MainWindow, TV* Tv, PlayerContext* Context, PlayerFlags Flags);

    bool StartPlaying();
    virtual void InitialSeek();
    virtual void EventLoop();
    void         ReinitVideo(bool ForceUpdate) override;
    virtual void VideoStart();
    virtual void EventStart();
    virtual bool VideoLoop();
    virtual void PreProcessNormalFrame();
    void ChangeSpeed() override;
    void ReleaseNextVideoFrame(MythVideoFrame* Frame, int64_t Timecode, bool Wrap = true) override;
    void SetVideoParams(int Width, int Height, double FrameRate, float Aspect,
                        bool ForceUpdate, int ReferenceFrames,
                        FrameScanType Scan = kScan_Ignore,
                        const QString& CodecName = QString()) override;
    bool DoFastForwardSecs(float Seconds, double Inaccuracy, bool UseCutlist);
    bool DoRewindSecs(float Seconds, double Inaccuracy, bool UseCutlist);
    void GetPlaybackData(InfoMap& Map);
    void GetCodecDescription(InfoMap& Map);
    void ToggleAdjustFill(AdjustFillMode Mode = kAdjustFill_Toggle);
    bool CanSupportDoubleRate();
    void SetWatched(bool ForceWatched = false);
    virtual void SetBookmark(bool Clear = false);

    // FIXME - should be private
    DetectLetterbox m_detectLetterBox { this };

    // N.B. Editor - keep ringfenced and move into subclass
    bool EnableEdit();
    void DisableEdit(int HowToSave);
    bool HandleProgramEditorActions(QStringList& Actions);
    uint64_t GetNearestMark(uint64_t Frame, bool Right);
    bool IsTemporaryMark(uint64_t Frame);
    bool HasTemporaryMark();
    bool IsCutListSaved();
    bool DeleteMapHasUndo();
    bool DeleteMapHasRedo();
    QString DeleteMapGetUndoMessage();
    QString DeleteMapGetRedoMessage();
    void HandleArbSeek(bool Direction);
    // End editor stuff

  protected:
    void InitFrameInterval() override;
    virtual void DisplayPauseFrame();
    virtual void DisplayNormalFrame(bool CheckPrebuffer = true);

    void FileChanged();
    void RefreshPauseFrame();
    void RenderVideoFrame(MythVideoFrame* Frame, FrameScanType Scan, bool Prepare, int64_t Wait);
    void DoDisplayVideoFrame(MythVideoFrame* Frame, int64_t Due);
    void EnableFrameRateMonitor(bool Enable = false);

    Jitterometer    m_outputJmeter { "Player" };

    // N.B. Editor - keep ringfenced and move into subclass
    QElapsedTimer   m_editUpdateTimer;
    float           m_speedBeforeEdit  { 1.0   };
    bool            m_pausedBeforeEdit { false };

  private:
    Q_DISABLE_COPY(MythPlayerUI)

    QTimer  m_osdDebugTimer;
};

#endif
