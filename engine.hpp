// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <unordered_map>
#include <set>
#include <string>

#include "io.hpp"
#include "order.hpp"
#include "safemap.hpp"
#include "safeset.hpp"
#include "lightswitch.hpp"

// #define DEBUG
typedef SafeMap<uint32_t, std::shared_ptr<Order>> CancelMap;
typedef SafeMap<std::string, LightSwitches> MutexMap;
typedef SafeSet<std::shared_ptr<Order>, buy_cmp> SingleBuyOrderBook;
typedef SafeSet<std::shared_ptr<Order>, sell_cmp> SingleSellOrderBook;
typedef SafeMap<std::string, SingleBuyOrderBook> MultipleBuyOrderBooks;
typedef SafeMap<std::string, SingleSellOrderBook> MultipleSellOrderBooks;
typedef std::set<std::shared_ptr<Order>, buy_cmp>::const_iterator OrderBook_iterator; // Iterators for sell and buy are the same

struct Engine {
public:
    void accept(ClientConnection conn);

private:
    // maps symbol <-> Orders
    MultipleBuyOrderBooks buy_order_books;
    MultipleSellOrderBooks sell_order_books;

    // maps order_id <-> {symbol, (buy/sell)}
    CancelMap cancelable;

    // map symbol <-> set of mutexes
    MutexMap mutexes;

    void buy(uint32_t id, const char *symbol, uint32_t price, uint32_t count);

    void sell(uint32_t id, const char *symbol, uint32_t price, uint32_t count);

    void cancel(uint32_t id);

    void connection_thread(ClientConnection conn);

    /*
     * Helper functions
     */
    void insert_buy_order(const char *symbol, std::shared_ptr<Order> new_order);

    static bool is_matching(const uint32_t &active_buy_price, const uint32_t &resting_sell_price);

#ifdef DEBUG
    void order_book_stat(const char* symbol);
#endif

    void insert_sell_order(const char *symbol, std::shared_ptr<Order> new_order);

    bool process_matching_order(uint32_t id, OrderBook_iterator current_order, uint32_t &count);
};

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

#endif
