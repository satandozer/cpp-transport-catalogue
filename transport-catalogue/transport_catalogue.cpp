#include <iostream>
#include <unordered_set>
#include "transport_catalogue.h"

namespace transport{

    using data::Stop;
    using data::Bus;
    
    void Catalogue::AddStop(const std::string& name, const geo::Coordinates& coordinates_){
        Stop new_stop(name,coordinates_);
        stops_.push_back(std::move(new_stop));
        std::string_view new_name = stops_.back().name;
        Stop* stop_pointer = &stops_.back();
        stops_id_.insert({new_name,stop_pointer});
    }

    void Catalogue::AddBus(const std::string& name, const std::vector<std::string>& stops){
        std::vector<Stop*> new_bus_stops;
        int stops_count = 0;
        int unique_count = 0;
        double length = 0.0;

        geo::Coordinates last = stops_id_.at(stops[0])->coordinates;
        std::unordered_set<Stop*> unique_stops;
        for(std::string stop : stops) {
            length += ComputeDistance(last, stops_id_.at(std::string(stop))->coordinates);
            ++stops_count;
            new_bus_stops.push_back(stops_id_.at(std::string(stop)));
            unique_stops.insert(stops_id_.at(std::string(stop)));
            last = stops_id_.at(stop)->coordinates;
        }
        unique_count = static_cast<int>(unique_stops.size());
        buses_.push_back({name,new_bus_stops,stops_count,unique_count,length});
        Bus* bus_pointer = &buses_.back();
        buses_id_.insert({name,bus_pointer});
        for (auto stop : unique_stops){
            stop->buses.insert(bus_pointer->name);
        }
    }

    std::optional<data::Stop*> Catalogue::GetStop(const std::string& stop_name) const {
        return stops_id_.count(stop_name) ? stops_id_.at(stop_name) : nullptr;
    }

    std::optional<data::Bus*> Catalogue::GetBus(const std::string& bus_name) const{
        return buses_id_.count(bus_name) ? buses_id_.at(bus_name) : nullptr;
    }
}