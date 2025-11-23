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
#include <vector>
#include <XPLMProcessing.h>
#include <XPLMMenus.h>
#include <XPLMDataAccess.h>

#include "blackbox/logger.h"
#include "blackbox/state.h"

class Sender;

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
    float maxTime = 5.0f; // 5 seconds
    std::deque<float> data;
    std::deque<float> time;

    void add(float v, float t)
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
        for (float v : data)
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

    std::unique_ptr<Sender> m_sender;
    float m_lastSendTime = 0;
    UFC::Coordinate m_lastPosition;

    XPLMDataRef m_latitudeDataRef = nullptr;
    XPLMDataRef m_longitudeDataRef = nullptr;
    XPLMDataRef m_elevationDataRef = nullptr;
    XPLMDataRef m_groundSpeedDataRef = nullptr;
    XPLMDataRef m_parkingBrakeDataRef = nullptr;
    XPLMDataRef m_verticalFPMDataRef = nullptr;
    XPLMDataRef m_gForceDataRef = nullptr;
    XPLMDataRef m_onGroundAnyDataRef = nullptr;
    XPLMDataRef m_onGroundAllDataRef = nullptr;
    XPLMDataRef m_aglDataRef = nullptr;
    XPLMDataRef m_pitchRateDataRef = nullptr;
    XPLMDataRef m_localTimeDataRef = nullptr;
    XPLMDataRef m_pausedDataRef = nullptr;
    XPLMDataRef m_replayDataRef = nullptr;

    XPLMFlightLoopID m_updateFlightLoop = nullptr;

    State m_state;

    DataSet m_fpm;

    std::unique_ptr<StatusWindow> m_statusWindow = nullptr;

    static float updateCallback(float elapsedMe, float elapsedSim, int counter, void * refcon);

    void sendEvent(float elapsedSim);

    float update(float elapsedMe, float elapsedSim, int counter);

 public:
    BlackBoxPlugin();
    ~BlackBoxPlugin() override = default;

    void reset();

    bool start();
    bool enable();
    bool disable();
    bool stop();

    void receiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam);

    [[nodiscard]] float getAGL() const;
    [[nodiscard]] FlightPhase getFlightPhase() const { return m_state.flightPhase; }
    [[nodiscard]] const DataSet& getFPM() const { return m_fpm; }

    void setMessage(const char* message, ...);
    [[nodiscard]] std::string getMessage() const { return m_message; }
};

#endif //BLACKBOX_MAIN_H
