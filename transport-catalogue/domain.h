#pragma once

#include <vector>
#include <string>
#include <set>
#include <variant>
#include <optional>
#include <unordered_map>

#include "geo.h"
#include "graph.h"

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

    namespace router_data {
        using namespace graph;
        using namespace domain;

        using Time = double;

        enum class EdgeType {
            WAIT,
            BUS
        };

        struct Settings {
            int bus_wait_time = 6;
            double velocity = 40;
        };
        
        struct StopVertex {
            VertexId id;
            Stop* stop = nullptr;
        };

        struct StopVertexPair {
            StopVertex stop_begin;
            StopVertex stop_end;
        };

        struct EdgeData {
            EdgeType type;
            int span_count = 0;
            Bus* bus = nullptr;
            Stop* stop_begin = nullptr;
            Stop* stop_end = nullptr;
        };

        struct ResponseItem {
            EdgeType type;
            std::string name;
            std::optional<int> span_count;
            Time time;
        };

        struct Response{
            Time total_time;
            std::vector<ResponseItem> items; 
        };
        
    }

    namespace request {
        enum class Type {
            STOP,
            BUS,
            MAP,
            ROUTE
        };

        struct Command{
            int id;
            std::string name;
            request::Type type;
            std::optional<std::string> to_name;
        };

        struct Response{
            int id;
            request::Type type;
            Stop* stop_data = nullptr;
            Bus* bus_data = nullptr;
            Stop* stop_to = nullptr;
        };
    }

    struct ParsedInput {
        std::vector<Stop> stop_commands;
        std::vector<domain::command::BusDescription> bus_commands;
        DistancesStringMap distances;
        std::vector<domain::request::Command> requests;
    };

}