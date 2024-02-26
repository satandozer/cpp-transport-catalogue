#include "svg.h"
#include <iostream>

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context .out;
    out << "<polyline points=\""sv;
    bool first = true;
    for (Point p : points_) {
        if (first) {
            out << p.x << ","sv << p.y;
            first = false;
            continue;
        }
        out << " "sv << p.x << ","sv << p.y;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    contents_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context .out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv 
        << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv
        << " font-size=\""sv << size_ << "\""sv;

    if (!font_.empty()) {
        out << " font-family=\""sv << font_ << "\""sv;
    }
    if (!weight_.empty()) {
        out << " font-weight=\"" << weight_ << "\"";
    }
    
    out << ">";

    for (char c : contents_){
        switch (c) {
        case '\"' :
            out << "&quot;";
            break;
        case '\'' :
            out << "&apos;";
            break;
        case '<' :
            out << "&lt;";
            break;
        case '>' :
            out << "&gt;";
            break;
        case '&' :
            out << "&amp;";
            break;
        default:
            out << c ;
            break;
        }
    }

    out << "</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext context (out);
    for (auto& object : objects_) {
        out << "  ";
        object->Render(context);
    }
    out << "</svg>"sv << std::endl;
}

}  // namespace svg