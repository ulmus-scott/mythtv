#include <QtGlobal>
#include <QWindow>

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

    DisplayChanged();
    connect(display, &MythDisplay::DisplayChanged, this,
            &MythPainterGPU::DisplayChanged);
}

void MythPainterGPU::SetViewControl(ViewControls Control)
{
    m_viewControl = Control;
}

void MythPainterGPU::DisplayChanged()
{
    QScreen *screen = m_parent->GetDisplay()->GetCurrentScreen();

    m_pixelRatio = screen->devicePixelRatio();
    m_usingHighDPI = !qFuzzyCompare(m_pixelRatio, 1.0);
}
