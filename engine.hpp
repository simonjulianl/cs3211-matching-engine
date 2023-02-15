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

typedef std::unordered_map<uint32_t, std::pair<std::string, CommandType>> CancelMap;
typedef std::set<Order, decltype(buy_cmp)> SingleBuyOrderBook;
typedef std::set<Order, decltype(sell_cmp)> SingleSellOrderBook;
typedef std::unordered_map<std::string, SingleBuyOrderBook> MultipleBuyOrderBooks;
typedef std::unordered_map<std::string, SingleSellOrderBook> MultipleSellOrderBooks;
typedef SingleBuyOrderBook::const_iterator OrderBook_iterator; // Iterators for sell and buy are the same

struct Engine {
public:
    void accept(ClientConnection conn);

private:
    // maps symbol <-> Orders
    MultipleBuyOrderBooks buy_order_books;
    MultipleSellOrderBooks sell_order_books;

    // maps order_id <-> {symbol, (buy/sell)}
    CancelMap cancelable;

    void buy(uint32_t id, const char *symbol, uint32_t price, uint32_t count);

    void sell(uint32_t id, const char *symbol, uint32_t price, uint32_t count);

    void cancel(uint32_t id);

    void connection_thread(ClientConnection conn);

    /*
     * Helper functions
     */
    void insert_buy_order(const char *symbol, const Order &new_order);

    static bool is_matching(const uint32_t &active_buy_price, const uint32_t &resting_sell_price);

#ifdef DEBUG
    void order_book_stat(const char* symbol);
#endif

    void insert_sell_order(const char *symbol, const Order &new_order);

    bool process_matching_order(uint32_t id, OrderBook_iterator current_order, uint32_t &count);

    template<typename T> void remove_order(T t, std::string, uint32_t id);
};

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept {
    return 1; // in serial execution, timestamp doesn't matter as the order of printing is already serialized
}

#endif
