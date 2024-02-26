#pragma once

#include <vector>
#include <string>
#include <set>
#include <variant>
#include <unordered_map>
#include "geo.h"



namespace domain {
        struct Stop {

            Stop(const std::string& name_, const geo::Coordinates& coordinates_)
                : name(name_)
                , coordinates(coordinates_){}	

            Stop(const std::string& name_, const double& latitude_, const double& longitude_)
                : name(name_){
                    coordinates.lat = latitude_;
                    coordinates.lng = longitude_;
                }

            std::string name = "";
            geo::Coordinates coordinates = {0.0,0.0};
            std::set<std::string_view> buses;
        };

        struct Bus {
            std::string name = "";
            std::vector<Stop*> stops;
            bool is_roundtrip;
            int stops_count = 0;
            int unique_stops = 0;
            double length = 0.0;	
            double geo_length = 0.0;
            double curvature = 0.0;
        };

    struct StopPairHasher {
        public:
            size_t operator()(const std::pair<domain::Stop*,domain::Stop*>& stop_pair)const;
    };

    struct PairHasher {
        public:
            size_t operator()(const std::pair<std::string,std::string>& stop_pair)const;
    };

    typedef std::unordered_map<std::pair<std::string,std::string>,double,PairHasher> DistancesStringMap;
    typedef std::unordered_map<std::pair<Stop*,Stop*>,double,StopPairHasher> DistancesStopMap;

    namespace command{

        struct BusDescription{
            std::string name;
            bool is_roundtrip;
            std::vector<std::string> stops;
        };
    }

    namespace request {
        enum class Type {
            STOP,
            BUS,
            MAP
        };

        struct Command{
            int id;
            std::string name;
            request::Type type;
        };

        struct Response{
            int id;
            request::Type type;
            Stop* stop_data = nullptr;
            Bus* bus_data = nullptr;
        };
    }

    struct ParsedInput {
        std::vector<Stop> stop_commands;
        std::vector<domain::command::BusDescription> bus_commands;
        DistancesStringMap distances;
        std::vector<domain::request::Command> requests;
    };
}