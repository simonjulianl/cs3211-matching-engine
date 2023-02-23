#ifndef SAFESET_HPP
#define SAFESET_HPP

#include <set>
#include <iterator>
#include <mutex>
#include <shared_mutex>
#include "order.hpp"

template<typename Key, typename Compare = std::less<Key>>
class SafeSet {
private:
    std::shared_mutex mtx;
    std::set<Key, Compare> s;
    typedef typename std::set<Key, Compare>::iterator it_t;

public:
    void insert(const Key &item) {
        std::unique_lock lock(mtx);
        s.insert(item);
    }

    void erase(const it_t &start, const it_t &end) {
        std::unique_lock lock(mtx);
        s.erase(start, end);
    }

    void erase(const it_t &it) {
        std::unique_lock lock(mtx);
        s.erase(it);
    }

    typename std::set<Key, Compare>::iterator begin() {
        std::shared_lock lock(mtx);
        return s.cbegin();
    }

    typename std::set<Key, Compare>::iterator end() {
        std::shared_lock lock(mtx);
        return s.cend();
    }

    typename std::set<Key, Compare>::iterator next(const it_t &it) {
        std::shared_lock lock(mtx);
        return std::next(it);
    }

    uint32_t size() {
        std::shared_lock lock(mtx);
        return s.size();
    }

    bool contains(const Key &item) {
        std::shared_lock lock(mtx);
        bool exists = (s.find(item) != s.end());
        return exists;
    }
};

#endif // SAFESET_HPP
