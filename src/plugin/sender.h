//
// Created by Ian Parker on 18/11/2025.
//

#ifndef BLACKBOX_SENDER_H
#define BLACKBOX_SENDER_H

#include "blackbox/event.h"
#include "blackbox/logger.h"

class Sender : BlackBox::Logger
{
    void* m_zmqContext = nullptr;
    void* m_zmqSender = nullptr;

 public:
    Sender();
    bool init();
    void send(const Event& event);
};


#endif //BLACKBOX_SENDER_H
