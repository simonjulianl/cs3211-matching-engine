// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <unordered_map>
#include <set>
#include <string>

#include "io.hpp"
#include "order.hpp"

// #define DEBUG

struct Engine
{
public:
	void accept(ClientConnection conn);

private:
	// maps symbol <-> Orders
	std::unordered_map<std::string, std::set<Order, decltype(buy_cmp)>> buy_order_books;
	std::unordered_map<std::string, std::set<Order, decltype(sell_cmp)>> sell_order_books;

	// maps order_id <-> {symbol, (buy/sell)}
	std::unordered_map<uint32_t, std::pair<std::string, char>> cancelable;

	void buy(uint32_t id, const char* symbol, uint32_t price, uint32_t count);
	void sell(uint32_t id, const char* symbol, uint32_t price, uint32_t count);
	void cancel(uint32_t id);
	void connection_thread(ClientConnection conn);
#ifdef DEBUG
	void order_book_stat(const char* symbol);
#endif
};

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

#endif
