#ifndef ORDER_HPP
#define ORDER_HPP

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <ostream>

class Order {
public:
    uint32_t price;
    intmax_t timestamp;
    mutable uint32_t count;
    uint32_t order_id;
    mutable uint32_t execution_id = 1;
    std::mutex order_mutex;

    Order(uint32_t price, intmax_t timestamp, uint32_t count, uint32_t order_id);
    std::pair<bool, bool> fulfill(uint32_t& active_count);
};

std::ostream &operator<<(std::ostream &os, const Order &o);

const auto sell_cmp = [](const std::shared_ptr<Order> a, const std::shared_ptr<Order> b) {
    if (a->price == b->price)
        return a->timestamp < b->timestamp;
    return a->price < b->price;
};

const auto buy_cmp = [](const std::shared_ptr<Order> a, const std::shared_ptr<Order> b) {
    if (a->price == b->price)
        return a->timestamp < b->timestamp;
    return a->price > b->price;
};

#endif