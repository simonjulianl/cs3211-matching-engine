#include <iostream>
#include <thread>

#include "engine.hpp"

void Engine::accept(ClientConnection connection) {
    auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
    thread.detach();
}

#ifdef DEBUG
void Engine::order_book_stat(const char* symbol)
{
    SyncCerr {}
        << std::endl
        << "DEBUG: " << symbol
        << " ORDER BOOK STATUS" << std::endl;
   
    SyncCerr {}  << "BUY: " << std::endl;

    for (const auto &o: buy_order_books[symbol]) {
        SyncCerr {} << "  " << o;
    }

    SyncCerr {} << "SELL: " << std::endl;

    for (const auto &o: sell_order_books[symbol]) {
        SyncCerr {} << "  " << o;
    }

    SyncCerr {} << std::endl;
}
#endif

bool Engine::is_matching(const uint32_t &buy_price, const uint32_t &sell_price) {
    return buy_price >= sell_price;
}

void Engine::buy(uint32_t id, const char *symbol, uint32_t price, uint32_t count) {
    bool is_order_fulfilled = false;

    auto &s = mutexes.getOrDefault(symbol);
    s.buy_lightswitch.lock(s.shared_m);

    auto &order_book = sell_order_books.getOrDefault(symbol);
    auto end_orderbook = order_book.end();

    // match order
    for (auto current_order = order_book.begin();
         !is_order_fulfilled &&
         current_order != end_orderbook &&
         is_matching(price,
                     (*current_order)->price);
         current_order = order_book.next(current_order)) {
        is_order_fulfilled = process_matching_order(id, current_order, count);
    }

    // insert the unfulfilled order to buy order book
    if (!is_order_fulfilled) {
        auto ts = getCurrentTimestamp();
        std::shared_ptr<Order> new_order = std::make_shared<Order>(price, ts, count, id);
        insert_buy_order(symbol, new_order);
    }

    s.buy_lightswitch.unlock_delete<SingleSellOrderBook>(s.shared_m, order_book);

#ifdef DEBUG
    order_book_stat(symbol);
#endif
}

void Engine::insert_buy_order(const char *symbol, std::shared_ptr<Order> new_order) {
    buy_order_books.getOrDefault(symbol).insert(new_order);
    const uint32_t id = new_order->order_id;
    cancelable.put({id, {symbol, input_buy}});

    Output::OrderAdded(
            id,
            symbol,
            new_order->price,
            new_order->count,
            false,
            new_order->timestamp
    );
}

void Engine::insert_sell_order(const char *symbol, std::shared_ptr<Order> new_order) {
    sell_order_books.getOrDefault(symbol).insert(new_order);
    const uint32_t id = new_order->order_id;
    cancelable.put({id, {symbol, input_sell}});

    Output::OrderAdded(
            id,
            symbol,
            new_order->price,
            new_order->count,
            true,
            new_order->timestamp
    );
}

void Engine::sell(uint32_t id, const char *symbol, uint32_t price, uint32_t count) {
    bool is_order_fulfilled = false;

    auto &s = mutexes.getOrDefault(symbol);
    s.sell_lightswitch.lock(s.shared_m);

    auto &order_book = buy_order_books.getOrDefault(symbol);
    auto end_orderbook = order_book.end();

    // match order
    for (auto current_order = order_book.begin();
         !is_order_fulfilled &&
         current_order != end_orderbook &&
         is_matching((*current_order)->price, price);
         current_order = order_book.next(current_order)) {
        is_order_fulfilled = process_matching_order(id, current_order, count);
    }

    // insert the unfulfilled order to buy order book
    if (!is_order_fulfilled) {
        auto ts = getCurrentTimestamp();
        std::shared_ptr<Order> new_order = std::make_shared<Order>(price, ts, count, id);
        insert_sell_order(symbol, new_order);
    }

    s.sell_lightswitch.unlock_delete<SingleBuyOrderBook>(s.shared_m, order_book);

#ifdef DEBUG
    order_book_stat(symbol);
#endif
}

bool Engine::process_matching_order(uint32_t id, OrderBook_iterator current_order, uint32_t &count) {
    std::lock_guard<std::mutex> lock((*current_order)->order_mutex);

    Output::OrderExecuted(
            (*current_order)->order_id,
            id,
            (*current_order)->execution_id,
            (*current_order)->price,
            std::min((*current_order)->count, count),
            getCurrentTimestamp()
    );

    if (count < (*current_order)->count) { // the order is fulfilled
        (*current_order)->count -= count;
        ((*current_order)->execution_id) += 1;
        count = 0;
        return true;
    } else {
        count -= (*current_order)->count;
        (*current_order)->count = 0;
        cancelable.erase((*current_order)->order_id);
        return count == 0;
    }
}

template<class T, class J> void Engine::remove_order(SafeMap<T, J> &t, std::string symbol, uint32_t id) {
    bool is_req_accept = false;

    intmax_t ts = 0;
    auto &order_book = t.getOrDefault(symbol);
    for (auto o = order_book.begin(); o != order_book.end(); order_book.next(o)) {
        std::lock_guard<std::mutex> guard((*o)->order_mutex);

        if ((*o)->order_id == id) {
            ts = getCurrentTimestamp();
            order_book.erase(o);
            is_req_accept = true;
            break;
        }
    }
    Output::OrderDeleted(
            id,
            is_req_accept,
            ts
    );
}

void Engine::cancel(uint32_t id) {
    if (!cancelable.contains(id)) {
        Output::OrderDeleted(
                id,
                false,
                getCurrentTimestamp()
        );
        return;
    }

    auto [symbol, type] = cancelable.getOrDefault(id);
    auto &s = mutexes.getOrDefault(symbol);
    s.cancel_lightswitch.lock(s.shared_m);

    if (type == input_buy) {
        remove_order(buy_order_books, symbol, id);
    } else {
        remove_order(sell_order_books, symbol, id);
    }

    s.cancel_lightswitch.unlock(s.shared_m);
#ifdef DEBUG
    order_book_stat(symbol.c_str());
#endif

}

void Engine::connection_thread(ClientConnection connection) {
    while (true) {
        ClientCommand input{};
        switch (connection.readInput(input)) {
            case ReadResult::Error:
                SyncCerr{} << "Error reading input" << std::endl;
            case ReadResult::EndOfFile:
                return;
            case ReadResult::Success:
                break;
        }

        // Functions for printing output actions in the prescribed format are
        // provided in the Output class:
        switch (input.type) {
            case input_cancel: {
                cancel(input.order_id);
                break;
            }

            case input_buy: {
                buy(input.order_id, input.instrument, input.price, input.count);
                break;
            }

            case input_sell: {
                sell(input.order_id, input.instrument, input.price, input.count);
                break;
            }

            default: {
                SyncCerr{}
                        << "Got order: " << static_cast<char>(input.type) << " " << input.instrument << " x "
                        << input.count << " @ "
                        << input.price << " ID: " << input.order_id << std::endl;

                // Remember to take timestamp at the appropriate time, or compute
                // an appropriate timestamp!
                auto output_time = getCurrentTimestamp();

                Output::OrderAdded(input.order_id, input.instrument, input.price, input.count, input.type == input_sell,
                                   output_time);
                break;
            }
        }
    }
}
