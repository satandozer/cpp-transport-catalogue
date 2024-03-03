#include "transport_catalogue.h"

namespace transport{

    using domain::Stop;
    using domain::Bus;
    using domain::StopPairHasher;
    using domain::PairHasher;

// ---------- Catalogue ---------

    void Catalogue::AddStop(domain::Stop&& stop){
        stops_.push_back(std::move(stop));
        std::string_view new_name = stops_.back().name;
        Stop* stop_pointer = &stops_.back();
        stops_id_.insert({new_name,stop_pointer});
    }

    void Catalogue::AddDistances(const DistancesStringMap& distances){
        for (const auto& [stop_pair,dst] : distances){
            Stop* stop1 = GetStop(stop_pair.first).value();
            Stop* stop2 = GetStop(stop_pair.second).value();
            distances_[std::make_pair(stop1,stop2)] = dst;
            if (!distances_.count(std::make_pair(stop2,stop1))){
                distances_[std::make_pair(stop2,stop1)] = dst;
            }
        }
    }

    Bus* Catalogue::AddBus(domain::Bus&& bus){
        buses_.push_back(std::move(bus));
        std::string_view name = buses_.back().name;       
        Bus* bus_pointer = &buses_.back();
        buses_id_.insert({name,bus_pointer});
        return bus_pointer;
    }

    std::optional<Stop*> Catalogue::GetStop(const std::string& stop_name) const {
        return stops_id_.count(stop_name) ? stops_id_.at(stop_name) : nullptr;
    }

    std::optional<Bus*> Catalogue::GetBus(const std::string& bus_name) const{
        return buses_id_.count(bus_name) ? buses_id_.at(bus_name) : nullptr;
    }

    std::optional<double> Catalogue::GetDistance(const std::string& first, const std::string& second) const {
        
        auto stop1 = GetStop(first);
        auto stop2 = GetStop(second);
        if (!stop1.has_value() || !stop2.has_value()){
            return false;
        }
        auto stop_pair = std::make_pair(stop1.value(),stop2.value());
        if (distances_.count(stop_pair)){
            return distances_.at(stop_pair);
        }
        return false;
    }

    std::vector<Bus*> Catalogue::GetAllBus() const {
        return all_buses_;
    }

    std::vector<Stop*> Catalogue::GetAllStops() const {
        return all_stops_;
    }

    void Catalogue::SortAll() {
        for (auto [name,bus]: buses_id_) {
            all_buses_.push_back(bus);
        }
        std::sort(all_buses_.begin(), all_buses_.end(),[](Bus* lhs, Bus* rhs){
            return lexicographical_compare(lhs->name.begin(),lhs->name.end(),
                        rhs->name.begin(),rhs->name.end());
        });
        for (auto [name,stop]: stops_id_) {
            all_stops_.push_back(stop);
        }
        std::sort(all_stops_.begin(),all_stops_.end(),[](Stop* lhs, Stop* rhs){
            return lexicographical_compare(lhs->name.begin(),lhs->name.end(),
                        rhs->name.begin(),rhs->name.end());
        });
    }

}