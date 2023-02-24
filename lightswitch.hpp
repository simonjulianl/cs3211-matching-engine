#ifndef _LIGHTSWITCH_H
#define _LIGHTSWITCH_H

#include <mutex>
#include "engine.hpp"

struct LightSwitch {
private:
    std::mutex lightswitch_mutex;
    int counter = 0;

public:
    void lock(std::mutex &m) {
        std::lock_guard<std::mutex> guard(lightswitch_mutex);
        counter++;
        if (counter == 1) {
            m.lock();
        }
    }

    void unlock(std::mutex &m) {
        std::lock_guard<std::mutex> guard(lightswitch_mutex);
        counter--;
        if (counter == 0) {
            m.unlock();
        }
    }

};

struct LightSwitches {
    LightSwitch buy_lightswitch;
    LightSwitch sell_lightswitch;
    std::mutex shared_m;
};

#endif // _LIGHTSWITCH_H
