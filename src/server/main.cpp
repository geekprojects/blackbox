//
// Created by Ian Parker on 17/11/2025.
//

#include <cstdio>
#include <deque>

#include <unistd.h>

#include "../../include/blackbox/datastore.h"
#include "receiver.h"
#include "blackbox/event.h"

int main(int argc, char** argv)
{


    Receiver receiver;
    receiver.init();
    receiver.receive();
}

