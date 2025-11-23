//
// Created by Ian Parker on 18/11/2025.
//

#include "receiver.h"
#include "blackbox/event.h"

#include <zmq.h>

using namespace std;
using namespace BlackBox;

Receiver::Receiver() : Logger("Receiver")
{
}

Receiver::~Receiver()
{
    zmq_close(m_zmqReceiver);
    zmq_ctx_destroy(m_zmqContext);
}

bool Receiver::init()
{
    m_zmqContext = zmq_ctx_new();

    m_zmqReceiver = zmq_socket(m_zmqContext, ZMQ_PULL);
    int res = zmq_bind(m_zmqReceiver, "tcp://*:8866");
    if (res != 0)
    {
        log(ERROR, "Failed to bind receiver socket: %d", res);
        return false;
    }

    if (!m_dataStore.init())
    {
        return false;
    }

    m_flightId = m_dataStore.createFlight("", "");

    return true;
}

void Receiver::receive()
{
    log(INFO, "receive: Waiting for messages");
    uint64_t lastCommit = 0;
    m_dataStore.startTransaction();
    while (true)
    {
        Event event;
        int res;
        res = zmq_recv(m_zmqReceiver, &event, sizeof(Event), 0);
        if (res < sizeof(State))
        {
            log(WARN, "Ignoring %d byte message\n", res);
            continue;
        }

        log(DEBUG, "Position: %ls", event.state.position.toString().c_str());
        m_dataStore.write(m_flightId, event.state.timestamp, event.state);

        uint64_t now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
        if (now - lastCommit > 1000)
        {
            m_dataStore.commitTransaction();
            m_dataStore.startTransaction();
        }
    }
}
