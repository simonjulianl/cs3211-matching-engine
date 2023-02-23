#include "order.hpp"

Order::Order(uint32_t prc, intmax_t t, uint32_t cnt, uint32_t id) : price{prc}, timestamp{t}, count{cnt}, order_id{id} {
}

std::pair<bool, bool> Order::fulfill(uint32_t &active_count) {
    std::lock_guard<std::mutex> guard(order_mutex);
    if (active_count < this->count) { // the order is fulfilled
        this->count -= active_count;
        this->execution_id += 1;
        active_count = 0;
        return {true, false};
    }

    active_count -= this->count;
    this->count = 0;
    return {active_count == 0, true};
}

std::ostream &operator<<(std::ostream &os, const Order &o) {
    os << "Order"
       << "  id: " << o.order_id
       << "  price: " << o.price
       << "  quantity: " << o.count
       << "  timestamp: " << o.timestamp;
    return os;
}