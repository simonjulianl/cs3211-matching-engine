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

    template<typename T> void unlock_delete(std::mutex &m, T &order_book) {
        std::lock_guard<std::mutex> guard(lightswitch_mutex);
        counter--;
        if (counter == 0) {
            auto current = order_book.begin();
            bool is_erasable = false;

            for (; current != order_book.end() && (*current)->count == 0; current = order_book.next(current)) {
                is_erasable = true;
            }

            if (is_erasable) {
                order_book.erase(order_book.begin(), current);
            }

            m.unlock();
        }
    }

    void unlock(std::mutex &m) {
        std::lock_guard<std::mutex> guard(lightswitch_mutex);
        counter--;
        if (counter == 0) m.unlock();
    }
};

struct LightSwitches {
    LightSwitch cancel_lightswitch;
    LightSwitch buy_lightswitch;
    LightSwitch sell_lightswitch;
    std::mutex shared_m;
};

#endif // _LIGHTSWITCH_H
