//
// Created by Ian Parker on 14/11/2025.
//

#ifndef BLACKBOX_MAIN_H
#define BLACKBOX_MAIN_H

#define XPLM200 1
#define XPLM210 1
#define XPLM300 1
#define XPLM301 1

#include <deque>
#include <filesystem>
#include <vector>

#include <XPLMProcessing.h>
#include <XPLMMenus.h>
#include <XPLMDataAccess.h>

#include "screenshots.h"
#include "blackbox/datastore.h"
#include "blackbox/logger.h"
#include "blackbox/state.h"

#include "screenshots.h"

class Writer;

class XPLogPrinter : public BlackBox::LogPrinter
{
public:
    virtual ~XPLogPrinter() = default;

    void printf(const char* message, ...) override
    {
        va_list va;
        va_start(va, message);

        char buf[4096];
        vsnprintf(buf, 4094, message, va);
        XPLMDebugString(buf);

        va_end(va);
    }
};

class StatusWindow;

struct DataSet
{
    float maxTime = 1.0f;
    std::deque<float> data;
    std::deque<float> time;

    void add(const float v, const float t)
    {
        data.push_back(v);
        time.push_back(t);

        while (!time.empty() && (t - time.front()) > maxTime)
        {
            time.pop_front();
            data.pop_front();
        }
    }

    [[nodiscard]] float average() const
    {
        float sum = 0;
        for (const float v : data)
        {
            sum += v;
        }
        return sum / static_cast<float>(data.size());
    }

    void reset()
    {
        data.clear();
        time.clear();
    }
};

class BlackBoxPlugin : public BlackBox::Logger
{
    XPLogPrinter m_logPrinter;
    std::string m_message;

    std::filesystem::path m_xplaneDir;
    DataStore m_datastore;
    Flight m_currentFlight;

    std::shared_ptr<Writer> m_writer;
    float m_lastSendTime = 0;
    UFC::Coordinate m_lastPosition;

    ScreenshotWatcher m_screenshotWatcher;

    XPLMDataRef m_aircraftICAODataRef = nullptr;
    XPLMDataRef m_aircraftTailNumberDataRef = nullptr;
    XPLMDataRef m_flightIDDataRef = nullptr;
    XPLMDataRef m_latitudeDataRef = nullptr;
    XPLMDataRef m_longitudeDataRef = nullptr;
    XPLMDataRef m_elevationDataRef = nullptr;
    XPLMDataRef m_groundSpeedDataRef = nullptr;
    XPLMDataRef m_iasDataRef = nullptr;
    XPLMDataRef m_parkingBrakeDataRef = nullptr;
    XPLMDataRef m_verticalFPMDataRef = nullptr;
    XPLMDataRef m_gForceDataRef = nullptr;
    XPLMDataRef m_onGroundAnyDataRef = nullptr;
    XPLMDataRef m_onGroundAllDataRef = nullptr;
    XPLMDataRef m_aglDataRef = nullptr;
    XPLMDataRef m_pitchRateDataRef = nullptr;
    XPLMDataRef m_rollRateDataRef = nullptr;
    XPLMDataRef m_yawRateDataRef = nullptr;
    XPLMDataRef m_pausedDataRef = nullptr;
    XPLMDataRef m_replayDataRef = nullptr;

    XPLMDataRef m_dateDataRef = nullptr;
    XPLMDataRef m_timeDataRef = nullptr;

    XPLMFlightLoopID m_updateFlightLoop = nullptr;

    State m_state;
    uint64_t m_yearTimestamp;

    DataSet m_fpm;

    int m_menuContainer = 0;
    XPLMMenuID m_menuId = nullptr;
    int m_showWindowMenu = 0;

    std::unique_ptr<StatusWindow> m_statusWindow = nullptr;

    static float updateCallback(float elapsedMe, float elapsedSim, int counter, void * refcon);

    void sendEvent(float elapsedSim);

    std::string findNearestAirport(float latitude, float longitude);

    void createFlight();

    void updatePosition();

    float update(float elapsedMe, float elapsedSim, int counter);

    static void menuCallback(void* menuRef, void* itemRef)
    {
        ((BlackBoxPlugin*)menuRef)->menuCallback(itemRef);
    }
    void menuCallback(void* in_item_ref);

 public:
    BlackBoxPlugin();
    ~BlackBoxPlugin() override = default;

    void reset();

    bool start();
    bool enable();
    bool disable();
    bool stop();

    void receiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam);

    void updateFlight();

    DataStore& getDataStore() { return m_datastore; }
    std::shared_ptr<Writer> getWriter() { return m_writer; }

    Flight& getCurrentFlight() { return m_currentFlight; }
    [[nodiscard]] FlightPhase getFlightPhase() const { return m_state.flightPhase; }
    [[nodiscard]] const State& getState() const { return m_state; }

    [[nodiscard]] std::filesystem::path getXPlaneDir() const { return m_xplaneDir; }

    void setMessage(const char* message, ...);
    [[nodiscard]] std::string getMessage() const { return m_message; }
};

#endif //BLACKBOX_MAIN_H
