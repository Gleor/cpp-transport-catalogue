#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <iostream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>


constexpr double EPSILON = 1e-6;

namespace map_renderer {

    bool IsZero(double value);

    class SphereProjector {
    public:
        SphereProjector() = default;

        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_ = 0.0;
        double min_lon_ = 0.0;
        double max_lat_ = 0.0;
        double zoom_coeff_ = 0.0;
    };

    struct RendererSettings {

        RendererSettings() = default;

        double width_ = 1200.0;
        double height_ = 1200.0;

        double padding_ = 50.0;
        double line_width_ = 14.0;
        double stop_radius_ = 5.0;

        size_t bus_label_font_size_ = 20;
        svg::Point bus_label_offset_ = { 7.0, 15.0 };

        size_t stop_label_font_size_ = 20;
        svg::Point stop_label_offset_ = { 7.0, -3.0 };

        svg::Color underlayer_color_ = svg::Rgba{ 255, 255, 255, 0.85 };
        double underlayer_width_ = 3.0;
        std::vector<svg::Color> color_palette_ = { std::string("green"), svg::Rgb{255, 160, 0}, std::string("red") };

        bool default_settings_ = true;
    };

    class MapRenderer final
    {
    public:
        explicit MapRenderer() = default;

        void SetSettings(RendererSettings&& settings);

        const RendererSettings& GetRenderSettings() const;

        svg::Document RenderMap(domain::RoutesMap& routes);
        
        struct RouteCompareName {
            bool operator()(const domain::Route* lhs, const domain::Route* rhs) const {
                return lhs->route_name_ < rhs->route_name_;
            }
        };

        using RoutesPoints = std::map<domain::Route*, std::vector<svg::Point>, RouteCompareName>;

    private:

        RendererSettings settings_;
        
        void ProjectRoutePoints(const std::vector<domain::Route*>& buses, RoutesPoints& projected_points, const SphereProjector& projector) const;

        void RenderRoutes(svg::Document& document, const RoutesPoints& projected_points, std::vector<svg::Color>& colors) const;
        /*
        void RenderRouteLabels(svg::Document& document, const RoutesPoints& projected_points, const std::vector<svg::Color>& colors) const;

        void RenderRouteLabel(svg::Document& document, const svg::Point point, const svg::Color& color, const std::string& route_name) const;
        */
        void RenderRouteLabels(std::vector<svg::Text>& labels,
            const domain::Route* bus, const svg::Color& color, const SphereProjector& projector) const;

        void RenderRouteLabel(std::vector<svg::Text>& labels, const svg::Point point, const svg::Color& color, const std::string& route_name) const;


        void RenderStop(svg::Document& document, const std::vector<domain::Stop*>& stops, const SphereProjector& projector) const;

        void RenderStopLabels(svg::Document& document, const std::vector<domain::Stop*>& stops, const SphereProjector& projector) const;
        void RenderLine(svg::Polyline& line, svg::Color& color);
        void RenderBuses(const std::vector<domain::Route*>& buses, const SphereProjector& projector,
            std::vector<svg::Color>& colors, svg::Document& document);
        
    };

    template <typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
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
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

}