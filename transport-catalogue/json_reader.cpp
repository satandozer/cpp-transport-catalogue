#include "json_reader.h"
#include <sstream>

namespace json_reader {

// ---------- JSON Reader setup ----------
    JsonReader::JsonReader(transport::Catalogue& catalogue) {
        SetCatalogue(catalogue);    
    }

    void JsonReader::SetRenderer(renderer::MapRenderer& map_renderer) {
        map_renderer_ = &map_renderer;
    }

// ---------- JSON Parsing ----------

    void JsonReader::ParseJson(std::istream& input) {
        json::Document requests = json::Load(input);
        json::Array base = requests.GetRoot().AsMap().at("base_requests").AsArray();
        json::Array stat = requests.GetRoot().AsMap().at("stat_requests").AsArray();
        for (auto& request : base) {
            json::Dict data = request.AsMap();
            if (data.at("type").AsString() == "Stop"){
                AddStopCommand(ParseName(data),ParseCoordinates(data),ParseDistances(data));
            }
            else if (data.at("type").AsString() == "Bus"){
                AddBusCommand(ParseName(data),ParseRoundtrip(data),ParseStops(data));
            }
        }
        FillCatalogue();
        if (map_renderer_ != nullptr) {
            json::Dict svg_settings = requests.GetRoot().AsMap().at("render_settings").AsMap();
            map_renderer_->SetSettings(ParseSettings(svg_settings));
        }
        for (auto& request : stat){
            json::Dict data = request.AsMap();
            if (data.at("type").AsString() == "Stop") {
                AddRequest({data.at("id").AsInt(),data.at("name").AsString(),domain::request::Type::STOP});
            } else if (data.at("type").AsString() == "Bus") {
                AddRequest({data.at("id").AsInt(),data.at("name").AsString(),domain::request::Type::BUS});
            } else if (data.at("type").AsString() == "Map") {
                AddRequest({data.at("id").AsInt(),"",domain::request::Type::MAP});
            }        
        }
    }

    std::string JsonReader::ParseName(const json::Dict& request) const {
        return request.at("name").AsString();
    }

    geo::Coordinates JsonReader::ParseCoordinates(const json::Dict& request) const {
        geo::Coordinates location;
        location.lat = request.at("latitude").AsDouble();
        location.lng = request.at("longitude").AsDouble();
        return location;
    }

    std::vector<std::pair<std::string,double>> JsonReader::ParseDistances(const json::Dict& request) const {
        std::vector<std::pair<std::string,double>> distances;
        for (auto [stop,dist] : request.at("road_distances").AsMap()){
            distances.push_back(std::make_pair(stop,dist.AsDouble()));
        }
        return distances;
    }

    std::vector<std::string> JsonReader::ParseStops(const json::Dict& request) const{
        std::vector<std::string> stops;
        for (auto stop : request.at("stops").AsArray()){
            stops.push_back(stop.AsString());
        }
        return stops;
    }

    bool JsonReader::ParseRoundtrip(const json::Dict& request) const{
        return request.at("is_roundtrip").AsBool();
    }

    renderer::Settings JsonReader::ParseSettings(const json::Dict& request) const {
        renderer::Settings output;

        output.width = request.at("width").AsDouble();
        output.height = request.at("height").AsDouble();
        output.padding = request.at("padding").AsDouble();

        output.line_width = request.at("line_width").AsDouble();
        output.stop_radius = request.at("stop_radius").AsDouble();

        output.bus_label_font_size = static_cast<uint32_t>(request.at("bus_label_font_size").AsInt());
        output.bus_label_offset.x = request.at("bus_label_offset").AsArray()[0].AsDouble();
        output.bus_label_offset.y = request.at("bus_label_offset").AsArray()[1].AsDouble();

        output.stop_label_font_size = static_cast<uint32_t>(request.at("stop_label_font_size").AsInt());
        output.stop_label_offset.x = request.at("stop_label_offset").AsArray()[0].AsDouble();
        output.stop_label_offset.y = request.at("stop_label_offset").AsArray()[1].AsDouble();   

        output.underlayer_color = ParseColor(request.at("underlayer_color"));
        output.underlayer_width = request.at("underlayer_width").AsDouble();

        for (auto color : request.at("color_palette").AsArray()) {
            output.color_palette.push_back(ParseColor(color));
        }
        return output;
    }

    svg::Color JsonReader::ParseColor(const json::Node& color_node) const {
        svg::Color output;
        if (color_node.IsArray()) {
            if (color_node.AsArray().size() == 3){
                svg::Rgb rgb;
                rgb.red = static_cast<uint8_t>(color_node.AsArray()[0].AsInt());
                rgb.green = static_cast<uint8_t>(color_node.AsArray()[1].AsInt());
                rgb.blue = static_cast<uint8_t>(color_node.AsArray()[2].AsInt());
                output = rgb;
            } else {
                svg::Rgba rgba;
                rgba.red = static_cast<uint8_t>(color_node.AsArray()[0].AsInt());
                rgba.green = static_cast<uint8_t>(color_node.AsArray()[1].AsInt());
                rgba.blue = static_cast<uint8_t>(color_node.AsArray()[2].AsInt());
                rgba.opacity = color_node.AsArray()[3].AsDouble();
                output = rgba;
            }
        } else {
            output = color_node.AsString();
        }
        return output;
    }

// ---------- JSON Printing ----------

    json::Document JsonReader::PrintJson(std::ostream& output) {
        json::Array output_array;
        for (auto response : GetRequests()){
            json::Dict response_print;
            response_print["request_id"] = json::Node(response.id);

            if (response.stop_data == nullptr && response.bus_data == nullptr && response.type != domain::request::Type::MAP){
                response_print["error_message"] = json::Node(std::string("not found"));
            } else if (response.type == domain::request::Type::STOP){
                PrintStop(response_print,response.stop_data);
            } else if (response.type == domain::request::Type::BUS){
                PrintBus(response_print,response.bus_data);
            } else if (response.type == domain::request::Type::MAP){
                PrintMap(response_print);
            }
            output_array.push_back(json::Node(response_print));
        }
        json::Print(json::Document(json::Node(output_array)),output);
        return json::Document(json::Node(output_array));
    }

    void JsonReader::PrintStop(json::Dict& response, domain::Stop* stop) const{
        json::Array buses;
        for (auto bus : stop->buses){
            buses.push_back(json::Node(std::string(bus)));
        }
        response["buses"] = json::Node(buses);
    }
    void JsonReader::PrintBus(json::Dict& response, domain::Bus* bus) const{
        response["curvature"] = json::Node(bus->curvature);
        response["route_length"] = json::Node(bus->length);
        response["stop_count"] = json::Node(bus->stops_count);
        response["unique_stop_count"] = json::Node(bus->unique_stops);
    }
    void JsonReader::PrintMap(json::Dict& response) const {
        std::ostringstream output;
        map_renderer_->RenderMap(output);
        response["map"] = json::Node(output.str());
    }

}