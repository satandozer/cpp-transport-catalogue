#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace json_reader {

class JsonReader : private transport::RequestProcessor {
    public:
        JsonReader(transport::Catalogue& catalogue);
        void SetRenderer(renderer::MapRenderer& map_renderer_);
        void ParseJson (std::istream& input);
        json::Document PrintJson (std::ostream& output);
    private:

        renderer::MapRenderer* map_renderer_ = nullptr;

        std::string ParseName(const json::Dict& request) const;
        geo::Coordinates ParseCoordinates(const json::Dict& request) const;
        std::vector<std::pair<std::string,double>> ParseDistances(const json::Dict& request) const;
        std::vector<std::string> ParseStops(const json::Dict& request) const;
        bool ParseRoundtrip(const json::Dict& request) const;
        renderer::Settings ParseSettings(const json::Dict& request) const;
        svg::Color ParseColor(const json::Node& color_node) const;

        void PrintStop(json::Dict& response, domain::Stop* stop) const;
        void PrintBus(json::Dict& response, domain::Bus* bus) const;
        void PrintMap(json::Dict& response) const;
};

}