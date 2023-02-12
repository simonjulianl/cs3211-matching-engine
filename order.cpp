#include "order.hpp"

Order::Order(uint32_t prc, uint32_t cnt, uint32_t id) : price{prc}, count{cnt}, order_id{id}
{
  this->timestamp = getCurrentTimestamp();
}

std::ostream& operator<<(std::ostream& os, const Order& o)
{
      os << "Order"
      << "  id: " << o.order_id 
      << "  price: " << o.price
      << "  quantity: " << o.count
      << "  timestamp: " << o.timestamp
      << std::endl;
      return os;
}