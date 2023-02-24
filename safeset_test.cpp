#include <iostream>
#include <thread>
#include <set>
#include <vector>
#include <cassert>

#include "safeset.hpp"
#include "order.hpp"

#define NUM_ITEMS 100
#define NUM_READERS 10
#define NUM_WRITERS 10

void int_writer(int id, SafeSet<int> &s) {
    for (int i = 0; i < NUM_ITEMS; ++i) {
        s.insert(id * NUM_ITEMS + i);
    }
}

void int_reader(SafeSet<int> &s) {
    auto it = s.begin();
    for (; it != s.end(); it = s.next(it)) {
        std::cout << *it << std::endl;
    }
}

void int_check() {
    SafeSet<int> s;
    std::vector<std::thread> rt(NUM_READERS);
    std::vector<std::thread> wt(NUM_WRITERS);

    std::cout << "running writer threads\n";
    for (int i = 0; i < NUM_WRITERS; ++i) {
        wt[i] = std::thread(int_writer, i, std::ref(s));
    }

    std::cout << "running reader threads\n";
    for (int i = 0; i < NUM_READERS; ++i) {
        rt[i] = std::thread(int_reader, std::ref(s));
    }

    for (auto &t: wt)
        t.join();

    for (auto &t: rt)
        t.join();

    std::cout << std::endl;
    std::cout << " == Final size of SafeSet: == " << s.size() << std::endl;
    std::cout << " == Final content of SafeSet: == " << std::endl;
    int_reader(s);

    std::cout << " == Correctness check: == " << std::endl;
    assert(s.size() == NUM_WRITERS * NUM_ITEMS);
    int i = 0;
    auto it = s.begin();
    for (; it != s.end(); it = s.next(it)) {
        assert(*it == i++);
    }
    std::cout << "OK" << std::endl;

}

void order_writer(int id, SafeSet<std::shared_ptr<Order>, buy_cmp> &s) {
    for (int i = 0; i < NUM_ITEMS; ++i) {
        int x = id * NUM_ITEMS + i;
        s.insert(std::make_shared<Order>(x, x, x, x));
    }
}

void order_reader(SafeSet<std::shared_ptr<Order>, buy_cmp> &s) {
    auto it = s.begin();
    for (; it != s.end(); it = s.next(it)) {
        std::cout << *it << std::endl;
    }
}

void order_check() {
    SafeSet<std::shared_ptr<Order>, buy_cmp> s;
    std::vector<std::thread> rt(NUM_READERS);
    std::vector<std::thread> wt(NUM_WRITERS);

    std::cout << "running writer threads\n";
    for (int i = 0; i < NUM_WRITERS; ++i) {
        wt[i] = std::thread(order_writer, i, std::ref(s));
    }

    std::cout << "running reader threads\n";
    for (int i = 0; i < NUM_READERS; ++i) {
        rt[i] = std::thread(order_reader, std::ref(s));
    }

    for (auto &t: wt)
        t.join();

    for (auto &t: rt)
        t.join();

    std::cout << std::endl;
    std::cout << " == Final size of SafeSet: == " << s.size() << std::endl;
    std::cout << " == Final content of SafeSet: == " << std::endl;
    order_reader(s);

    std::cout << " == Correctness check: == " << std::endl;
    assert(s.size() == NUM_WRITERS * NUM_ITEMS);
    std::cout << "OK" << std::endl;

}

int main() {
    int_check();
    order_check();
    return 0;
}
