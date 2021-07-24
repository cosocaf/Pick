#ifndef PICKC_UTILS_MAP_UTILS_H_
#define PICKC_UTILS_MAP_UTILS_H_

#include <map>
#include <unordered_map>

namespace pickc
{
  template<typename Key, typename Value>
  inline size_t indexOf(const std::map<Key, Value>& map, const Key& key)
  {
    assert(map.find(key) != map.end());
    return std::distance(map.begin(), map.find(key));
  }
  template<typename Key, typename Value>
  inline bool keyExists(const std::map<Key, Value>& map, const Key& key)
  {
    return map.find(key) != map.end();
  }
  template<typename Key, typename Value>
  inline bool keyExists(const std::unordered_map<Key, Value>& map, const Key& key)
  {
    return map.find(key) != map.end();
  }
}

#endif // PICKC_UTILS_MAP_UTILS_H_