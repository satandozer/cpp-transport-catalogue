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

    void Catalogue::AddDistances(const std::map<std::pair<std::string,std::string>,int>& distances){
        for (const auto& [stop_pair,dst] : distances){
            Stop* stop1 = stops_id_.at(stop_pair.first);
            Stop* stop2 = stops_id_.at(stop_pair.second);
            distances_[std::make_pair(stop1,stop2)] = dst;
            if (!distances_.count(std::make_pair(stop2,stop1))){
                distances_[std::make_pair(stop2,stop1)] = dst;
            } 
        }
    }

    void Catalogue::AddBus(const std::string& name, const std::vector<std::string>& stops){
        std::vector<Stop*> new_bus_stops;
        int stops_count = 0;
        int unique_count = 0;
        int length = 0.0;
        double geo_length = 0.0;

        geo::Coordinates last = stops_id_.at(stops[0])->coordinates;
        std::string prev_stop = stops.at(0);
        std::unordered_set<Stop*> unique_stops;

        for(std::string stop : stops) {

            Stop* stop_ = stops_id_.at(stop);
            Stop* prev_ = stops_id_.at(prev_stop);

            geo_length += ComputeDistance(prev_->coordinates, stop_->coordinates);
            std::pair<Stop*,Stop*> dst_pair = {prev_,stop_};
            if (distances_.count(dst_pair)){
                length += distances_.at(dst_pair);
            }
            ++stops_count;
            new_bus_stops.push_back(stops_id_.at(std::string(stop)));
            unique_stops.insert(stops_id_.at(std::string(stop)));
            last = stops_id_.at(stop)->coordinates;
            prev_stop = stop;
        }
        unique_count = static_cast<int>(unique_stops.size());
        double curvature = (length + 0.0)/(geo_length + 0.0);
        buses_.push_back({name,new_bus_stops,stops_count,unique_count,length,geo_length,curvature});
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