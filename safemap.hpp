#ifndef SAFEMAP_HPP
#define SAFEMAP_HPP

#include <unordered_map>
#include <mutex>
#include <shared_mutex>

template <typename KeyType, typename ValueType>
class SafeMap {
private:
  std::shared_mutex mtx;
  std::unordered_map<KeyType, ValueType> hmap;

public:
  ValueType& get(const KeyType &key) {
    std::shared_lock lock(mtx);
    auto ptr = hmap.find(key);
    // for now, default value = hmap.begin()->second
    ValueType &ret = (ptr != hmap.end()) ? ptr->second : hmap.begin()->second;
    return ret;
  }

  bool contains(const KeyType &key) {
    std::shared_lock lock(mtx);
    bool exists = (hmap.find(key) != hmap.end());
    return exists;
  }

  void put(const std::pair<KeyType, ValueType> &item) {
    std::unique_lock lock(mtx);
    // if (hmap.find(item.first) != hmap.end())
    //   hmap.erase(item.first);
    hmap.insert(item);
  }

};

#endif // SAFEMAP_HPP
