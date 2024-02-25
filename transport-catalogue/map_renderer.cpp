#include "map_renderer.h"

namespace renderer {
    
    void MapRenderer::SetSettings(Settings&& settings) {
        settings_ = std::move(settings);
    }

    void MapRenderer::SetCatalogue(transport::Catalogue& catalogue) {
        catalogue_ = &catalogue;
    }

    void MapRenderer::RenderMap(std::ostream& output) const {
        std::vector<geo::Coordinates> all_coordinates;
        for (auto stop : catalogue_->GetAllStops()){
            if (stop->buses.empty()){
                continue;
            }
            all_coordinates.push_back(stop->coordinates);
        }
        SphereProjector project (all_coordinates.begin(),all_coordinates.end(),
            settings_.width,settings_.height,settings_.padding);

        svg::Document output_document;

        size_t color = 0;
        for(auto bus : catalogue_->GetAllBus()){
            if (bus->stops.empty()){
                continue;
            }
            svg::Polyline bus_line = RenderBus(bus,project);
            bus_line.SetStrokeColor(settings_.color_palette[color]);
            color = NextColor(color);

            output_document.Add(std::move(bus_line));
        }

        color = 0;
        for(auto bus : catalogue_->GetAllBus()){
            if (bus->stops.empty()){
                continue;
            }
            RenderBusText(bus,project,settings_.color_palette[color]).AddToDocument(output_document);
            color = NextColor(color);
        }

        for(auto stop : catalogue_->GetAllStops()){
            if (stop->buses.empty()){
                continue;
            }
            output_document.Add(RenderStop(stop,project));
        }

        for(auto stop : catalogue_->GetAllStops()){
            if (stop->buses.empty()){
                continue;
            }
            RenderStopText(stop,project).AddToDocument(output_document);
        }

        output_document.Render(output);
    }

    svg::Polyline MapRenderer::RenderBus(domain::Bus* bus, const SphereProjector& project) const {
        svg::Polyline bus_line;

        bus_line.SetStrokeWidth(settings_.line_width);
        bus_line.SetFillColor(svg::NoneColor);
        bus_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        bus_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (auto stop : bus->stops){
            svg::Point p = project(stop->coordinates);
            bus_line.AddPoint(std::move(p));
        }
        return bus_line;
    }

    size_t MapRenderer::NextColor(size_t prev) const {
        size_t max_color = settings_.color_palette.size() - 1;
        return (prev == max_color ? 0 : ++prev);
    }

    svg::Circle MapRenderer::RenderStop(domain::Stop* stop, const SphereProjector& project) const{
        svg::Circle stop_label;
        stop_label.SetRadius(settings_.stop_radius);
        stop_label.SetFillColor("white");
        stop_label.SetCenter(project(stop->coordinates));
        return stop_label;
    }

    UnderlayedText MapRenderer::RenderStopText(domain::Stop* stop, const SphereProjector& project) const{
        UnderlayedText text(settings_,TextType::STOP);
        text.SetText(stop->name);
        text.SetPosition(project(stop->coordinates));
        return text;
    }

    BusText MapRenderer::RenderBusText(domain::Bus* bus, const SphereProjector& project, const svg::Color& color) const{
        BusText text(settings_);
        text.SetColor(color);
        text.SetText(bus->name);
        if (bus->stops.at(0) == bus->stops.at((bus->stops.size()-1)/2) || bus->is_roundtrip) {
            text.is_roundtrip = true;
        }
        text.begin.SetPosition(project(bus->stops.at(0)->coordinates));
        text.end.SetPosition(project(bus->stops.at((bus->stops.size()-1)/2)->coordinates));
        return text;
    }
}