#include <iostream>
#include <unordered_set>

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

    void Catalogue::AddDistances(const std::unordered_map<std::pair<std::string,std::string>,double,PairHasher>& distances){
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
        std::vector<Bus*> output;
        for (auto [name,bus]: buses_id_) {
            output.push_back(bus);
        }
        return output;
    }

    std::vector<Stop*> Catalogue::GetAllStops() const {
        std::vector<Stop*> output;
        for (auto [name,stop]: stops_id_) {
            output.push_back(stop);
        }
        return output;
    }
// ---------- RequestProcessor ---------

    void RequestProcessor::SetCatalogue(Catalogue& catalogue) {
        catalogue_ = &catalogue;    
    }

    void RequestProcessor::AddStopCommand(const std::string& name, 
                        const geo::Coordinates& coordinates, 
                        const std::vector<std::pair<std::string,double>>& distances) {
        stop_commands_.push_back({name,coordinates});
        for (const auto&[name2,distance] : distances){
            distances_[std::make_pair(name,name2)] = distance;
        }
    }

    void RequestProcessor::AddBusCommand(const std::string& name, bool is_roundtrip, const std::vector<std::string>& stops){
        bus_commands_.push_back({name,is_roundtrip,stops});
    }

    void RequestProcessor::FillCatalogue(){
        for (auto&& stop : stop_commands_ ) {
            catalogue_->AddStop(std::move(stop));
        }
        catalogue_->AddDistances(distances_);
        for (auto bus_command : bus_commands_){
            Bus new_bus;
            new_bus.name = bus_command.name;
            new_bus.is_roundtrip = bus_command.is_roundtrip;
            std::unordered_set<Stop*> unique_stops;
            if (!new_bus.is_roundtrip) {
                std::vector<std::string> reverse = bus_command.stops;
                for (int i = static_cast<int>(bus_command.stops.size()) - 2; i >= 0; i--) {
                    reverse.push_back(bus_command.stops[static_cast<size_t>(i)]);
                }
                bus_command.stops = reverse;
            }
            std::string prev_stop = bus_command.stops.at(0);
            bool first = true;
            for(std::string stop : bus_command.stops) {
                Stop* stop_ = catalogue_->GetStop(stop).value();
                if (!first) {
                    Stop* prev_ = catalogue_->GetStop(prev_stop).value();
                    new_bus.geo_length += ComputeDistance(prev_->coordinates, stop_->coordinates);
                    auto distance = catalogue_->GetDistance(prev_->name,stop_->name);
                    if (distance.has_value()){
                        new_bus.length += distance.value();
                    }
                }
                new_bus.stops.push_back(catalogue_->GetStop(std::string(stop)).value());
                unique_stops.insert(catalogue_->GetStop(std::string(stop)).value());
                prev_stop = stop;
                first = false;
            }
            
            new_bus.unique_stops = static_cast<int>(unique_stops.size());
            new_bus.curvature = new_bus.length/new_bus.geo_length;
            new_bus.stops_count = static_cast<int>(new_bus.stops.size());
            Bus* bus_pointer =catalogue_->AddBus(std::move(new_bus));
            for (auto stop : unique_stops){
                stop->buses.insert(bus_pointer->name);
            }
        }
    }

    void RequestProcessor::AddRequest(domain::request::Command request) {
        requests_.push_back(request);
    }

    std::vector<domain::request::Response> RequestProcessor::GetRequests() {
        std::vector<domain::request::Response> output;
        for (auto request : requests_){
            domain::request::Response response;
            response.id = request.id;
            if (request.type == domain::request::Type::STOP) {
                auto stop = catalogue_->GetStop(request.name);
                if (stop.has_value()){
                    response.stop_data = stop.value();
                    response.type = domain::request::Type::STOP;
                }
            } 
            else if (request.type == domain::request::Type::BUS) {
                auto bus = catalogue_->GetBus(request.name);
                if (bus.has_value()){
                    response.bus_data = bus.value();
                    response.type = domain::request::Type::BUS;
                }
            }
            else if (request.type == domain::request::Type::MAP) {
                response.type = domain::request::Type::MAP;
            }
            output.push_back(response);
        }
        return output;
    }

}