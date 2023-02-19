#ifndef SAFEMAP_HPP
#define SAFEMAP_HPP

#include <unordered_map>
#include <mutex>
#include <shared_mutex>

template <typename Key, typename Val>
class SafeMap {
private:
  std::shared_mutex mtx;
  std::unordered_map<Key, Val> hmap;

public:
  void put(const std::pair<Key, Val>& item) {
    std::unique_lock lock(mtx);
    // if (hmap.find(item.first) != hmap.end())
    //   hmap.erase(item.first);
    hmap.insert(item);
  }

  Val& get(const Key& key) {
    std::shared_lock lock(mtx);
    auto ptr = hmap.find(key);
    // for now, default value = hmap.begin()->second
    Val &ret = (ptr != hmap.end()) ? ptr->second : hmap.begin()->second;
    return ret;
  }
  
  uint32_t size() {
    std::shared_lock lock(mtx);
    return hmap.size();
  }

  bool contains(const Key& key) {
    std::shared_lock lock(mtx);
    bool exists = (hmap.find(key) != hmap.end());
    return exists;
  }

};

#endif // SAFEMAP_HPP
