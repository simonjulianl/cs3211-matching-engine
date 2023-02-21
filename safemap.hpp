#ifndef SAFEMAP_HPP
#define SAFEMAP_HPP

#include <unordered_map>
#include <mutex>
#include <optional>
#include <shared_mutex>

template<typename Key, typename Val>
class SafeMap {
private:
    std::shared_mutex mtx;
    std::unordered_map<Key, Val> hmap;

public:
    void put(const std::pair<Key, Val> &item) {
        std::unique_lock lock(mtx);
        // if (hmap.find(item.first) != hmap.end())
        //   hmap.erase(item.first);
        hmap.insert(item);
    }

    bool contains(const Key &key) {
        std::shared_lock lock(mtx);
        bool exists = (hmap.find(key) != hmap.end());
        return exists;
    }

    Val &getOrDefault(const Key &key, const Val &default_value) {
        typename std::unordered_map<Key, Val>::iterator ptr;
        {
            std::shared_lock lock(mtx);
            ptr = hmap.find(key);
        }
        if (ptr == hmap.end()) {
            {
                std::unique_lock lock(mtx);
                hmap.insert({key, default_value});
                return hmap.find(key)->second;
            }
        }

        return ptr->second;
    }

    void erase(const Key &key) {
        std::unique_lock lock(mtx);
        if (hmap.find(key) == hmap.end())
            return;
        hmap.erase(key);
    }

    uint32_t size() {
        std::shared_lock lock(mtx);
        return hmap.size();
    }

};

#endif // SAFEMAP_HPP
