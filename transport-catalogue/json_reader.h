#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace json_reader {

class JsonReader {
    public:
        JsonReader(transport::Catalogue& catalogue);

        void SetRenderer(renderer::MapRenderer& map_renderer_);
        void SetCatalogue(transport::Catalogue& catalogue);

        domain::ParsedInput ParseJson (std::istream& input);
        json::Document PrintJson (std::ostream& output, const std::vector<domain::request::Response>& requests) const;
    private:
        renderer::MapRenderer* map_renderer_ = nullptr;
        transport::Catalogue* catalogue_;
        domain::ParsedInput* commands_ptr_;

        void AddStopCommand(const std::string& name, 
                    const geo::Coordinates& coordinates_, 
                    const std::vector<std::pair<std::string,double>>& distances);
        void AddBusCommand(const std::string& name, bool is_roundtrip, const std::vector<std::string>& stops);
		void AddRequest (domain::request::Command);

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