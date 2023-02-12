#include <iostream>
#include <thread>

#include "io.hpp"
#include "engine.hpp"
#include "order.hpp"

#define DEBUG

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

void Engine::buy(uint32_t id, const char* symbol, uint32_t price, uint32_t count)
{
	// SyncCerr {} << "BUY id: " << id << " symbol: " << symbol << " price: " << price << " count: " << count << std::endl;
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
			last_fulfilled_order = best_order;
		}

		if (count == 0) {
			order_fulfilled = true;
			break;
		}
	}

	// insert the unfulfilled order to buy order book
	if (!order_fulfilled) {
		buy_order_books[symbol].insert(Order(price, count, id));
		Output::OrderAdded(id, symbol, price, count, false, getCurrentTimestamp());
	}
			
	// cleanup fulfilled orders from sell order book
	if (last_fulfilled_order != sell_order_books[symbol].end())	
		sell_order_books[symbol].erase(sell_order_books[symbol].begin(), last_fulfilled_order);

	#ifdef DEBUG
	SyncCerr {} << "DEBUG: ORDER BOOK AFTER BUY" << std::endl;
	for (const auto &o: buy_order_books[symbol]) {
		SyncCerr {} << o;
	}
	#endif
}

void Engine::sell(uint32_t id, const char* symbol, uint32_t price, uint32_t count)
{
	// SyncCerr {} << "SELL id: " << id << " symbol: " << symbol << " price: " << price << " count: " << count << std::endl;
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
			last_fulfilled_order = best_order;
		}

		if (count == 0) {
			order_fulfilled = true;
			break;
		}
	}

	// insert the unfulfilled order to sell order book
	if (!order_fulfilled) {
		sell_order_books[symbol].insert(Order(price, count, id));
		Output::OrderAdded(id, symbol, price, count, true, getCurrentTimestamp());
	}
			
	// cleanup fulfilled orders from buy order book
	if (last_fulfilled_order != buy_order_books[symbol].end())	
		buy_order_books[symbol].erase(buy_order_books[symbol].begin(), last_fulfilled_order);

	#ifdef DEBUG
	SyncCerr {} << "DEBUG: ORDER BOOK AFTER SELL" << std::endl;
	for (const auto &o: sell_order_books[symbol]) {
		SyncCerr {} << o;
	}
	#endif
}

// void Engine::cancel(uint32_t id)
// {

// }

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
				SyncCerr {} << "Got cancel: ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				auto output_time = getCurrentTimestamp();
				Output::OrderDeleted(input.order_id, true, output_time);
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

		// // Additionally:

		// // Remember to take timestamp at the appropriate time, or compute
		// // an appropriate timestamp!
		// intmax_t output_time = getCurrentTimestamp();

		// // Check the parameter names in `io.hpp`.
		// Output::OrderExecuted(123, 124, 1, 2000, 10, output_time);
	}
}
