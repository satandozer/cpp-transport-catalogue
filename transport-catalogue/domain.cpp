#include "domain.h"

namespace domain {
    size_t StopPairHasher::operator()(const std::pair<domain::Stop*,domain::Stop*>& stop_pair) const {
        std::hash<std::string> hash_;
        std::size_t hash1 = hash_(stop_pair.first->name);
        std::size_t hash2 = hash_(stop_pair.second->name);
        return (hash1 + 11*hash2);
    }

    size_t PairHasher::operator()(const std::pair<std::string,std::string>& stop_pair)const{
        std::hash<std::string> hash_;
        std::size_t hash1 = hash_(stop_pair.first);
        std::size_t hash2 = hash_(stop_pair.second);
        return (hash1 + 7*hash2);
    }
}