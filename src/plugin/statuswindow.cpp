//
// Created by Ian Parker on 18/11/2025.
//

#include "statuswindow.h"

using namespace std;
using namespace UFC;

StatusWindow::StatusWindow(BlackBoxPlugin* plugin) : m_plugin(plugin)
{
}

StatusWindow::~StatusWindow()
{
    if (m_window != nullptr)
    {
        XPLMDestroyWindow(m_window);
    }
}

bool StatusWindow::open()
{
    int left;
    int bottom;
    XPLMGetScreenBoundsGlobal(&left, nullptr, nullptr, &bottom);

    XPLMCreateWindow_t params;
    params.structSize = sizeof(params);
    params.left = left + 10;
    params.bottom = bottom + 60;
    params.right = left + 250;
    params.top = bottom + 10;
    params.visible = 1;
    params.drawWindowFunc = drawCallback;
    params.handleMouseClickFunc = nullptr;
    params.handleRightClickFunc = nullptr;
    params.handleMouseWheelFunc = nullptr;
    params.handleKeyFunc = nullptr;
    params.handleCursorFunc = nullptr;
    params.refcon = this;
    params.layer = xplm_WindowLayerFloatingWindows;
    params.decorateAsFloatingWindow = 1;

    m_window = XPLMCreateWindowEx(&params);

    XPLMSetWindowPositioningMode(m_window, xplm_WindowPositionFree, -1);
    XPLMSetWindowGravity(m_window, 0, 1, 0, 1);
    XPLMSetWindowResizingLimits(m_window, 100, 50, 500, 100);
    XPLMSetWindowTitle(m_window, "BlackBox Flight Recorder");

    return true;
}

void StatusWindow::draw(XPLMWindowID in_window_id) const
{
    int char_height;
    XPLMGetFontDimensions(xplmFont_Proportional, nullptr, &char_height, nullptr);

    int l;
    int t;
    XPLMGetWindowGeometry(in_window_id, &l, &t, nullptr, nullptr);

    string phase;
    switch (m_plugin->getFlightPhase())
    {
        using enum FlightPhase;
        case INIT: phase = "Init"; break;
        case PARKED: phase = "Parked"; break;
        case TAXI: phase = "Taxi"; break;
        case TAKE_OFF: phase = "Take Off"; break;
        case FLIGHT: phase = "Flight"; break;
        case APPROACH: phase = "Approach"; break;
        case LANDING: phase = "Landing"; break;
        case CRASHED: phase = "Crashed"; break;
    }

    float agl = m_plugin->getAGL();

    float col_white[] = {1.0, 1.0, 1.0};
    char buf[1024];
    snprintf(buf, 1024, "Phase: %s: FPM: %0.2f (%ld samples), AGL=%0.2f", phase.c_str(), m_plugin->getFPM().average(), m_plugin->getFPM().data.size(), agl);
    XPLMDrawString(col_white, l + 10, t - (char_height + 5), buf, nullptr, xplmFont_Proportional);

    snprintf(buf, 1024, "Last message: %s", m_plugin->getMessage().c_str());
    XPLMDrawString(col_white, l + 10, t - ((char_height * 2) + 10), buf, nullptr, xplmFont_Proportional);
}

