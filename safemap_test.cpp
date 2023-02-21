#include <iostream>
#include <thread>
#include <set>
#include <vector>
#include <cassert>

#include "safemap.hpp"
#include "order.hpp"

#define NUM_ITEMS 100
#define NUM_READERS 10
#define NUM_WRITERS 10


std::vector<std::string> global_symbols;

void int_writer(int id, SafeMap<int, std::set<int>> &m) {
  for (int i = 0; i < NUM_ITEMS; ++i) {
    if (!m.contains(id * NUM_ITEMS + i)) {
      m.put({id * NUM_ITEMS + i, std::set<int>()});
    }
  }
}

void int_reader(SafeMap<int, std::set<int>> &m) {
  for (int i = 0; i < NUM_WRITERS * NUM_ITEMS; ++i) {
    std::cout << "Item " << (i) << " exists? " << (m.contains(i) ? "True " : "False ");
  }
}

void int_check() {
  SafeMap<int, std::set<int>> m;
  std::vector<std::thread> rt(NUM_READERS);
  std::vector<std::thread> wt(NUM_WRITERS);

  std::cout << "running writer threads\n";
  for (int i = 0; i < NUM_WRITERS; ++i) {
    wt[i] = std::thread(int_writer, i, std::ref(m));
  }

  std::cout << "running reader threads\n";
  for (int i = 0; i < NUM_READERS; ++i) {
    rt[i] = std::thread(int_reader, std::ref(m));
  }

  for (auto& t: wt)
    t.join();

  for (auto& t: rt)
    t.join();

  std::cout << std::endl;
  std::cout << " == Final size of SafeMap: == " << m.size() << std::endl;
  std::cout << " == Final content of SafeMap: == " << std::endl;
  int_reader(m);
  
  std::cout << " == Correctness check: == " << std::endl;
  assert(m.size() == NUM_WRITERS * NUM_ITEMS);
  std::cout << "OK" << std::endl;
}

void order_writer(int id, SafeMap<std::string, std::set<std::shared_ptr<Order>, decltype(buy_cmp)>> &m) {
  for (int i = 0; i < NUM_ITEMS; ++i) {
    std::string key = "SYMBOL" + std::to_string(id * NUM_ITEMS + i);
    m.getOrDefault(key, std::set<std::shared_ptr<Order>, decltype(buy_cmp)>());
  }
}

void order_reader(SafeMap<std::string, std::set<std::shared_ptr<Order>, decltype(buy_cmp)>> &m) {
  for (int i = 0; i < NUM_WRITERS * NUM_ITEMS; ++i) {
    std::string key = "SYMBOL" + std::to_string(i);
    std::cout << "Item " << (key) << " exists? " << (m.contains(key) ? "True " : "False ");
  }
}

void order_check() {
  SafeMap<std::string, std::set<std::shared_ptr<Order>, decltype(buy_cmp)>> m;
  std::vector<std::thread> rt(NUM_READERS);
  std::vector<std::thread> wt(NUM_WRITERS);

  std::cout << "running writer threads\n";
  for (int i = 0; i < NUM_WRITERS; ++i) {
    wt[i] = std::thread(order_writer, i, std::ref(m));
  }

  std::cout << "running reader threads\n";
  for (int i = 0; i < NUM_READERS; ++i) {
    rt[i] = std::thread(order_reader, std::ref(m));
  }

  for (auto& t: wt)
    t.join();

  for (auto& t: rt)
    t.join();

  std::cout << std::endl;
  std::cout << " == Final size of SafeMap: == " << m.size() << std::endl;
  std::cout << " == Final content of SafeMap: == " << std::endl;
  order_reader(m);
  
  std::cout << " == Correctness check: == " << std::endl;
  assert(m.size() == NUM_WRITERS * NUM_ITEMS);
  std::cout << "OK" << std::endl;
}

void setup() {
  for (int i = 0; i < NUM_WRITERS * NUM_ITEMS; i++)
    global_symbols.push_back("SYMBOL" + std::to_string(i));
}

int main() {
  setup();
  int_check();
  order_check();
  return 0;
}
