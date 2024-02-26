#pragma once

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

struct Rgb {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;    
};

using Color = std::variant<std::monostate,Rgb,Rgba,std::string>;

struct ColorPrinter {
    std::ostream& out;

    void operator()(std::monostate)const {
        using namespace std::literals;
        out << "none"sv;
    } 
    void operator()(Rgb rgb) const{
        out << "rgb(" << static_cast<int>(rgb.red) << ',' 
                        << static_cast<int>(rgb.green) << ',' 
                        << static_cast<int>(rgb.blue) << ')';
    }
    void operator()(Rgba rgba) const{
        out << "rgba(" << static_cast<int>(rgba.red) << ',' 
                        << static_cast<int>(rgba.green) << ',' 
                        << static_cast<int>(rgba.blue) << ',' 
                        << rgba.opacity << ')';
    }
    void operator()(const std::string& color) const {
        out << color;
    }
};

inline std::ostream& operator<<(std::ostream& out, const Color& color) {
    visit(ColorPrinter{out},color);
    return out;
}

// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor{"none"};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline std::ostream& operator<<(std::ostream& out, StrokeLineCap linecap) {
    switch (linecap){
        case (StrokeLineCap::BUTT):
            out << "butt";
            break;
        case (StrokeLineCap::ROUND):
            out << "round";
            break;
        case (StrokeLineCap::SQUARE):
            out << "square";
            break;
    }
    return out;
}

inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin linejoin) {
    switch (linejoin){
        case (StrokeLineJoin::ARCS):
            out << "arcs";
            break;
        case (StrokeLineJoin::BEVEL):
            out << "bevel";
            break;
        case (StrokeLineJoin::MITER):
            out << "miter";
            break;
        case (StrokeLineJoin::MITER_CLIP):
            out << "miter-clip";
            break;
        case (StrokeLineJoin::ROUND):
            out << "round";
            break;
    }
    return out;
}

//Вспомогательный класс для задания цвета и формы для заливки и обводки фигур
template <typename Owner>
class PathProps {
public:
    //Цвет заливки
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    //Цвет обводки
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    //Ширина обводки
    Owner& SetStrokeWidth(const double& width){
        stroke_width_ = width;
        return AsOwner();
    }

    //Форма конца линии
    Owner& SetStrokeLineCap (StrokeLineCap linecap){
        linecap_ = linecap;
        return AsOwner();
    }

    //Форма соединения линий
    Owner& SetStrokeLineJoin (StrokeLineJoin linejoin){
        linejoin_ = linejoin;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;
        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (linecap_) {
            out << " stroke-linecap=\""sv << *linecap_ << "\""sv;
        }
        if (linejoin_) {
            out << " stroke-linejoin=\""sv << *linejoin_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> linecap_;
    std::optional<StrokeLineJoin> linejoin_;

};

struct Point {
    Point() = default;
    Point(double x_, double y_)
        : x(x_)
        , y(y_) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out_)
        : out(out_) {
    }

    RenderContext(std::ostream& out_, int indent_step_, int indent_ = 0)
        : out(out_)
        , indent_step(indent_step_)
        , indent(indent_) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle() = default;

    Circle& SetCenter(Point center);

    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_ = Point{0.0,0.0};
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline() = default;

    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_ = {};
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    std::string contents_ = "";
    Point position_ = Point{0.0,0.0};
    Point offset_ = Point{0.0,0.0};
    uint32_t size_ = 1;
    std::string weight_;
    std::string font_;
};

class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj object) {
        objects_.emplace_back(std::make_unique<Obj>(std::move(object)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    std::vector<std::unique_ptr<Object>> objects_ = {};
};

class Drawable {
public:

    virtual ~Drawable() = default;

    virtual void Draw(svg::ObjectContainer& container) const = 0;
};

class Document : public ObjectContainer {
public:

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj);

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
};

}  // namespace svg