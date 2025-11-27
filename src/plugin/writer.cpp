//
// Created by Ian Parker on 18/11/2025.
//

#include "writer.h"
#include "plugin.h"

using namespace std;
using namespace UFC;
using namespace BlackBox;

Writer::Writer(BlackBoxPlugin* plugin) : Logger("Writer"), m_plugin(plugin)
{
}

void Writer::close()
{
    stop();
}

void Writer::start()
{
    if (m_running || m_writerThread != nullptr)
    {
    log(DEBUG, "start: Already started?");
        return;
    }

    m_running = true;
    m_writerThread = new thread(&Writer::main, this);
}

void Writer::stop()
{
    if (!m_running || m_writerThread == nullptr)
    {
        return;
    }

    m_running = false;

    log(DEBUG, "stop: Signalling to the writer thread...");
    {
        scoped_lock lock(m_mutex);
        m_queueSignal.notify_one();
    }

    log(DEBUG, "stop: Waiting for writer thread to finish...");
    m_writerThread->join();
    m_writerThread = nullptr;
}

void Writer::write(const Event &event)
{
    scoped_lock lock(m_mutex);
    m_queue.push_back(event);
    m_queueSignal.notify_one();
}

void Writer::main()
{
    while (m_running)
    {
        vector<Event> events;

        {
            unique_lock lock(m_mutex);
            m_queueSignal.wait(lock);
            events = m_queue;
            m_queue.clear();
        }

        m_plugin->getDataStore().startTransaction();
        for (const Event& event : events)
        {
            m_plugin->getDataStore().writeState(event.flightId, event.state);
        }
        m_plugin->updateFlight();
        m_plugin->getDataStore().commitTransaction();
    }
}
