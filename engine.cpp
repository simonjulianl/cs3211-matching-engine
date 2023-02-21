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
    bool is_order_fulfilled = false, is_matching_successful = false;


    auto last_fulfilled_order = sell_order_books[symbol].end();
    auto end_orderbook = sell_order_books[symbol].end();
    auto current_order = sell_order_books[symbol].begin();

    // match order
    for (; !is_order_fulfilled &&
           current_order != end_orderbook &&
           is_matching(price, (*current_order)->price); current_order++) {
        is_matching_successful = true;
        is_order_fulfilled = process_matching_order(id, current_order, count);
        if ((*current_order)->count == 0)
            last_fulfilled_order = current_order;
    }

    // cleanup fulfilled orders
    if (is_matching_successful) {
        sell_order_books[symbol].erase(sell_order_books[symbol].begin(), ++last_fulfilled_order);
    }

    // insert the unfulfilled order to buy order book
    if (!is_order_fulfilled) {
        auto ts = getCurrentTimestamp();
        std::shared_ptr<Order> new_order = std::make_shared<Order>(price, ts, count, id);
        insert_buy_order(symbol, new_order);
    }

#ifdef DEBUG
    order_book_stat(symbol);
#endif
}

void Engine::insert_buy_order(const char* symbol, std::shared_ptr<Order> new_order) {
    buy_order_books[symbol].insert(new_order);
    const uint32_t id = new_order->order_id;
    cancelable[id] = {symbol, input_buy};

    Output::OrderAdded(
            id,
            symbol,
            new_order->price,
            new_order->count,
            false,
            new_order->timestamp
    );
}

void Engine::insert_sell_order(const char* symbol, std::shared_ptr<Order> new_order) {
    sell_order_books[symbol].insert(new_order);
    const uint32_t id = new_order->order_id;
    cancelable[id] = {symbol, input_sell};

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
    bool is_order_fulfilled = false, is_matching_successful = false;

    auto last_fulfilled_order = buy_order_books[symbol].end();
    auto end_orderbook = buy_order_books[symbol].end();
    auto current_order = buy_order_books[symbol].begin();

    // match order
    for (; !is_order_fulfilled &&
           current_order != end_orderbook &&
           is_matching((*current_order)->price, price); current_order++) {
        is_matching_successful = true;
        is_order_fulfilled = process_matching_order(id, current_order, count);
        if ((*current_order)->count == 0)
            last_fulfilled_order = current_order;
    }

    // cleanup fulfilled orders
    if (is_matching_successful) {
        buy_order_books[symbol].erase(buy_order_books[symbol].begin(), ++last_fulfilled_order);
    }

    // insert the unfulfilled order to buy order book
    if (!is_order_fulfilled) {
        auto ts = getCurrentTimestamp();
        std::shared_ptr<Order> new_order = std::make_shared<Order>(price, ts, count, id);
        insert_sell_order(symbol, new_order);
    }

#ifdef DEBUG
    order_book_stat(symbol);
#endif
}

bool Engine::process_matching_order(uint32_t id, OrderBook_iterator current_order, uint32_t &count) {
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

template<typename T> void Engine::remove_order(T &t, std::string symbol, uint32_t id) {
    bool is_req_accept = false;

    intmax_t ts = 0;
    auto &order_book = t[symbol];
    for (auto o = order_book.begin(); o != order_book.end(); o++) {
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
    if (cancelable.find(id) == cancelable.end()) {
        Output::OrderDeleted(
                id,
                false,
                getCurrentTimestamp()
        );
        return;
    }

    auto [symbol, type] = cancelable[id];
    if (type == input_buy) {
        remove_order<MultipleBuyOrderBooks>(buy_order_books, symbol, id);
    } else {
        remove_order<MultipleSellOrderBooks>(sell_order_books, symbol, id);
    }
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
