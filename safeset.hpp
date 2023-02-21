#ifndef SAFESET_HPP
#define SAFESET_HPP

#include <set>
#include <iterator>
#include <mutex>
#include <shared_mutex>

template <typename Key, typename Compare = std::less<Key>>
class SafeSet {
private:
  std::shared_mutex mtx;
  std::set<Key, Compare> s;

public:
  void insert(const Key& item) {
    std::unique_lock lock(mtx);
    s.insert(item);
  }

  void erase(const auto& start, const auto& end) {
    std::unique_lock lock(mtx);
    s.erase(start, end);
  }

  typename std::set<Key, Compare>::iterator begin() {
    std::shared_lock lock(mtx);
    return s.cbegin();
  }
  
  typename std::set<Key, Compare>::iterator end() {
    std::shared_lock lock(mtx);
    return s.cend();
  }

  typename std::set<Key, Compare>::iterator next(const auto& it) {
    std::shared_lock lock(mtx);
    return std::next(it);
  }

  uint32_t size() {
    std::shared_lock lock(mtx);
    return s.size();
  }
  
  bool contains(const Key& item) {
    std::shared_lock lock(mtx);
    bool exists = (s.find(item) != s.end());
    return exists;
  }

};

#endif // SAFESET_HPP
