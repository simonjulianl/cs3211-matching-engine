#ifndef _LIGHTSWITCH_H
#define _LIGHTSWITCH_H

#include <mutex>

struct lightswitch {
    std::mutex lightswitch_mutex;
    int counter = 0;

    void lightswitch_lock(std::mutex& m) {
        std::lock_guard<std::mutex> guard(lightswitch_mutex);
        counter++;
        if (counter == 1) {
            m.lock();
        }
    }

    void lightswitch_unlock(std::mutex& m) {
        std::lock_guard<std::mutex> guard(lightswitch_mutex);
        counter--;
        if (counter == 1) {
            m.unlock();
        }
    }
};


#endif // _LIGHTSWITCH_H
