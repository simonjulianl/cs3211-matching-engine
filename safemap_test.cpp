#include <iostream>
#include <thread>
#include <set>
#include <vector>
#include <cassert>

#include "safemap.hpp"
#include "order.hpp"

#define NUM_THREADS 100
#define NUM_ITEMS 100
#define NUM_INSTRUMENTS 20


std::vector<std::string> global_symbols;

void int_fn(int index, SafeMap<int, std::set<int>> &m) {
  sleep(1);
  for (int j = 0; j < NUM_ITEMS; ++j) {
    if (!m.contains(index)) {
      m.put({index, std::set<int>()});
    }
    m.get(index).insert(j);
  }
}

void int_check() {
  SafeMap<int, std::set<int>> m;
  std::vector<std::thread> tv(NUM_THREADS);

  std::cout << "inserting items\n";
  for (int i = 0; i < NUM_THREADS; ++i) {
    tv[i] = std::thread(int_fn, i, std::ref(m));
  }

  std::cout << "running threads\n";
  for (auto& t: tv)
    t.join();

  std::cout << "correctness check\n";
  for (int i = 0; i < NUM_THREADS; ++i) {
    assert(m.get(i).size() == NUM_ITEMS);
    for (int j = 0; j < NUM_ITEMS; ++j) {
      assert(m.get(i).find(j) != m.get(i).end());
    }
  }
  std::cout << "INT OK" << std::endl;
  std::cout << std::endl;
}

void ord_fn(SafeMap<std::string, std::set<Order, decltype(buy_cmp)>> &m) {
  sleep(1);
  std::string symbol = global_symbols[rand() % NUM_INSTRUMENTS];
  for (int j = 0; j < NUM_ITEMS; ++j) {
    if (!m.contains(symbol)) {
      m.put({symbol, std::set<Order, decltype(buy_cmp)>()});
    }
    m.get(symbol).insert(Order(rand(), rand(), rand(), rand()));
  }
}

void order_check() {
  SafeMap<std::string, std::set<Order, decltype(buy_cmp)>> sm;
  std::vector<std::thread> tv(NUM_THREADS);

  std::cout << "inserting items\n";
  for (int i = 0; i < NUM_THREADS; ++i) {
    tv[i] = std::thread(ord_fn, std::ref(sm));
  }

  std::cout << "running threads\n";
  for (auto& t: tv)
    t.join();

  std::cout << "correctness check\n";
  int inserted = 0;
  for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
    inserted += sm.get(global_symbols[i]).size();
  }
  std::cout << "inserted orders: " << inserted << std::endl;
  std::cout << "ORDER OK" << std::endl;
}

int main() {
  for (int i = 0; i < NUM_INSTRUMENTS; i++)
    global_symbols.push_back("SYMBOL" + std::to_string(i));

  int_check();
  order_check();
  return 0;
}
