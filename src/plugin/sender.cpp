//
// Created by Ian Parker on 18/11/2025.
//

#include "sender.h"

#include <zmq.h>

using namespace std;
using namespace UFC;
using namespace BlackBox;

Sender::Sender() : Logger("Sender")
{
}

bool Sender::init()
{
    m_zmqContext = zmq_ctx_new();
    log(DEBUG, "init: context=%p", m_zmqContext);

    m_zmqSender = zmq_socket(m_zmqContext, ZMQ_PUSH);
    log(DEBUG, "init: sender=%p", m_zmqSender);

    int res;
    res = zmq_connect(m_zmqSender, "tcp://localhost:8866");
    log(DEBUG, "init: connect=%d", res);

    return true;
}

void Sender::send(const Event &event)
{
    int res;
    log(DEBUG, "send: Attempting to send state, size=%d", sizeof(Event));
    res = zmq_send(m_zmqSender, &event, sizeof(Event), 0);
    log(DEBUG, "send: res=%d", res);
}
