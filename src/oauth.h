#pragma once

#include <thread>

namespace oauth {

    void serverThread();

    inline void startServer() {
        std::thread(serverThread).detach();
    }

}