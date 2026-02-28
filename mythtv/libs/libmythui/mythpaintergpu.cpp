#include <QtGlobal>

// MythTV
#include "libmythbase/mythlogging.h"
#include "mythdisplay.h"
#include "mythmainwindow.h"
#include "mythpaintergpu.h"

MythPainterGPU::MythPainterGPU(MythMainWindow* Parent)
  : m_parent(Parent)
{
    MythDisplay* display = m_parent->GetDisplay();
    QScreen *screen = display->GetCurrentScreen();

    CurrentDPIChanged(screen->physicalDotsPerInch());
    connect(display, &MythDisplay::CurrentDPIChanged, this,
            &MythPainterGPU::CurrentDPIChanged);
}

void MythPainterGPU::SetViewControl(ViewControls Control)
{
    m_viewControl = Control;
}

void MythPainterGPU::CurrentDPIChanged(qreal DPI)
{
    QScreen *screen = m_parent->GetDisplay()->GetCurrentScreen();

    m_DPI = DPI;
    m_pixelRatio = screen->devicePixelRatio();
    m_usingHighDPI = !qFuzzyCompare(m_pixelRatio, 1.0);
}
