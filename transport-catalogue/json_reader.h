#pragma once
#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace json_reader {

class JsonReader {
    public:
        JsonReader(transport::Catalogue& catalogue);

        void SetRenderer(renderer::MapRenderer& map_renderer_);
        void SetRouter(transport_router::TransportRouter& router);
        void SetCatalogue(transport::Catalogue& catalogue);

        domain::ParsedInput ParseJson (std::istream& input);
        json::Document PrintJson (std::ostream& output, const std::vector<domain::request::Response>& requests) const;
    private:
        renderer::MapRenderer* map_renderer_ = nullptr;
        transport_router::TransportRouter* router_ = nullptr;
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
        renderer::Settings ParseMapSettings(const json::Dict& request) const;
        svg::Color ParseColor(const json::Node& color_node) const;
        domain::router_data::Settings ParseRouteSettings(const json::Dict& request) const;

        void PrintStop(json::Builder& builder, domain::Stop* stop) const;
        void PrintBus(json::Builder& builder, domain::Bus* bus) const;
        void PrintMap(json::Builder& builder) const;
        void PrintRoute(json::Builder& builder, domain::Stop* stop_from, domain::Stop* stop_to) const;
};

}