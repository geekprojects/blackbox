//
// Created by Ian Parker on 18/11/2025.
//

#ifndef BLACKBOX_STATUSWINDOW_H
#define BLACKBOX_STATUSWINDOW_H

#include "plugin.h"

#include <XPLMDisplay.h>

class StatusWindow
{
    BlackBoxPlugin* m_plugin;

    XPLMWindowID m_window = nullptr;

    static void drawCallback(XPLMWindowID in_window_id, void * in_refcon)
    {
        static_cast<StatusWindow*>(in_refcon)->draw(in_window_id);
    }
    void draw(XPLMWindowID in_window_id) const;

 public:
    explicit StatusWindow(BlackBoxPlugin* plugin);
    ~StatusWindow();

    bool open();

};


#endif //BLACKBOX_STATUSWINDOW_H
