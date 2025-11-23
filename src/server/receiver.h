//
// Created by Ian Parker on 18/11/2025.
//

#ifndef BLACKBOX_RECEIVER_H
#define BLACKBOX_RECEIVER_H

#include "blackbox/datastore.h"

class Receiver : BlackBox::Logger
{
    void* m_zmqContext = nullptr;
    void* m_zmqReceiver = nullptr;

    DataStore m_dataStore;
    uint64_t m_flightId = 0;

 public:
    Receiver();
    ~Receiver();

    bool init();

    void receive();
};


#endif //BLACKBOX_RECEIVER_H
