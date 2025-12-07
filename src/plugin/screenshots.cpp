//
// Created by Ian Parker on 05/12/2025.
//


#include "screenshots.h"

#include "plugin.h"
#include "writer.h"

using namespace std;
using namespace BlackBox;


ScreenshotWatcher::ScreenshotWatcher(BlackBoxPlugin* plugin) :
    Logger("ScreenshotWatcher"),
    m_plugin(plugin)
{
}

bool ScreenshotWatcher::init()
{
    FSW_STATUS res;
    res = fsw_init_library();
    if (res != FSW_OK)
    {
        fsw_last_error();
        log(ERROR, "Failed to initialize fswatch library");
        return false;
    }

    m_handle = fsw_init_session(fsevents_monitor_type);
    if (m_handle == nullptr)
    {
        fsw_last_error();
        log(ERROR, "init: Failed to create session");
        return false;
    }

    auto xplaneDir = m_plugin->getXPlaneDir();
    auto screenshotDir = filesystem::path(xplaneDir) / "Output" / "screenshots";

    res = fsw_add_path(m_handle, screenshotDir.c_str());
    if (res != FSW_OK)
    {
        log(ERROR, "Failed to add path to watch: %s", screenshotDir.c_str());
        return false;
    }

    res = fsw_set_callback(m_handle, callback, this);
    if (res != FSW_OK)
    {
        log(ERROR, "Failed to set callback: %d", res);
        return false;
    }

    fsw_set_allow_overflow(m_handle, false);

    return true;
}

void ScreenshotWatcher::start()
{
    m_watcherThread = new thread(&ScreenshotWatcher::run, this);
}

void ScreenshotWatcher::stop()
{
    fsw_stop_monitor(m_handle);
}

void ScreenshotWatcher::callback(fsw_cevent const* const events, const unsigned int event_num, void* data)
{
    static_cast<ScreenshotWatcher*>(data)->callback(events, event_num);
}

void ScreenshotWatcher::callback(fsw_cevent const* const events, const unsigned int event_num)
{
    for (unsigned int i = 0; i < event_num; i++)
    {
        auto& event = events[i];
        bool created = false;
        for (int flagIdx = 0; flagIdx < event.flags_num; flagIdx++)
        {
            if (event.flags[flagIdx] == Created)
            {
                created = true;
            }
        }
        if (created)
        {
            log(DEBUG, "New Screenshot: %s", events[i].path);

            Screenshot screenshot;
            screenshot.flightId = m_plugin->getCurrentFlight().id;
            screenshot.flightStateId = m_plugin->getWriter()->getLastStatusId();
            screenshot.timestamp = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            screenshot.path = events[i].path;

            m_plugin->getDataStore().writeScreenshot(screenshot);
        }
    }
}

void ScreenshotWatcher::run()
{
    FSW_STATUS res;
    res = fsw_start_monitor(m_handle);
    if (res != FSW_OK)
    {
        log(ERROR, "Failed to start monitor: %d", res);
    }
}
