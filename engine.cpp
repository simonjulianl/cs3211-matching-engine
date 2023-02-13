#include <iostream>
#include <thread>

#include "engine.hpp"

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

#ifdef DEBUG
void Engine::order_book_stat(const char* symbol) 
{

	SyncCerr {} << std::endl;
	SyncCerr {} << "DEBUG: " << symbol << " ORDER BOOK STATUS" << std::endl;
	SyncCerr {} << "BUY: " << std::endl;
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

void Engine::buy(uint32_t id, const char* symbol, uint32_t price, uint32_t count)
{
	bool order_fulfilled = false;
	
	auto best_order = sell_order_books[symbol].begin();
	auto last_fulfilled_order = sell_order_books[symbol].end();
	
	// match order
	for (; best_order != sell_order_books[symbol].end(); best_order++) {

		if (best_order->price > price)
			break;

		// SyncCerr {} << "best_order: " << *best_order;
		Output::OrderExecuted(best_order->order_id, id, best_order->execution_id, best_order->price, std::min(best_order->count, count), getCurrentTimestamp());
		if (count < best_order->count) {
			best_order->count -= count;
			(best_order->execution_id) += 1;
			count = 0;
		} else {
			count -= best_order->count;
			cancelable.erase(best_order->order_id);
			last_fulfilled_order = best_order;
		}

		if (count == 0) {
			order_fulfilled = true;
			break;
		}
	}

	// insert the unfulfilled order to buy order book
	if (!order_fulfilled) {
		auto ts = getCurrentTimestamp();
		buy_order_books[symbol].insert(Order(price, ts, count, id));
		cancelable[id] = {symbol, input_buy};
		Output::OrderAdded(id, symbol, price, count, false, ts);
	}
			
	// cleanup fulfilled orders from sell order book
	if (last_fulfilled_order != sell_order_books[symbol].end())	
		sell_order_books[symbol].erase(sell_order_books[symbol].begin(), ++last_fulfilled_order);

#ifdef DEBUG
	order_book_stat(symbol);
#endif
}

void Engine::sell(uint32_t id, const char* symbol, uint32_t price, uint32_t count)
{
	bool order_fulfilled = false;
	
	auto best_order = buy_order_books[symbol].begin();
	auto last_fulfilled_order = buy_order_books[symbol].end();
	
	// match order
	for (; best_order != buy_order_books[symbol].end(); best_order++) {

		if (best_order->price < price)
			break;

		// SyncCerr {} << "best_order: " << best_order;
		Output::OrderExecuted(best_order->order_id, id, best_order->execution_id, best_order->price, std::min(best_order->count, count), getCurrentTimestamp());
		if (count < best_order->count) {
			best_order->count -= count;
			(best_order->execution_id) += 1;
			count = 0;
		} else {
			count -= best_order->count;
			cancelable.erase(best_order->order_id);
			last_fulfilled_order = best_order;
		}

		if (count == 0) {
			order_fulfilled = true;
			break;
		}
	}

	// insert the unfulfilled order to sell order book
	if (!order_fulfilled) {
		auto ts = getCurrentTimestamp();
		sell_order_books[symbol].insert(Order(price, ts, count, id));
		cancelable[id] = {symbol, input_sell};
		Output::OrderAdded(id, symbol, price, count, true, ts);
	}
			
	// cleanup fulfilled orders from buy order book
	if (last_fulfilled_order != buy_order_books[symbol].end())
		buy_order_books[symbol].erase(buy_order_books[symbol].begin(), ++last_fulfilled_order);

#ifdef DEBUG
	order_book_stat(symbol);
#endif
}

void Engine::cancel(uint32_t id)
{
	if (cancelable.find(id) == cancelable.end()) {
		Output::OrderDeleted(id, false, getCurrentTimestamp());
		return;
	}

	bool accept_req = false;
	intmax_t ts = 0;
	auto [symbol, type] = cancelable[id];
	if (type == input_buy) {
		auto &order_book = buy_order_books[symbol];
		for (auto o = order_book.begin(); o != order_book.end(); o++) {
			if (o->order_id == id) {
				ts = getCurrentTimestamp();
				order_book.erase(o);
				accept_req = true;
				break;
			}
		}
	} else {
		auto &order_book = sell_order_books[symbol];
		for (auto o = order_book.begin(); o != order_book.end(); o++) {
			if (o->order_id == id) {
				ts = getCurrentTimestamp();
				order_book.erase(o);
				accept_req = true;
				break;
			}
		}
	}
#ifdef DEBUG
	order_book_stat(symbol.c_str());
#endif
	Output::OrderDeleted(id, accept_req, ts);
}

void Engine::connection_thread(ClientConnection connection)
{
	while(true)
	{
		ClientCommand input {};
		switch(connection.readInput(input))
		{
			case ReadResult::Error: SyncCerr {} << "Error reading input" << std::endl;
			case ReadResult::EndOfFile: return;
			case ReadResult::Success: break;
		}

		// Functions for printing output actions in the prescribed format are
		// provided in the Output class:
		switch(input.type)
		{
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
				SyncCerr {}
				    << "Got order: " << static_cast<char>(input.type) << " " << input.instrument << " x " << input.count << " @ "
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
