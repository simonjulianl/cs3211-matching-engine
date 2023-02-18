#ifndef SAFEMAP_HPP
#define SAFEMAP_HPP

#include "lightswitch.hpp"
#include <unordered_map>
#include <iostream>

template <typename KeyType, typename ValueType>
class SafeMap {
private:

  LightSwitch read_lightswitch;
  std::mutex in_use;
  std::unordered_map<KeyType, ValueType> hmap;

public:

  ValueType& get(const KeyType &key) {
    read_lightswitch.lock(in_use);

    auto ptr = hmap.find(key);

    // for now, default value = hmap.begin()->second
    ValueType &ret = (ptr != hmap.end()) ? ptr->second : hmap.begin()->second;

    read_lightswitch.unlock(in_use);
    return ret;
  }

  bool contains(const KeyType &key) {
    read_lightswitch.lock(in_use);

    bool exists = (hmap.find(key) != hmap.end());
    
    read_lightswitch.unlock(in_use);
    // if (!exists)
    //   std::cout << "CONTAINS ? " << (exists ? "true" : "false") << std::endl;
    return exists;
  }

  void put(const std::pair<KeyType, ValueType> &item) {
    in_use.lock();
    // std::cout << "PUT " << item.first << std::endl;
    if (hmap.find(item.first) != hmap.end())
      hmap.erase(item.first);
    hmap.insert(std::move(item));

    in_use.unlock();
  }

};

#endif // SAFEMAP_HPP
