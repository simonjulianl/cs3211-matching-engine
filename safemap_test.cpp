#include <iostream>
#include <thread>
#include <set>
#include <vector>
#include <cassert>

#include "safemap.hpp"
#include "order.hpp"

#define NUM_ITERATIONS 1'000
#define NUM_THREADS 10'000
#define NUM_INSTRUMENTS 20


std::vector<std::string> global_symbols;

void int_fn(int index, SafeMap<int, std::set<int>> &m) {
  index %= NUM_INSTRUMENTS;
  for (int j = 0; j < NUM_ITERATIONS; ++j) {
    if (!m.contains(index)) {
      m.put({index, std::set<int>()});
    }
    m.get(index);
  }
}

void int_check() {
  SafeMap<int, std::set<int>> m;
  std::vector<std::thread> tv(NUM_THREADS);

  std::cout << "running threads\n";
  for (int i = 0; i < NUM_THREADS; ++i) {
    tv[i] = std::thread(int_fn, i, std::ref(m));
  }

  for (auto& t: tv)
    t.join();
}

void ord_fn(int index, SafeMap<std::string, std::set<Order, decltype(buy_cmp)>> &m) {
  std::string symbol = global_symbols[index % NUM_INSTRUMENTS];
  for (int j = 0; j < NUM_ITERATIONS; ++j) {
    if (!m.contains(symbol)) {
      m.put({symbol, std::set<Order, decltype(buy_cmp)>()});
    }
    m.get(symbol);
  }
}

void order_check() {
  SafeMap<std::string, std::set<Order, decltype(buy_cmp)>> sm;
  std::vector<std::thread> tv(NUM_THREADS);

  std::cout << "running threads\n";
  for (int i = 0; i < NUM_THREADS; ++i) {
    tv[i] = std::thread(ord_fn, i, std::ref(sm));
  }

  for (auto& t: tv)
    t.join();
}

int main() {
  for (int i = 0; i < NUM_INSTRUMENTS; i++)
    global_symbols.push_back("SYMBOL" + std::to_string(i));

  int_check();
  order_check();
  return 0;
}
