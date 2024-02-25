#pragma once

#include <algorithm>
#include <cstdlib>

#include "svg.h"
#include "transport_catalogue.h"

namespace renderer {

    class SphereProjector {

        const double EPSILON = 1e-6;
        
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;

        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }
    };

    // Параметры изображения
    struct Settings {
        // Параметры холста
        double width = 0; // ширина изображения в пикселях
        double height = 0; // высота изображения в пикселях
        double padding = 0; // отступ краёв карты от границ SVG-документа

        // Параметры объектов
        double line_width = 0; // толщина линий, которыми рисуются автобусные маршруты
        double stop_radius = 0; // радиус окружностей, которыми обозначаются остановки

        // Параметры текста
        uint32_t bus_label_font_size = 0; // размер текста, которым написаны названия автобусных маршрутов
        svg::Point bus_label_offset = {0.0,0.0}; // cмещение надписи с названием маршрута относительно координат конечной остановки на карте
        
        uint32_t stop_label_font_size = 0; // размер текста, которым отображаются названия остановок
        svg::Point stop_label_offset = {0.0,0.0}; // смещение названия остановки относительно её координат на карте

        //Параметры цвета
        svg::Color underlayer_color; // цвет подложки под названиями остановок и маршрутов
        double underlayer_width; // толщина подложки под названиями остановок и маршрутов
        std::vector<svg::Color> color_palette;
        
    };

    enum class TextType {
        STOP,
        BUS
    };

    struct UnderlayedText {
        UnderlayedText(){
            front_text.SetFontFamily("Verdana");
            underlayer.SetFontFamily("Verdana");
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        }

        UnderlayedText(const Settings& settings, TextType type){
            front_text.SetFontFamily("Verdana");
            underlayer.SetFontFamily("Verdana");
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            underlayer.SetFillColor(settings.underlayer_color);
            underlayer.SetStrokeColor(settings.underlayer_color);
            underlayer.SetStrokeWidth(settings.underlayer_width);

            if (type == TextType::STOP){
                front_text.SetFontSize(settings.stop_label_font_size);
                front_text.SetOffset(settings.stop_label_offset);
                underlayer.SetFontSize(settings.stop_label_font_size);
                underlayer.SetOffset(settings.stop_label_offset);
                front_text.SetFillColor("black");
            } else if (type == TextType::BUS){
                front_text.SetFontSize(settings.bus_label_font_size);
                front_text.SetOffset(settings.bus_label_offset);
                underlayer.SetFontSize(settings.bus_label_font_size);
                underlayer.SetOffset(settings.bus_label_offset);
            }
        }

        void SetText(const std::string& text) {
            front_text.SetData(text);
            underlayer.SetData(text);
        }

        void SetPosition(svg::Point point){
            front_text.SetPosition(point);
            underlayer.SetPosition(point);
        }

        void SetOffset(svg::Point offset){
            front_text.SetOffset(offset);
            underlayer.SetOffset(offset);
        }

        void SetFontSize(uint32_t font_size){
            front_text.SetFontSize(font_size);
            underlayer.SetFontSize(font_size);
        }

        void AddToDocument(svg::Document& doc){
            doc.Add(underlayer);
            doc.Add(front_text);
        }

        svg::Text front_text;
        svg::Text underlayer;
    };

    struct BusText {

        BusText() = default;

        BusText(const Settings& settings) 
            : begin(settings,TextType::BUS)
            , end(settings,TextType::BUS){
                begin.front_text.SetFontWeight("bold");
                begin.underlayer.SetFontWeight("bold");
                end.front_text.SetFontWeight("bold");
                end.underlayer.SetFontWeight("bold");
            }
        
        void SetText(const std::string& text) {
            begin.SetText(text);
            end.SetText(text);
        }

        void SetColor(svg::Color color) {
            begin.front_text.SetFillColor(color);
            end.front_text.SetFillColor(color);
        }


        
        void AddToDocument(svg::Document& doc){
            begin.AddToDocument(doc);
            if (!is_roundtrip){
                end.AddToDocument(doc);
            }
        }

        UnderlayedText begin;
        UnderlayedText end;
        bool is_roundtrip = false;
    };

    // Отрисовщик схемы маршрутов
    class MapRenderer {
        public:
            MapRenderer(transport::Catalogue& catalogue)
                : catalogue_(&catalogue){}
            
            MapRenderer(Settings&& settings, transport::Catalogue& catalogue)
                : settings_(std::move(settings))
                , catalogue_(&catalogue){}
            
            void SetCatalogue(transport::Catalogue& catalogue);

            void SetSettings(Settings&& settings);
            
            void RenderMap(std::ostream& output) const;

        private:
            Settings settings_;
            transport::Catalogue* catalogue_;

            svg::Polyline RenderBus(domain::Bus* bus, const SphereProjector& project) const;
            size_t NextColor(size_t prev) const;
            svg::Circle RenderStop(domain::Stop* stop, const SphereProjector& project) const;
            UnderlayedText RenderStopText(domain::Stop* stop, const SphereProjector& project) const;
            BusText RenderBusText(domain::Bus* bus, const SphereProjector& project, const svg::Color& color) const;
    };
}
