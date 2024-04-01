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

    void JsonReader::SetRouter(transport_router::TransportRouter& router) {
        router_ = &router;
    }
    
    void JsonReader::SetCatalogue(transport::Catalogue& catalogue) {
        catalogue_ = &catalogue;    
    }

// ---------- JSON Parsing ----------

    domain::ParsedInput JsonReader::ParseJson(std::istream& input) {
        commands_ptr_ = new domain::ParsedInput;
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
        if (requests.GetRoot().AsMap().count("render_settings")) {
            json::Dict svg_settings = requests.GetRoot().AsMap().at("render_settings").AsMap();
            map_renderer_->SetSettings(ParseMapSettings(svg_settings));
        }
        if (requests.GetRoot().AsMap().count("routing_settings")) {
            json::Dict route_settings = requests.GetRoot().AsMap().at("routing_settings").AsMap();
            router_->SetSettings(ParseRouteSettings(route_settings));
        }
        for (auto& request : stat){
            json::Dict data = request.AsMap();
            if (data.at("type").AsString() == "Stop") {
                AddRequest({data.at("id").AsInt(),data.at("name").AsString(),domain::request::Type::STOP,""});
            } else if (data.at("type").AsString() == "Bus") {
                AddRequest({data.at("id").AsInt(),data.at("name").AsString(),domain::request::Type::BUS,""});
            } else if (data.at("type").AsString() == "Map") {
                AddRequest({data.at("id").AsInt(),"",domain::request::Type::MAP,""});
            } else if (data.at("type").AsString() == "Route") {
                AddRequest({data.at("id").AsInt(),
                            data.at("from").AsString(),
                            domain::request::Type::ROUTE,
                            data.at("to").AsString()});
            }
        }
        return *commands_ptr_;
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

    renderer::Settings JsonReader::ParseMapSettings(const json::Dict& request) const {
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

    void JsonReader::AddStopCommand(const std::string& name, 
                        const geo::Coordinates& coordinates, 
                        const std::vector<std::pair<std::string,double>>& distances) {
        
        commands_ptr_->stop_commands.push_back({name,coordinates});
        for (const auto&[name2,distance] : distances){
            commands_ptr_->distances[std::make_pair(name,name2)] = distance;
        }
    }

    void JsonReader::AddBusCommand(const std::string& name, bool is_roundtrip, const std::vector<std::string>& stops){
        commands_ptr_->bus_commands.push_back({name,is_roundtrip,stops});
    }

    void JsonReader::AddRequest(domain::request::Command request) {
        commands_ptr_->requests.push_back(request);
    } 

    domain::router_data::Settings JsonReader::ParseRouteSettings(const json::Dict& request) const {
        domain::router_data::Settings output;
        output.bus_wait_time = request.at("bus_wait_time").AsInt();
        output.velocity = request.at("bus_velocity").AsDouble();
        return output;
    }

// ---------- JSON Printing ----------

    json::Document JsonReader::PrintJson(std::ostream& output, const std::vector<domain::request::Response>& requests) const {
        json::Array response_arr;
        for (auto response : requests){
            json::Builder dict_builder;
            //response_dict["request_id"] = json::Node(response.id);
            dict_builder.StartDict().Key("request_id").Value(response.id);
            if (response.stop_data == nullptr && response.bus_data == nullptr && response.type != domain::request::Type::MAP){
                //response_dict["error_message"] = json::Node(std::string("not found"));
                dict_builder.Key("error_message").Value(std::string("not found"));
            } else if (response.type == domain::request::Type::STOP){
                PrintStop(dict_builder, response.stop_data);
            } else if (response.type == domain::request::Type::BUS){
                PrintBus(dict_builder,response.bus_data);
            } else if (response.type == domain::request::Type::MAP){
                PrintMap(dict_builder);
            } else if (response.type == domain::request::Type::ROUTE){
                PrintRoute(dict_builder, response.stop_data, response.stop_to);
            }
            response_arr.push_back(dict_builder.EndDict().Build().AsMap());

        }
        json::Builder builder;
        builder.Value(response_arr);
        json::Print(json::Document(builder.Build()),output);
        return json::Document(builder.Build());
    }

    void JsonReader::PrintStop(json::Builder& builder, domain::Stop* stop) const{
        /*json::Array buses;
        for (auto bus : stop->buses){
            buses.push_back(json::Node(std::string(bus)));
        }
        response_dict["buses"] = json::Node(buses);*/
        builder.Key("buses").StartArray();
        for (auto bus : stop->buses){
            builder.Value((std::string(bus)));
        }
        builder.EndArray();
    }
    void JsonReader::PrintBus(json::Builder& builder, domain::Bus* bus) const{
        /*
        response_dict["curvature"] = json::Node(bus->curvature);
        response_dict["route_length"] = json::Node(bus->length);
        response_dict["stop_count"] = json::Node(bus->stops_count);
        response_dict["unique_stop_count"] = json::Node(bus->unique_stops);
        */
        builder
            .Key("curvature")           .Value(bus->curvature)
            .Key("route_length")        .Value(bus->length)
            .Key("stop_count")          .Value(bus->stops_count)
            .Key("unique_stop_count")   .Value(bus->unique_stops);

    }
    void JsonReader::PrintMap(json::Builder& builder) const {
        std::ostringstream output;
        map_renderer_->RenderMap(output);
        builder.Key("map").Value(output.str());
        //response_dict["map"] = json::Node(output.str());
    }
    
    void JsonReader::PrintRoute(json::Builder& builder, domain::Stop* stop_from, domain::Stop* stop_to) const {
        auto response = router_->GetRoute(stop_from,stop_to);
        if (!response.has_value()){
            builder.Key("error_message").Value(std::string("not found"));
            return;
        }
        builder
            .Key("total_time")  .Value(response.value().total_time)
            .Key("items")       .StartArray();
        for (auto item : response.value().items){
            builder.StartDict().Key("time").Value(item.time);
            if (item.type == domain::router_data::EdgeType::WAIT){
                builder
                    .Key("type")        .Value("Wait")
                    .Key("stop_name")   .Value(item.name);
            } else if (item.type == domain::router_data::EdgeType::BUS){
                builder
                    .Key("type")        .Value("Bus")
                    .Key("bus")         .Value(item.name)
                    .Key("span_count")  .Value(item.span_count.value());
            }
            builder.EndDict();
        }
        builder.EndArray();
    }

}