//
// Created by Ian Parker on 18/11/2025.
//

#ifndef BLACKBOX_SENDER_H
#define BLACKBOX_SENDER_H

#include <thread>

#include "blackbox/logger.h"
#include "blackbox/datastore.h"

class BlackBoxPlugin;

struct Event
{
    uint64_t flightId;
    State state;
};

class Writer : BlackBox::Logger
{
    BlackBoxPlugin* m_plugin = nullptr;

    std::thread* m_writerThread = nullptr;
    std::vector<Event> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_queueSignal;

    uint64_t m_lastStatusId = 0;

    bool m_running = false;

    void main();

 public:
    explicit Writer(BlackBoxPlugin* plugin);

    void close();

    void start();
    void stop();

    void write(const Event& event);

    [[nodiscard]] uint64_t getLastStatusId() const { return m_lastStatusId; }
};

#endif //BLACKBOX_SENDER_H
