// MythTV
#include "mythcontext.h"
#include "mythmainwindow.h"
#include "mythplayer.h"
#include "videooutbase.h"
#include "videodisplayprofile.h"
#include "osd.h"
#include "mythuihelper.h"
#include "mythrender_opengl.h"
#include "mythpainter_ogl.h"
#include "mythcodeccontext.h"
#include "mythopenglinterop.h"
#include "videoout_opengl.h"

#define LOC QString("VidOutGL: ")

/*! \brief Generate the list of available OpenGL profiles
 *
 * \note This could be improved by eliminating unsupported profiles at run time -
 * but it is currently called statically and hence options would be fixed and unable
 * to reflect changes in UI render device.
*/
void VideoOutputOpenGL::GetRenderOptions(render_opts &Options,
                                         QStringList &SoftwareDeinterlacers)
{
    QStringList gldeints;
    gldeints << "opengllinearblend" <<
                "openglonefield" <<
                "openglkerneldeint" <<
                "openglbobdeint" <<
                "opengldoubleratelinearblend" <<
                "opengldoubleratekerneldeint" <<
                "opengldoubleratefieldorder";

    QStringList safe;
    safe << "opengl" << "opengl-yv12" << "opengl-hquyv";

    // all profiles can handle all software frames
    (*Options.safe_renderers)["dummy"].append(safe);
    (*Options.safe_renderers)["nuppel"].append(safe);
    if (Options.decoders->contains("ffmpeg"))
        (*Options.safe_renderers)["ffmpeg"].append(safe);
    if (Options.decoders->contains("openmax"))
        (*Options.safe_renderers)["openmax"].append(safe);
    if (Options.decoders->contains("mediacodec-dec"))
        (*Options.safe_renderers)["mediacodec-dec"].append(safe);
    if (Options.decoders->contains("vaapi-dec"))
        (*Options.safe_renderers)["vaapi-dec"].append(safe);
    if (Options.decoders->contains("vdpau-dec"))
        (*Options.safe_renderers)["vdpau-dec"].append(safe);
    if (Options.decoders->contains("nvdec-dec"))
        (*Options.safe_renderers)["nvdec-dec"].append(safe);
    if (Options.decoders->contains("vtb-dec"))
        (*Options.safe_renderers)["vtb-dec"].append(safe);

    // OpenGL UYVY
    Options.renderers->append("opengl");
    Options.deints->insert("opengl", SoftwareDeinterlacers + gldeints);
    (*Options.osds)["opengl"].append("opengl2");
    Options.priorities->insert("opengl", 65);

    // OpenGL HQ UYV
    Options.renderers->append("opengl-hquyv");
    Options.deints->insert("opengl-hquyv", SoftwareDeinterlacers + gldeints);
    (*Options.osds)["opengl-hquyv"].append("opengl2");
    Options.priorities->insert("opengl-hquyv", 60);

    // OpenGL YV12
    Options.renderers->append("opengl-yv12");
    Options.deints->insert("opengl-yv12", SoftwareDeinterlacers + gldeints);
    (*Options.osds)["opengl-yv12"].append("opengl2");
    Options.priorities->insert("opengl-yv12", 65);

#if defined(USING_VAAPI) || defined (USING_VTB) || defined (USING_MEDIACODEC) || defined (USING_VDPAU) || defined (USING_NVDEC)
    Options.renderers->append("opengl-hw");
    (*Options.deints)["opengl-hw"].append("none");
    (*Options.osds)["opengl-hw"].append("opengl2");
    (*Options.safe_renderers)["dummy"].append("opengl-hw");
    (*Options.safe_renderers)["nuppel"].append("opengl-hw");
    Options.priorities->insert("opengl-hw", 110);
#endif
#ifdef USING_VAAPI
    if (Options.decoders->contains("vaapi"))
        (*Options.safe_renderers)["vaapi"].append("opengl-hw");
#endif
#ifdef USING_VTB
    if (Options.decoders->contains("vtb"))
        (*Options.safe_renderers)["vtb"].append("opengl-hw");
#endif
#ifdef USING_MEDIACODEC
    if (Options.decoders->contains("mediacodec"))
        (*Options.safe_renderers)["mediacodec"].append("opengl-hw");
#endif
#ifdef USING_VDPAU
    if (Options.decoders->contains("vdpau"))
        (*Options.safe_renderers)["vdpau"].append("opengl-hw");
#endif
#ifdef USING_NVDEC
    if (Options.decoders->contains("nvdec"))
        (*Options.safe_renderers)["nvdec"].append("opengl-hw");
#endif
}

VideoOutputOpenGL::VideoOutputOpenGL(const QString &Profile)
  : VideoOutput(),
    m_render(nullptr),
    m_isGLES2(false),
    m_openGLVideo(nullptr),
    m_openGLVideoPiPActive(nullptr),
    m_openGLPainter(nullptr),
    m_videoProfile(Profile),
    m_newCodecId(kCodec_NONE),
    m_newVideoDim(),
    m_newVideoDispDim(),
    m_newAspect(0.0f),
    m_buffersCreated(false)
{
    // Setup display switching
    if (gCoreContext->GetBoolSetting("UseVideoModes", false))
        m_displayRes = DisplayRes::GetDisplayRes(true);

    // Retrieve render context
    m_render = MythRenderOpenGL::GetOpenGLRender();
    if (!m_render)
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + "Failed to retrieve OpenGL context");
        return;
    }

    // Retain and lock
    m_render->IncrRef();
    OpenGLLocker locker(m_render);

    // Disallow unsupported video texturing on GLES2
    if (m_render->isOpenGLES() && m_render->format().majorVersion() < 3)
    {
        LOG(VB_GENERAL, LOG_INFO, LOC + "Disabling unsupported texture formats for GLES2");
        m_isGLES2 = true;
    }

    // Retrieve OpenGL painter
    MythMainWindow *win = MythMainWindow::getMainWindow();
    m_openGLPainter = dynamic_cast<MythOpenGLPainter*>(win->GetCurrentPainter());
    if (!m_openGLPainter)
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + "Failed to get painter");
        return;
    }

    m_openGLPainter->SetSwapControl(false);

    // Create OpenGLVideo
    QRect dvr = GetDisplayVisibleRect();
    m_openGLVideo = new OpenGLVideo(m_render, &m_videoColourSpace, m_window.GetVideoDim(),
                                    m_window.GetVideoDispDim(), dvr, m_window.GetDisplayVideoRect(),
                                    m_window.GetVideoRect(), true, m_videoProfile);

    // Connect VideoOutWindow to OpenGLVideo
    QObject::connect(&m_window, &VideoOutWindow::VideoSizeChanged, m_openGLVideo, &OpenGLVideo::SetVideoDimensions);
    QObject::connect(&m_window, &VideoOutWindow::VideoRectsChanged, m_openGLVideo, &OpenGLVideo::SetVideoRects);
    QObject::connect(&m_window, &VideoOutWindow::VisibleRectChanged, m_openGLVideo, &OpenGLVideo::SetViewportRect);
}

VideoOutputOpenGL::~VideoOutputOpenGL()
{
    DestroyBuffers();
    while (!m_openGLVideoPiPs.empty())
    {
        delete *m_openGLVideoPiPs.begin();
        m_openGLVideoPiPs.erase(m_openGLVideoPiPs.begin());
    }
    m_openGLVideoPiPsReady.clear();
    if (m_openGLPainter)
        m_openGLPainter->SetSwapControl(true);
    if (m_openGLVideo)
        delete m_openGLVideo;
    if (m_render)
        m_render->DecrRef();
    m_render = nullptr;
}

void VideoOutputOpenGL::DestroyBuffers(void)
{
    m_videoBuffers.BeginLock(kVideoBuffer_pause);
    while (m_videoBuffers.Size(kVideoBuffer_pause))
        m_videoBuffers.DiscardFrame(m_videoBuffers.Tail(kVideoBuffer_pause));
    m_videoBuffers.EndLock();

    DiscardFrames(true);
    m_videoBuffers.DeleteBuffers();
    m_videoBuffers.Reset();
    m_buffersCreated = false;
}

bool VideoOutputOpenGL::Init(const QSize &VideoDim, const QSize &VideoDispDim, float Aspect,
                             WId, const QRect &DisplayVisibleRect, MythCodecID CodecId)
{
    if (!m_render || !m_openGLPainter || !m_openGLVideo)
        return false;

    if (!gCoreContext->IsUIThread())
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + "Cannot initialise OpenGL video from this thread");
        return false;
    }

    OpenGLLocker ctx_lock(m_render);

    // Default initialisation - mainly VideoOutWindow
    if (!VideoOutput::Init(VideoDim, VideoDispDim, Aspect, 0, DisplayVisibleRect, CodecId))
        return false;

    // Ensure any new profile preferences are handled after a stream change
    if (m_dbDisplayProfile)
        m_openGLVideo->SetProfile(m_dbDisplayProfile->GetVideoRenderer());

    // Set default support for picture attributes
    InitPictureAttributes();

    // Setup display
    QSize size = m_window.GetVideoDim();
    InitDisplayMeasurements(size.width(), size.height(), false);

    // Create buffers
    if (!CreateBuffers(CodecId, m_window.GetVideoDim()))
        return false;

    // Adjust visible rect for embedding
    QRect dvr = GetDisplayVisibleRect();
    if (m_videoCodecID == kCodec_NONE)
    {
        m_render->SetViewPort(QRect(QPoint(), dvr.size()));
        MoveResize();
        return true;
    }

    if (m_window.GetPIPState() >= kPIPStandAlone)
    {
        QRect tmprect = QRect(QPoint(0,0), dvr.size());
        ResizeDisplayWindow(tmprect, true);
    }

    // Reset OpenGLVideo
    if (m_openGLVideo->IsValid())
        m_openGLVideo->ResetFrameFormat();

    // Finalise output
    MoveResize();
    return true;
}

bool VideoOutputOpenGL::InputChanged(const QSize &VideoDim, const QSize &VideoDispDim,
                                     float Aspect, MythCodecID CodecId, bool &AspectOnly,
                                     MythMultiLocker* Locks)
{
    QSize currentvideodim     = m_window.GetVideoDim();
    QSize currentvideodispdim = m_window.GetVideoDispDim();
    MythCodecID currentcodec  = m_videoCodecID;
    float currentaspect       = m_window.GetVideoAspect();

    if (m_newCodecId != kCodec_NONE)
    {
        // InputChanged has been called twice in quick succession without a call to ProcessFrame
        currentvideodim = m_newVideoDim;
        currentvideodispdim = m_newVideoDispDim;
        currentcodec = m_newCodecId;
        currentaspect = m_newAspect;
    }

    LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Video changed: %1x%2 (%3x%4) '%5' (Aspect %6)"
                                             "-> %7x%8 (%9x%10) '%11' (Aspect %12)")
        .arg(currentvideodispdim.width()).arg(currentvideodispdim.height())
        .arg(currentvideodim.width()).arg(currentvideodim.height())
        .arg(toString(currentcodec)).arg(static_cast<double>(currentaspect))
        .arg(VideoDispDim.width()).arg(VideoDispDim.height())
        .arg(VideoDim.width()).arg(VideoDim.height())
        .arg(toString(CodecId)).arg(static_cast<double>(Aspect)));

    bool cidchanged = (CodecId != currentcodec);
    bool reschanged = (VideoDispDim != currentvideodispdim);

    // aspect ratio changes are a no-op as changes are handled at display time
    if (!cidchanged && !reschanged)
    {
        AspectOnly = true;
        return true;
    }

    // fail fast if we don't know how to display the codec
    if (!codec_sw_copy(CodecId))
    {
        // MythOpenGLInterop::GetInteropType will block if we don't release our current locks
        Locks->Unlock();
        MythOpenGLInterop::Type support = MythOpenGLInterop::GetInteropType(CodecId);
        Locks->Relock();
        if (support == MythOpenGLInterop::Unsupported)
        {
            LOG(VB_GENERAL, LOG_ERR, LOC + "New video codec is not supported.");
            m_errorState = kError_Unknown;
            return false;
        }
    }

    // delete and recreate the buffers and flag that the input has changed
    m_videoBuffers.BeginLock(kVideoBuffer_all);
    DestroyBuffers();
    m_buffersCreated = CreateBuffers(CodecId, VideoDim);
    m_videoBuffers.EndLock();
    if (!m_buffersCreated)
        return false;

    m_newCodecId= CodecId;
    m_newVideoDim = VideoDim;
    m_newVideoDispDim = VideoDispDim;
    m_newAspect = Aspect;
    return true;
}

QRect VideoOutputOpenGL::GetDisplayVisibleRect(void)
{
    QRect dvr = m_window.GetDisplayVisibleRect();

    MythMainWindow *mainwin = GetMythMainWindow();
    if (!mainwin)
        return dvr;
    QSize size = mainwin->size();

    // If the Video screen mode has vertically less pixels
    // than the GUI screen mode - OpenGL coordinate adjustments
    // must be made to put the video at the top of the display
    // area instead of at the bottom.
    if (dvr.height() < size.height())
        dvr.setTop(dvr.top()-size.height()+dvr.height());

    // If the Video screen mode has horizontally less pixels
    // than the GUI screen mode - OpenGL width must be set
    // as the higher GUI width so that the Program Guide
    // invoked from playback is not cut off.
    if (dvr.width() < size.width())
        dvr.setWidth(size.width());
    return dvr;
}

bool VideoOutputOpenGL::CreateBuffers(MythCodecID CodecID, QSize Size)
{
    if (m_buffersCreated)
        return true;

    if (codec_is_mediacodec_dec(CodecID))
    {
        m_videoBuffers.Init(VideoBuffers::GetNumBuffers(FMT_MEDIACODEC), false, 1, 4, 2, 1);
        return m_videoBuffers.CreateBuffers(FMT_YV12, Size.width(), Size.height());
    }
    else if (codec_is_vtb_dec(CodecID))
    {
        m_videoBuffers.Init(VideoBuffers::GetNumBuffers(FMT_VTB), false, 1, 4, 2, 1);
        return m_videoBuffers.CreateBuffers(FMT_YV12, Size.width(), Size.height());
    }

    if (codec_is_mediacodec(CodecID))
        return m_videoBuffers.CreateBuffers(FMT_MEDIACODEC, Size, false, 1, 2, 2, 1);
    else if (codec_is_vaapi(CodecID))
        return m_videoBuffers.CreateBuffers(FMT_VAAPI, Size, false, 2, 1, 4, 1);
    else if (codec_is_vtb(CodecID))
        return m_videoBuffers.CreateBuffers(FMT_VTB, Size, false, 1, 4, 2, 1);
    else if (codec_is_vdpau(CodecID))
        return m_videoBuffers.CreateBuffers(FMT_VDPAU, Size, false, 2, 1, 4, 1);
    else if (codec_is_nvdec(CodecID))
        return m_videoBuffers.CreateBuffers(FMT_NVDEC, Size, false, 2, 1, 4, 1);
    return m_videoBuffers.CreateBuffers(FMT_YV12, Size, false, 1, 12, 4, 2);
}

void VideoOutputOpenGL::ProcessFrame(VideoFrame *Frame, OSD */*osd*/,
                                     const PIPMap &PiPPlayers,
                                     FrameScanType Scan)
{
    if (!m_render)
        return;

    OpenGLLocker ctx_lock(m_render);

    // Process input changes
    if (m_newCodecId != kCodec_NONE)
    {
        // Ensure we don't lose embedding through program changes.
        bool wasembedding = m_window.IsEmbedding();
        QRect oldrect;
        if (wasembedding)
        {
            oldrect = m_window.GetEmbeddingRect();
            StopEmbedding();
        }

        bool ok = Init(m_newVideoDim, m_newVideoDispDim, m_newAspect,
                       0, m_window.GetDisplayVisibleRect(), m_newCodecId);
        m_newCodecId = kCodec_NONE;
        m_newVideoDim = QSize();
        m_newVideoDispDim = QSize();
        m_newAspect = 0.0f;

        if (wasembedding && ok)
            EmbedInWidget(oldrect);

        if (!ok)
            return;
    }

    if (VERBOSE_LEVEL_CHECK(VB_GPU, LOG_INFO))
        m_render->logDebugMarker(LOC + "PROCESS_FRAME_START");

    bool swframe = Frame ? !format_is_hw(Frame->codec) : false;
    bool dummy   = Frame ? Frame->dummy : false;

    // software deinterlacing
    if (!dummy && swframe)
        m_deinterlacer.Filter(Frame, Scan);

    if (!m_window.IsEmbedding())
    {
        m_openGLVideoPiPActive = nullptr;
        ShowPIPs(Frame, PiPPlayers);
    }

    if (m_openGLVideo && swframe && !dummy)
        m_openGLVideo->ProcessFrame(Frame, Scan);

    if (VERBOSE_LEVEL_CHECK(VB_GPU, LOG_INFO))
        m_render->logDebugMarker(LOC + "PROCESS_FRAME_END");
}

void VideoOutputOpenGL::PrepareFrame(VideoFrame *Frame, FrameScanType Scan, OSD *Osd)
{
    if (!m_render)
        return;

    if (m_newCodecId != kCodec_NONE)
        return; // input changes need to be handled in ProcessFrame

    OpenGLLocker ctx_lock(m_render);

    if (VERBOSE_LEVEL_CHECK(VB_GPU, LOG_INFO))
        m_render->logDebugMarker(LOC + "PREPARE_FRAME_START");

    bool dummy = false;
    int topfieldfirst = 0;
    if (Frame)
    {
        m_framesPlayed = Frame->frameNumber + 1;
        topfieldfirst = Frame->interlaced_reversed ? !Frame->top_field_first : Frame->top_field_first;
        dummy = Frame->dummy;
    }
    else
    {
        // see VideoOutputOpenGL::DoneDisplayingFrame
        // we only retain pause frames for hardware formats
        if (m_videoBuffers.Size(kVideoBuffer_pause))
            Frame = m_videoBuffers.Tail(kVideoBuffer_pause);
    }

    m_render->BindFramebuffer(nullptr);
    if (m_dbLetterboxColour == kLetterBoxColour_Gray25)
        m_render->SetBackground(127, 127, 127, 255);
    else
        m_render->SetBackground(0, 0, 0, 255);
    m_render->ClearFramebuffer();

    // stereoscopic views
    QRect main   = m_render->GetViewPort();
    QRect first  = main;
    QRect second = main;
    bool twopass = (m_stereo == kStereoscopicModeSideBySide) || (m_stereo == kStereoscopicModeTopAndBottom);

    if (kStereoscopicModeSideBySide == m_stereo)
    {
        first  = QRect(main.left() / 2,  main.top(), main.width() / 2, main.height());
        second = first.translated(main.width() / 2, 0);
    }
    else if (kStereoscopicModeTopAndBottom == m_stereo)
    {
        first  = QRect(main.left(),  main.top() / 2, main.width(), main.height() / 2);
        second = first.translated(0, main.height() / 2);
    }

    // main UI when embedded
    if (m_window.IsEmbedding())
    {
        MythMainWindow *win = GetMythMainWindow();
        if (win && win->GetPaintWindow())
        {
            if (twopass)
                m_render->SetViewPort(first, true);
            win->GetPaintWindow()->clearMask();
            win->draw(m_openGLPainter);
            if (twopass)
            {
                m_render->SetViewPort(second, true);
                win->GetPaintWindow()->clearMask();
                win->draw(m_openGLPainter);
                m_render->SetViewPort(main, true);
            }
        }
    }

    // video
    if (m_openGLVideo && !dummy)
        m_openGLVideo->PrepareFrame(Frame, topfieldfirst, Scan, m_stereo);

    // PiPs/PBPs
    if (!m_openGLVideoPiPs.empty())
    {
        QMap<MythPlayer*,OpenGLVideo*>::iterator it = m_openGLVideoPiPs.begin();
        for (; it != m_openGLVideoPiPs.end(); ++it)
        {
            if (m_openGLVideoPiPsReady[it.key()])
            {
                bool active = m_openGLVideoPiPActive == *it;
                if (twopass)
                    m_render->SetViewPort(first, true);
                (*it)->PrepareFrame(nullptr, topfieldfirst, Scan, kStereoscopicModeNone, active);
                if (twopass)
                {
                    m_render->SetViewPort(second, true);
                    (*it)->PrepareFrame(nullptr, topfieldfirst, Scan, kStereoscopicModeNone, active);
                    m_render->SetViewPort(main);
                }
            }
        }
    }

    // visualisation
    if (m_visual && m_openGLPainter && !m_window.IsEmbedding())
    {
        if (twopass)
            m_render->SetViewPort(first, true);
        m_visual->Draw(GetTotalOSDBounds(), m_openGLPainter, nullptr);
        if (twopass)
        {
            m_render->SetViewPort(second, true);
            m_visual->Draw(GetTotalOSDBounds(), m_openGLPainter, nullptr);
            m_render->SetViewPort(main);
        }
    }

    // OSD
    if (Osd && m_openGLPainter && !m_window.IsEmbedding())
    {
        if (twopass)
            m_render->SetViewPort(first, true);
        Osd->DrawDirect(m_openGLPainter, GetTotalOSDBounds().size(), true);
        if (twopass)
        {
            m_render->SetViewPort(second, true);
            Osd->DrawDirect(m_openGLPainter, GetTotalOSDBounds().size(), true);
            m_render->SetViewPort(main);
        }
    }

    m_render->Flush();

    if (VERBOSE_LEVEL_CHECK(VB_GPU, LOG_INFO))
        m_render->logDebugMarker(LOC + "PREPARE_FRAME_END");
}

/*! \brief Release a video frame back into the decoder pool.
 *
 * Software frames do not need a pause frame as OpenGLVideo
 * holds a copy of the last frame in its input textures. So
 * just release the frame.
 *
 * Hardware frames hold the underlying interop class and
 * hence access to the video texture. We cannot access them
 * without a frame so retain the most recent frame by removing
 * it from the 'used' queue and adding it to the 'pause' queue.
*/
void VideoOutputOpenGL::DoneDisplayingFrame(VideoFrame *Frame)
{
    if (!Frame)
        return;

    bool retain = format_is_hw(Frame->codec);

    m_videoBuffers.BeginLock(kVideoBuffer_pause);
    while (m_videoBuffers.Size(kVideoBuffer_pause))
    {
        VideoFrame* frame = m_videoBuffers.Dequeue(kVideoBuffer_pause);
        if (!retain || (retain && (frame != Frame)))
            VideoOutput::DoneDisplayingFrame(frame);
    }

    if (retain)
    {
        m_videoBuffers.Enqueue(kVideoBuffer_pause, Frame);
        if (m_videoBuffers.Contains(kVideoBuffer_used, Frame))
            m_videoBuffers.Remove(kVideoBuffer_used, Frame);
    }
    else
    {
        m_videoBuffers.DoneDisplayingFrame(Frame);
    }
    m_videoBuffers.EndLock();
}

VideoFrameType* VideoOutputOpenGL::DirectRenderFormats(void)
{
    static VideoFrameType openglformats[] =
        { FMT_YV12,     FMT_NV12,      FMT_YUY2,      FMT_YUV422P,   FMT_YUV444P,
          FMT_YUV420P9, FMT_YUV420P10, FMT_YUV420P12, FMT_YUV420P14, FMT_YUV420P16,
          FMT_YUV422P9, FMT_YUV422P10, FMT_YUV422P12, FMT_YUV422P14, FMT_YUV422P16,
          FMT_YUV444P9, FMT_YUV444P10, FMT_YUV444P12, FMT_YUV444P14, FMT_YUV444P16,
          FMT_P010, FMT_P016,
          FMT_NONE };
    // OpenGLES2 only allows luminance textures - no RG etc
    static VideoFrameType opengles2formats[] =
        { FMT_YV12, FMT_YUY2, FMT_YUV422P, FMT_YUV444P, FMT_NONE };
    return m_isGLES2 ? &opengles2formats[0] : &openglformats[0];
}

void VideoOutputOpenGL::Show(FrameScanType /*scan*/)
{
    if (m_render && !IsErrored())
    {
        m_render->makeCurrent();
        if (VERBOSE_LEVEL_CHECK(VB_GPU, LOG_INFO))
            m_render->logDebugMarker(LOC + "SHOW");
        m_render->swapBuffers();
        m_render->doneCurrent();
    }
}

void VideoOutputOpenGL::ClearAfterSeek(void)
{
    if (m_openGLVideo)
        m_openGLVideo->ResetTextures();
    VideoOutput::ClearAfterSeek();
}

/*! \brief Generate a list of supported OpenGL profiles.
 *
 * \note This list could be filtered based upon current feature support. This
 * would however assume an OpenGL render device (not currently a given) but more
 * importantly, filtering out a selected profile encourages the display profile
 * code to use a higher priority, non-OpenGL renderer (such as VDPAU). By not
 * filtering, we allow the OpenGL video code to fallback to a supported, reasonable
 * alternative.
*/
QStringList VideoOutputOpenGL::GetAllowedRenderers(MythCodecID CodecId, const QSize&)
{
    QStringList allowed;
    if (getenv("NO_OPENGL"))
        return allowed;

    if (codec_sw_copy(CodecId))
    {
        allowed << "opengl" << "opengl-yv12" << "opengl-hquyv";
        return allowed;
    }

    allowed += MythOpenGLInterop::GetAllowedRenderers(CodecId);
    return allowed;
}

void VideoOutputOpenGL::UpdatePauseFrame(int64_t &DisplayTimecode)
{
    m_videoBuffers.BeginLock(kVideoBuffer_used);
    VideoFrame *used = m_videoBuffers.Head(kVideoBuffer_used);
    if (used)
    {
        if (format_is_hw(used->codec))
            DoneDisplayingFrame(used);
        else
            m_openGLVideo->ProcessFrame(used);
        DisplayTimecode = used->disp_timecode;
    }
    else
    {
        LOG(VB_PLAYBACK, LOG_WARNING, LOC + "Could not update pause frame");
    }
    m_videoBuffers.EndLock();
}

void VideoOutputOpenGL::InitPictureAttributes(void)
{
    m_videoColourSpace.SetSupportedAttributes(ALL_PICTURE_ATTRIBUTES);
}

void VideoOutputOpenGL::ShowPIP(VideoFrame*, MythPlayer *PiPPlayer, PIPLocation Location)
{
    if (!PiPPlayer)
        return;

    int pipw, piph;
    VideoFrame *pipimage     = PiPPlayer->GetCurrentFrame(pipw, piph);
    const QSize pipvideodim  = PiPPlayer->GetVideoBufferSize();
    QRect       pipvideorect = QRect(QPoint(0, 0), pipvideodim);

    if ((PiPPlayer->GetVideoAspect() <= 0.0f) || !pipimage || !pipimage->buf ||
        (pipimage->codec != FMT_YV12) || !PiPPlayer->IsPIPVisible())
    {
        PiPPlayer->ReleaseCurrentFrame(pipimage);
        return;
    }

    QRect position = GetPIPRect(Location, PiPPlayer);
    QRect dvr = m_window.GetDisplayVisibleRect();

    m_openGLVideoPiPsReady[PiPPlayer] = false;
    OpenGLVideo *gl_pipchain = m_openGLVideoPiPs[PiPPlayer];
    if (!gl_pipchain)
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + "Initialise PiP");
        VideoColourSpace *colourspace = new VideoColourSpace(&m_videoColourSpace);
        m_openGLVideoPiPs[PiPPlayer] = gl_pipchain = new OpenGLVideo(m_render, colourspace,
                                                                pipvideodim, pipvideodim,
                                                                dvr, position, pipvideorect,
                                                                false, m_videoProfile);

        colourspace->DecrRef();
        if (!gl_pipchain->IsValid())
        {
            PiPPlayer->ReleaseCurrentFrame(pipimage);
            return;
        }
        gl_pipchain->SetMasterViewport(dvr.size());
    }

    if (gl_pipchain->GetVideoSize() != pipvideodim)
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + "Re-initialise PiP.");
        delete gl_pipchain;
        VideoColourSpace *colourspace = new VideoColourSpace(&m_videoColourSpace);
        m_openGLVideoPiPs[PiPPlayer] = gl_pipchain = new OpenGLVideo(m_render, colourspace,
                                                                pipvideodim, pipvideodim,
                                                                dvr, position, pipvideorect,
                                                                false, m_videoProfile);
        colourspace->DecrRef();
        if (!gl_pipchain->IsValid())
        {
            PiPPlayer->ReleaseCurrentFrame(pipimage);
            return;
        }
        gl_pipchain->SetMasterViewport(dvr.size());
    }

    if (gl_pipchain->IsValid())
    {
        gl_pipchain->SetVideoRects(position, pipvideorect);
        gl_pipchain->ProcessFrame(pipimage);
    }

    m_openGLVideoPiPsReady[PiPPlayer] = true;
    if (PiPPlayer->IsPIPActive())
        m_openGLVideoPiPActive = gl_pipchain;
    PiPPlayer->ReleaseCurrentFrame(pipimage);
}

void VideoOutputOpenGL::RemovePIP(MythPlayer *PiPPlayer)
{
    if (m_openGLVideoPiPs.contains(PiPPlayer))
    {
        m_render->makeCurrent();
        delete m_openGLVideoPiPs.take(PiPPlayer);
        m_openGLVideoPiPsReady.remove(PiPPlayer);
        m_openGLVideoPiPs.remove(PiPPlayer);
        m_render->doneCurrent();
    }
}

void VideoOutputOpenGL::MoveResizeWindow(QRect NewRect)
{
    if (m_render)
        m_render->MoveResizeWindow(NewRect);
}

void VideoOutputOpenGL::EmbedInWidget(const QRect &Rect)
{
    VideoOutput::EmbedInWidget(Rect);
    MoveResize();
}

void VideoOutputOpenGL::StopEmbedding(void)
{
    VideoOutput::StopEmbedding();
    MoveResize();
}

QStringList VideoOutputOpenGL::GetVisualiserList(void)
{
    if (m_render)
        return VideoVisual::GetVisualiserList(m_render->Type());
    return VideoOutput::GetVisualiserList();
}

MythPainter *VideoOutputOpenGL::GetOSDPainter(void)
{
    return m_openGLPainter;
}

bool VideoOutputOpenGL::CanVisualise(AudioPlayer *Audio, MythRender*)
{
    return VideoOutput::CanVisualise(Audio, m_render);
}

bool VideoOutputOpenGL::SetupVisualisation(AudioPlayer *Audio, MythRender*, const QString &Name)
{
    return VideoOutput::SetupVisualisation(Audio, m_render, Name);
}
