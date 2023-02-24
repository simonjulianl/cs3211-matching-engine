#include "order.hpp"

Order::Order(uint32_t prc, intmax_t t, uint32_t cnt, uint32_t id) : price{prc}, timestamp{t}, count{cnt}, order_id{id} {
}

std::ostream &operator<<(std::ostream &os, const Order &o) {
    os << "Order"
       << "  id: " << o.order_id
       << "  price: " << o.price
       << "  quantity: " << o.count
       << "  timestamp: " << o.timestamp;
    return os;
}