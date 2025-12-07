//
// Created by Ian Parker on 05/12/2025.
//

#ifndef BLACKBOX_SCREENSHOTS_H
#define BLACKBOX_SCREENSHOTS_H

#include <thread>

#include "blackbox/logger.h"

#include <libfswatch/c/libfswatch.h>

class BlackBoxPlugin;

class ScreenshotWatcher : public BlackBox::Logger
{
    BlackBoxPlugin* m_plugin;
    FSW_HANDLE m_handle = nullptr;
    std::thread* m_watcherThread = nullptr;

    static void callback(
        fsw_cevent const* events,
        unsigned int event_num,
        void *data);
    void callback(
       fsw_cevent const* events,
       unsigned int event_num);

    void run();

 public:
    ScreenshotWatcher(BlackBoxPlugin* plugin);
    bool init();
    void start();
    void stop();
};


#endif //BLACKBOX_SCREENSHOTS_H
