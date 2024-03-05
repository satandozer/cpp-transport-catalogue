#include "request_handler.h"

#include <unordered_set>


namespace request {

    // ---------- Processor ---------

    void Handler::FillCatalogue(){
        for (auto&& stop : commands_.stop_commands ) {
            catalogue_->AddStop(std::move(stop));
        }
        catalogue_->AddDistances(commands_.distances);
        for (auto bus_command : commands_.bus_commands){
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
        catalogue_->SortAll();
    }

    std::vector<domain::request::Response> Handler::GetRequests() const {
        std::vector<domain::request::Response> output;
        for (auto request : commands_.requests){
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


    void Handler::ReadJson(std::istream& input) {
        commands_= json_reader_->ParseJson(input);
    }
    void Handler::PrintJson(std::ostream& output) const{
        std::vector<domain::request::Response> responses = GetRequests();
        json_reader_->PrintJson(output,responses);
    }
    void Handler::RenderMap(std::ostream& output) const{
        renderer_->RenderMap(output);
    }
    void Handler::GenerateOutput(std::ostream& json_output, std::ostream& svg_output) const{
        PrintJson(json_output);
        RenderMap(svg_output);
    }
}