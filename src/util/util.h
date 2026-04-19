#ifndef __DHomeUtil__
#define __DHomeUtil__

namespace dhome::util {

template<typename Error, typename KeyT, typename ValueT, typename MapT>
Error getValFromMap(const MapT& map, const KeyT& key, ValueT& val) {
    std::cout << "Looking for key: " << key << "\n";

    auto v = map.find(key);
    if(v == map.end()) {
        return Error::kError;
    } else {
        val = v->second; 
        return Error::kNoError;
    }
}

}

#endif