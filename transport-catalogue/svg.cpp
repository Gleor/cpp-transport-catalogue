#include "svg.h"

namespace svg {

    using namespace std::literals;

    Rgb::Rgb(uint16_t red, uint16_t green, uint16_t blue)
        : red(red), green(green), blue(blue)
    {
    }

    Rgba::Rgba(uint16_t red, uint16_t green, uint16_t blue, double alpha)
        : red(red), green(green), blue(blue), alpha(alpha)
    {
    }

    std::ostream& operator<<(std::ostream& out, const Rgb& rgb)
    {   
        out << "rgb"sv << "("sv << rgb.red << ","sv
            << rgb.green << ","sv << rgb.blue << ")"sv;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Rgba& rgba)
    {
        out << "rgba"sv << "("sv << rgba.red << ","sv << rgba.green
            << ","sv << rgba.blue << ","sv << rgba.alpha << ")"sv;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color)
    {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    void ColorPrinter::operator()(std::monostate) const {
        out << "none"sv;
    }

    void ColorPrinter::operator()(std::string str) const {
        out << str;
    }

    void ColorPrinter::operator()(Rgb rgb) const {
        out << rgb;
    }

    void ColorPrinter::operator()(Rgba rgba) const {
        out << rgba;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        out << line_cap_words.at(line_cap);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        out << line_join_words.at(line_join);
        return out;
    }

    bool operator==(const Point& lhs, const Point& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    bool operator!=(const Point& lhs, const Point& rhs)
    {
        return !(lhs == rhs);
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool IsFirst = true;
        for (auto& point : points_) {
            if (IsFirst) {
                out << point.x << ","sv << point.y;
                IsFirst = false;
            }
            else {
                out << " "sv << point.x << ","sv << point.y;
            }
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }
    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }
    std::string Text::TextOutputPreProccess(const std::string& text) const {
        std::string out_text;
        for (char c : text) {
            auto iter = escape_characters_.find(c);
            if (iter != escape_characters_.end()) {
                out_text += iter->second;
            }
            else {
                out_text += c;
            }
        }
        return out_text;
    }
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv << TextOutputPreProccess(data_) << "</text>"sv;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }
    void Document::Render(std::ostream& out) const {
        RenderContext context_ = RenderContext(out, 2, 2);

        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        context_.indent += context_.indent_step;
        for (const auto& obj : objects_) {
            obj->Render(context_);
        }
        context_.indent -= context_.indent_step;
        out << "</svg>"sv;
    }

}  // namespace svg