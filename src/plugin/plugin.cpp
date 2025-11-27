//
// Created by Ian Parker on 14/11/2025.
//

#include "plugin.h"
#include "statuswindow.h"
#include "writer.h"

#include <cfloat>

#include <XPLMPlugin.h>
#include <XPLMNavigation.h>

using namespace std;
using namespace BlackBox;
using namespace UFC;

BlackBoxPlugin g_bbPlugin;

constexpr float METRES_TO_FEET = 3.28084f;
constexpr float MS_TO_KNOTS = 1.943844f;

float degreesToRadians(float degrees)
{
    return degrees * M_PI / 180.0f;
}

double distance(Coordinate c1, Coordinate c2)
{
    constexpr float earthRadiusKm = 6371;

    const auto dLat = degreesToRadians(c2.latitude-c1.latitude);
    const auto dLon = degreesToRadians(c2.longitude-c1.longitude);

    const float lat1 = degreesToRadians(c1.latitude);
    const float lat2 = degreesToRadians(c2.latitude);

    const auto a =
        sinf(dLat/2.0f) * sinf(dLat/2.0f) +
        sinf(dLon/2.0f) * sinf(dLon/2.0f) *
        cosf(lat1) * cosf(lat2);
    const auto c = 2.0f * atan2f(sqrtf(a), sqrtf(1-a));
    return earthRadiusKm * c;
}

BlackBoxPlugin::BlackBoxPlugin() : Logger("BlackBox")
{
    setLogPrinter(&m_logPrinter);

    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

    reset();
}

void BlackBoxPlugin::reset()
{
    m_state.flightPhase = FlightPhase::INIT;
    m_fpm.reset();
}

bool BlackBoxPlugin::start()
{
    if (!m_datastore.init())
    {
        return false;
    }

    m_writer = make_unique<Writer>(this);

    m_aircraftICAODataRef = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
    m_flightIDDataRef = XPLMFindDataRef("sim/cockpit2/tcas/targets/flight_id");
    m_latitudeDataRef = XPLMFindDataRef("sim/flightmodel/position/latitude");
    m_longitudeDataRef = XPLMFindDataRef("sim/flightmodel/position/longitude");
    m_elevationDataRef = XPLMFindDataRef("sim/flightmodel/position/elevation");
    m_groundSpeedDataRef = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
    m_iasDataRef = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed");
    m_parkingBrakeDataRef = XPLMFindDataRef("sim/flightmodel/controls/parkbrake");
    m_verticalFPMDataRef = XPLMFindDataRef("sim/flightmodel/position/vh_ind_fpm");
    m_gForceDataRef = XPLMFindDataRef("sim/flightmodel2/misc/gforce_normal");
    m_onGroundAnyDataRef = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
    m_onGroundAllDataRef = XPLMFindDataRef("sim/flightmodel/failures/onground_all");
    m_aglDataRef = XPLMFindDataRef("sim/flightmodel/position/y_agl");
    m_pitchRateDataRef = XPLMFindDataRef("sim/flightmodel/position/true_theta");
    m_rollRateDataRef = XPLMFindDataRef("sim/flightmodel/position/true_phi");
    m_yawRateDataRef = XPLMFindDataRef("sim/flightmodel/position/true_psi");
    m_localTimeDataRef = XPLMFindDataRef("sim/time/local_time_sec");
    m_pausedDataRef = XPLMFindDataRef("sim/time/paused");
    m_replayDataRef = XPLMFindDataRef("sim/time/is_in_replay");

    XPLMCreateFlightLoop_t flightLoop;
    flightLoop.structSize = sizeof(flightLoop);
    flightLoop.callbackFunc = updateCallback;
    flightLoop.refcon = this;
    flightLoop.phase = xplm_FlightLoop_Phase_AfterFlightModel;
    m_updateFlightLoop = XPLMCreateFlightLoop(&flightLoop);

    m_statusWindow = make_unique<StatusWindow>(this);
    m_statusWindow->open();

    return true;
}

bool BlackBoxPlugin::enable()
{
    reset();

    m_writer->start();

    XPLMScheduleFlightLoop(m_updateFlightLoop, -1, true);
    return true;
}

bool BlackBoxPlugin::disable()
{
    XPLMDestroyFlightLoop(m_updateFlightLoop);

    m_writer->stop();
    return true;
}

bool BlackBoxPlugin::stop()
{
    return true;
}

void BlackBoxPlugin::receiveMessage(XPLMPluginID from, int msg, void* param)
{
    if (from != XPLM_PLUGIN_XPLANE)
    {
        return;
    }

    switch (msg)
    {
        case XPLM_MSG_PLANE_CRASHED:
            log(DEBUG, "receiveMessage: User's plane crashed!");
            m_state.eventType = EventType::CRASH;
            // TODO: Write it to the DB!
            break;

        case XPLM_MSG_AIRPORT_LOADED:
            log(DEBUG, "receiveMessage: AIRPORT_LOADED: Starting new flight!");
            createFlight();
            break;

        default:
            // Ignore
            break;
    }
}

float BlackBoxPlugin::updateCallback(float elapsedMe, float elapsedSim, int counter, void* refcon)
{
    return static_cast<BlackBoxPlugin*>(refcon)->update(elapsedMe, elapsedSim, counter);
}

void BlackBoxPlugin::sendEvent(float elapsedSim)
{
    m_state.timestamp = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

    Event event;
    event.flightId = m_currentFlight.id;
    event.state = m_state;
    m_writer->write(event);
    m_lastSendTime = elapsedSim;
}

string BlackBoxPlugin::findNearestAirport(float latitude, float longitude)
{
    XPLMNavRef airportRef = XPLMFindNavAid(
        nullptr,
        nullptr,
        &latitude,
        &longitude,
        nullptr,
        xplm_Nav_Airport);

    if (airportRef != XPLM_NAV_NOT_FOUND)
    {
        char idBuf[1024];
        char nameBuf[1024];
        XPLMGetNavAidInfo(
            airportRef,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            idBuf,
            nameBuf,
            nullptr);
        log(DEBUG, "createFlight: Starting new flight for airport %s (%s)", idBuf, nameBuf);
        return idBuf;
    }
    else
    {
        log(DEBUG, "createFlight: Starting new flight for unknown airport");
        return "";
    }
}

std::string getString(const XPLMDataRef ref)
{
    int bytes = XPLMGetDatab(ref, nullptr, 0, 0);
    char buffer[bytes + 1];
    XPLMGetDatab(ref, buffer, 0, bytes);
    buffer[bytes] = '\0';
    return string(buffer);
}

void BlackBoxPlugin::createFlight()
{
    float latitude = XPLMGetDataf(m_latitudeDataRef);
    float longitude = XPLMGetDataf(m_longitudeDataRef);
    string id = findNearestAirport(latitude, longitude);

    m_currentFlight.origin = id;
    m_currentFlight.icaoType = getString(m_aircraftICAODataRef);
    m_currentFlight.flightId = getString(m_flightIDDataRef);
    m_currentFlight.startTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    m_datastore.createFlight(m_currentFlight);
}

void BlackBoxPlugin::updateFlight()
{
    string icaoType = getString(m_aircraftICAODataRef);
    string flightId = getString(m_flightIDDataRef);

    bool update = false;
    if (m_currentFlight.icaoType != icaoType)
    {
        log(WARN, "updateFlight: Aircraft type has changed! %s -> %s", m_currentFlight.icaoType.c_str(), icaoType.c_str());
        m_currentFlight.icaoType = icaoType;
        update = true;
    }
    if (m_currentFlight.flightId != flightId)
    {
        log(DEBUG, "updateFlight: flightId has changed! %s -> %s", m_currentFlight.flightId.c_str(), flightId.c_str());
        update = true;
    }
    if (update)
    {
        m_currentFlight.icaoType = icaoType;
        m_currentFlight.flightId = flightId;
        m_datastore.updateFlight(m_currentFlight);
    }
}

void BlackBoxPlugin::updatePosition()
{
    m_state.position.latitude = XPLMGetDataf(m_latitudeDataRef);
    m_state.position.longitude = XPLMGetDataf(m_longitudeDataRef);
    m_state.position.altitude = XPLMGetDataf(m_elevationDataRef) * METRES_TO_FEET;
}

float BlackBoxPlugin::update(float elapsedMe, float elapsedSim, int counter)
{
    bool paused = XPLMGetDatai(m_pausedDataRef);
    if (paused != m_state.paused)
    {
        m_state.paused = paused;
        if (m_state.paused)
        {
            log(INFO, "Paused!");
        }
        else
        {
            log(INFO, "Unpaused!");
        }
    }

    bool replay = XPLMGetDatai(m_replayDataRef);
    if (replay != m_state.replay)
    {
        m_state.replay = replay;
        if (m_state.replay)
        {
            log(INFO, "Replay Started!");
        }
        else
        {
            log(INFO, "Replay Finished!");
        }
    }

    if (m_state.paused || m_state.replay)
    {
        // Don't do anything while pause/replaying
        return -1;
    }

    if (m_currentFlight.id == 0)
    {
        log(DEBUG, "update: No active flight");
        return -1;
    }

    // Only get the data we need to decide if we need to send an update
    bool parkingBrake = XPLMGetDataf(m_parkingBrakeDataRef) > FLT_EPSILON;
    bool anyOnGround = XPLMGetDatai(m_onGroundAnyDataRef);
    bool allOnGround = XPLMGetDatai(m_onGroundAllDataRef);
    float groundSpeed = XPLMGetDataf(m_groundSpeedDataRef) * MS_TO_KNOTS;
    float fpm = XPLMGetDataf(m_verticalFPMDataRef);
    float gForce = XPLMGetDataf(m_gForceDataRef);
    float pitch = XPLMGetDataf(m_pitchRateDataRef);
    float agl = XPLMGetDataf(m_aglDataRef) * METRES_TO_FEET;

    m_fpm.add(fpm, elapsedSim);

    m_state.groundSpeed = groundSpeed;
    m_state.agl = agl;
    m_state.pitch = pitch;
    m_state.fpm = fpm;
    m_state.fpmAverage = m_fpm.average();

    if (m_state.flightPhase == FlightPhase::INIT)
    {
        // Work out where we started...
        if (allOnGround)
        {
            setMessage("Starting in PARKED phase");
            m_state.flightPhase = FlightPhase::PARKED;
        }
        else
        {
            setMessage("Starting in FLIGHT phase");
            m_state.flightPhase = FlightPhase::FLIGHT;
        }

        updatePosition();
        sendEvent(elapsedSim);

        return -1;
    }

    bool changes = false;
    bool parkingBrakeChanged = false;
    if (parkingBrake != m_state.parkingBrake)
    {
        parkingBrakeChanged = true;
        changes = true;
    }

    bool anyOnGroundChanged = false;
    if (anyOnGround != m_state.anyOnGround)
    {
        anyOnGroundChanged = true;
        changes = true;
    }

    bool allOnGroundChanged = false;
    if (allOnGround != m_state.allOnGround)
    {
        allOnGroundChanged = true;
        changes = true;
    }

    m_state.parkingBrake = parkingBrake;
    m_state.anyOnGround = anyOnGround;
    m_state.allOnGround = allOnGround;

    bool descending = m_state.fpmAverage < -100.0f;
    bool climbing = m_state.fpmAverage > 500.0f;
    bool updatedPosition = false;

    switch (m_state.flightPhase)
    {
        case FlightPhase::PARKED:
            if (!m_state.parkingBrake)
            {
                setMessage("PARKED: Parking brake released, taxiing");
                m_state.flightPhase = FlightPhase::TAXI;
                changes = true;
            }
            else if (groundSpeed > 1.0f)
            {
                // The X-Plane AI likes to brake with the parking brake, so don't log this!
                //setMessage("PARKED: We're moving when parked?", groundSpeed);
            }
            if (!allOnGround)
            {
                setMessage("PARKED: Wheels aren't all on ground??");
            }
            break;

        case FlightPhase::TAXI:
            if (parkingBrakeChanged && m_state.parkingBrake)
            {
                setMessage("TAXI: Parking brake set, parked");
                m_state.flightPhase = FlightPhase::PARKED;
                changes = true;
            }
            else if (groundSpeed > 40.0 || (groundSpeed > 20 && !allOnGround))
            {
                setMessage("TAXI: Looks like we're taking off!");
                m_state.flightPhase = FlightPhase::TAKE_OFF;
                changes = true;
            }
            break;

        case FlightPhase::TAKE_OFF:
            if (allOnGroundChanged && !allOnGround)
            {
                setMessage("TAKE_OFF: Rotating");
            }
            else if (!anyOnGround)
            {
                setMessage("TAKE_OFF: We're up!");
                m_state.flightPhase = FlightPhase::FLIGHT;
                m_state.eventType = EventType::TAKE_OFF;
                changes = true;
            }
            break;

        case FlightPhase::FLIGHT:
            if (descending && agl < 1000.0f)
            {
                setMessage("DESCENT: Approaching ground");
                m_state.flightPhase = FlightPhase::APPROACH;
                changes = true;
                // TODO: Are we stable??
            }
            break;

        case FlightPhase::APPROACH:
            if (anyOnGroundChanged && anyOnGround)
            {
                // TODO: This should be asynchronous
                updatePosition();
                updatedPosition = true;

                string airport = findNearestAirport(m_state.position.latitude, m_state.position.longitude);
                m_currentFlight.destination = airport;
                m_datastore.updateFlight(m_currentFlight);

                setMessage(
                    "APPROACH: Landing Started: airport=%s, FPM=%0.2f (average=%0.2f), G-Force=%0.2f, pitch=%0.2f",
                    airport.c_str(),
                    fpm,
                    m_fpm.average(),
                    gForce,
                    pitch);
                m_state.flightPhase = FlightPhase::LANDING;
                m_state.eventType = EventType::LANDING;

                changes = true;
            }
            else if (climbing && agl > 1000.0f)
            {
                setMessage("APPROACH: Go around??");
                m_state.flightPhase = FlightPhase::FLIGHT;
                changes = true;
            }
            break;

        case FlightPhase::LANDING:
            if (anyOnGroundChanged && !anyOnGround)
            {
                setMessage("LANDING: Bounce!");
            }
            else if (!anyOnGround && agl > 100.0f)
            {
                setMessage("LANDING: Go around?");
                m_state.flightPhase = FlightPhase::FLIGHT;
                changes = true;
            }
            else if (allOnGroundChanged && allOnGround)
            {
                setMessage("LANDING: Landing finished? FPM=%0.2f, G-Force=%0.2f", fpm, gForce);
            }
            if (m_state.fpmAverage < 10 && allOnGround && groundSpeed < 40)
            {
                setMessage("LANDING: Slowed down to taxiing");
                m_state.flightPhase = FlightPhase::TAXI;
                changes = true;
            }
            break;

        default:
            setMessage("Warning: Unhandled flight phase: %d", m_state.flightPhase);
            break;
    }

    float diff = elapsedSim - m_lastSendTime;
    bool send = (changes || diff > 5.0f);
    if (!send && diff > 1.0f)
    {
        if (!updatedPosition)
        {
            updatePosition();
            updatedPosition = true;
        }
        float d = distance(m_state.position, m_lastPosition);
        if (d > 0.1f)
        {
            send = true;
        }
    }
    if (send)
    {
        if (!updatedPosition)
        {
            updatePosition();
        }
        m_state.indicatedAirSpeed = XPLMGetDataf(m_iasDataRef);
        m_state.yaw = XPLMGetDataf(m_yawRateDataRef);
        m_state.roll = XPLMGetDataf(m_rollRateDataRef);

        sendEvent(elapsedSim);
        m_lastPosition = m_state.position;
        m_state.eventType = EventType::NONE;
    }

    return -1;
}

void BlackBoxPlugin::setMessage(const char* message, ...)
{
    va_list args;
    va_start(args, message);

    char buf[1024];
    vsnprintf(buf, 1024, message, args);
    m_message = string(buf);
    log(INFO, "%s", m_message.c_str());
    va_end(args);
}

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
    strcpy(outName, "BlackBox Flight Recorder");
    strcpy(outSig, "com.geekprojects.blackbox.recorder");
    strcpy(outDesc, "BlackBox Flight Recorder");

    return g_bbPlugin.start();
}

PLUGIN_API void XPluginStop()
{
    g_bbPlugin.stop();
}

PLUGIN_API int XPluginEnable()
{
    return g_bbPlugin.enable();
}

PLUGIN_API void XPluginDisable()
{
    g_bbPlugin.disable();
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
    g_bbPlugin.receiveMessage(inFrom, inMsg, inParam);
}

