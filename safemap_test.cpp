#include <iostream>
#include <thread>
#include <set>
#include <vector>
#include <cassert>

#include "safemap.hpp"

#define NUM_THREADS 100
#define NUM_ITEMS 1'000'000

void fill_set(int index, SafeMap<int, std::set<int>> &m) {
  for (int j = 0; j < NUM_ITEMS; ++j) {
    if (!m.contains(index)) {
      m.put({index, std::set<int>()});
    }
    m.get(index).insert(j);
  }
}

int main() {

  SafeMap<int, std::set<int>> m;
  // for (int i = 0; i < NUM_THREADS; ++i) {
  //   m.put({i, std::set<int>()});
  // }

  std::vector<std::thread> tv(NUM_THREADS);

  std::cout << "\n inserting items\n";
  for (int i = 0; i < NUM_THREADS; ++i) {
    tv[i] = std::thread(fill_set, i, ref(m));
  }

  std::cout << "\n joining threads\n";
  for (auto& t: tv)
    t.join();

  std::cout << "\n correctness check\n";
  for (int i = 0; i < NUM_THREADS; ++i) {
    auto s = m.get(i);
    std::cout << "inserted item: " << s.size() << std::endl;
    assert(m.get(i).size() == NUM_ITEMS);
    for (int j = 0; j < NUM_ITEMS; ++j) {
      assert(s.find(j) != s.end());
    }
  }
  std::cout << "OK" << std::endl;

  return 0;
}
