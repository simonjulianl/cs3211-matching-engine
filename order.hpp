#ifndef ORDER_HPP
#define ORDER_HPP

#include <chrono>
#include <cstdint>

struct Order
{
    uint32_t price;
    intmax_t timestamp;
    mutable uint32_t count;
    uint32_t order_id;
    mutable uint32_t execution_id = 1;

    Order(uint32_t price, uint32_t count, uint32_t order_id);
};

std::ostream& operator<<(std::ostream& os, const Order& o);

auto sell_cmp = [](const Order& a, const Order& b)
{
  if (a.price == b.price)
    return a.timestamp < b.timestamp;
  return a.price < b.price;
};

auto buy_cmp = [](const Order& a, const Order& b)
{
  if (a.price == b.price)
    return a.timestamp < b.timestamp;
  return a.price > b.price;
};

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
#endif