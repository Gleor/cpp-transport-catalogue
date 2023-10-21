#include "map_renderer.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <algorithm>

namespace map_renderer {

	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	// Проецирует широту и долготу в координаты внутри SVG-изображения
	svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}
	RendererSettings& RendererSettings::SetWidth(double width) {
		width_ = std::move(width);
		
		return *this;
	}

	// задать высоту отображаемого изображения
	RendererSettings& RendererSettings::SetHeight(double height) {
		height_ = std::move(height);
		
		return *this;
	}

	// задать отступ краёв карты от границ SVG-документа
	RendererSettings& RendererSettings::SetPadding(double padding) {
		padding_ = std::move(padding);
		
		return *this;
	}

	// задать толщину линий, которыми рисуются автобусные маршруты
	RendererSettings& RendererSettings::SetLineWidth(double l_width) {
		line_width_ = std::move(l_width);
		
		return *this;
	}

	// задать радиус окружностей, которыми обозначаются остановки
	RendererSettings& RendererSettings::SetStopRadius(double radius) {
		stop_radius_ = std::move(radius);
		
		return *this;
	}

	// задать размер текста написания автобусных остановок
	RendererSettings& RendererSettings::SetBusLabelFont(size_t size) {
		bus_label_font_size_ = std::move(size);
		
		return *this;
	}

	// задать смещение надписи с названием маршрута
	RendererSettings& RendererSettings::SetBusLabelOffset(svg::Point offset) {
		bus_label_offset_ = std::move(offset);
		
		return *this;
	}

	// задать размер текста написания автобусных остановок
	RendererSettings& RendererSettings::SetStopLabelFont(size_t size) {
		stop_label_font_size_ = std::move(size);
		
		return *this;
	}

	// задать смещение надписи названия остановки
	RendererSettings& RendererSettings::SetStopLabelOffset(svg::Point offset) {
		stop_label_offset_ = std::move(offset);
		
		return *this;
	}

	// задать цвет подложки под названиями
	RendererSettings& RendererSettings::SetUnderlayerColor(const svg::Color& color) {
		underlayer_color_ = color;
		
		return *this;
	}

	// задать цвет подложки под названиями
	RendererSettings& RendererSettings::SetUnderlayerColor(svg::Color&& color) {
		underlayer_color_ = std::move(color);
		
		return *this;
	}

	// задать толщину подложки под названиями
	RendererSettings& RendererSettings::SetUnderlayerWidth(double width) {
		underlayer_width_ = std::move(width);
		
		return *this;
	}

	// добавить цвет в палитру
	RendererSettings& RendererSettings::AddColorInPalette(const svg::Color& color) {
		color_palette_.push_back(color);
		
		return *this;
	}

	// добавить цвет в палитру
	RendererSettings& RendererSettings::AddColorInPalette(svg::Color&& color) {
		color_palette_.push_back(std::move(color));
		
		return *this;
	}

	// обнулить палетту цветов
	RendererSettings& RendererSettings::ResetColorPalette() {
		color_palette_.clear();
		
		return *this;
	}
	double RendererSettings::GetWight() const {
		return width_;
	}

	// получить высоту отображаемого изображения
	double RendererSettings::GetHeight() const {
		return height_;
	}

	// получить отступ краёв карты от границ SVG-документа
	double RendererSettings::GetPadding() const {
		return padding_;
	}

	// получить толщину линий отрисовки маршрутов
	double RendererSettings::GetLineWidth() const {
		return line_width_;
	}

	// получить радиус окружностей отрисовки остановок
	double RendererSettings::GetStopRadius() const {
		return stop_radius_;
	}

	// получить размер текста названий маршрутов
	size_t RendererSettings::GetBusLabelFontSize() const {
		return bus_label_font_size_;
	}
	// получить смещение надписей названий маршрутов
	svg::Point RendererSettings::GetBusLabelOffSet() const {
		return bus_label_offset_;
	}

	// получить размер текста названий остановок
	size_t RendererSettings::GetStopLabelFontSize() const {
		return stop_label_font_size_;
	}
	// получить смещение надписей названий остановок
	svg::Point RendererSettings::GetStopLabelOffSet() const {
		return stop_label_offset_;
	}

	// получить цвет подложки под названиями остановок и маршрутов
	svg::Color RendererSettings::GetUnderlaterColor() const {
		return underlayer_color_;
	}
	// получить толщину подложки под названиями остановок и маршрутов
	double RendererSettings::GetUnderlayerWidth() const {
		return underlayer_width_;
	}
	// получить ссылку на палитру цветов рендера
	const std::vector<svg::Color>& RendererSettings::GetColorPalette() const {
		return color_palette_;
	}
	MapRenderer::MapRenderer(const RendererSettings& settings)
		: settings_(settings)
	{
	}
	void MapRenderer::SetSettings(RendererSettings& settings) {
		settings_ = settings;
	}

	const RendererSettings& MapRenderer::GetRenderSettings() const {
		return settings_;
	}

	RendererSettings& MapRenderer::GetRenderSettings()
	{
		return settings_;
	}
	
	svg::Document MapRenderer::RenderMap(const domain::BusesMap& routes) {
		
		svg::Document document;

		std::vector<domain::Stop*> all_stops;
		std::vector<domain::Bus*> all_routes;

		std::vector<geo::Coordinates> stops_coords;
		svg::Document result;

		for (const auto& [bus_name, bus] : routes) {

			if (bus->stops_.size() == 0) {
				continue;
			}

			all_routes.push_back(bus);

			for (const auto& stop : bus->stops_) {
				stops_coords.push_back(stop->coordinates_);
				all_stops.push_back(stop);

			}
		}

		sort(all_stops.begin(), all_stops.end(),
			[](domain::Stop* lhs, domain::Stop* rhs) {
				return lhs->name_ < rhs->name_;
			});
		sort(all_routes.begin(), all_routes.end(),
			[](domain::Bus* lhs, domain::Bus* rhs) {
				return lhs->bus_name_ < rhs->bus_name_;
			});
		
		const auto last = std::unique(all_stops.begin(), all_stops.end());
		all_stops.erase(last, all_stops.end());

		SphereProjector projector(stops_coords.begin(), stops_coords.end(), settings_.width_, settings_.height_, settings_.padding_);
		
		std::vector<svg::Color>& colors = settings_.color_palette_;
		
		RenderBuses(all_routes, projector, colors, document);
		
		RenderStop(document, all_stops, projector);
		RenderStopLabels(document, all_stops, projector);

		return document;
	}
	void MapRenderer::RenderLine(svg::Polyline& line, svg::Color& color) {
		line.SetFillColor(svg::NoneColor)
			.SetStrokeColor(color)
			.SetStrokeWidth(settings_.line_width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	}
	void MapRenderer::RenderBuses(const std::vector<domain::Bus*>& buses, const SphereProjector& projector,
		std::vector<svg::Color>& colors, svg::Document& document) {
		
		size_t i = 0;

		svg::Color curr_color;
		std::vector<svg::Text> labels;

		for (domain::Bus* bus : buses) {
			svg::Polyline line;
			for (const domain::Stop* stop : bus->stops_) {
				line.AddPoint(projector(stop->coordinates_));
			}
			curr_color = colors[i % colors.size()];
			RenderBusLabels(labels, bus, curr_color, projector);
			RenderLine(line, curr_color);
			document.Add(std::move(line));
			++i;
		}
		for (auto& label : labels) {
			document.Add(label);
		}
	}
	void MapRenderer::RenderBusLabel(std::vector<svg::Text>& labels, const svg::Point point, const svg::Color& color, const std::string& route_name) const {
		labels.push_back(svg::Text()
			.SetData(route_name)
			.SetPosition(point)
			.SetOffset(settings_.bus_label_offset_)
			.SetFontSize(settings_.bus_label_font_size_)
			.SetFontFamily("Verdana"s)
			.SetFontWeight("bold"s)
			.SetFillColor(settings_.underlayer_color_)
			.SetStrokeColor(settings_.underlayer_color_)
			.SetStrokeWidth(settings_.underlayer_width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
		labels.push_back(svg::Text()
			.SetData(route_name)
			.SetPosition(point)
			.SetOffset(settings_.bus_label_offset_)
			.SetFontSize(settings_.bus_label_font_size_)
			.SetFontFamily("Verdana"s)
			.SetFontWeight("bold"s)
			.SetFillColor(color));
	}

	void MapRenderer::RenderBusLabels(std::vector<svg::Text>& labels,
		const domain::Bus* bus, const svg::Color& color, const SphereProjector& projector) const {
		
		size_t m = (bus->stops_.size() / 2);
		auto first_stop = bus->stops_.front();

		RenderBusLabel(labels, projector(first_stop->coordinates_), color, bus->bus_name_);
		if (!bus->is_circular_ && first_stop != bus->stops_[m]) {
			RenderBusLabel(labels, projector(bus->stops_[m]->coordinates_), color, bus->bus_name_);

		}
	}

	void MapRenderer::RenderStop(svg::Document& document, const std::vector<domain::Stop*>& stops, const SphereProjector& projector) const {
		for (const domain::Stop& stop : stops) {
			const svg::Point projected_point = projector(stop.coordinates_);
			document.Add(svg::Circle()
				.SetCenter(projected_point)
				.SetRadius(settings_.stop_radius_)
				.SetFillColor("white"s));
		}
	}

	void MapRenderer::RenderStopLabels(svg::Document& document, const std::vector<domain::Stop*>& stops, const SphereProjector& projector) const {
		for (const domain::Stop& stop : stops) {
			const svg::Point projected_point = projector(stop.coordinates_);
			document.Add(svg::Text()
				.SetPosition(projected_point)
				.SetOffset(settings_.stop_label_offset_)
				.SetFontSize(settings_.stop_label_font_size_)
				.SetFontFamily("Verdana"s)
				.SetData(stop.name_)
				.SetFillColor(settings_.underlayer_color_)
				.SetStrokeColor(settings_.underlayer_color_)
				.SetStrokeWidth(settings_.underlayer_width_)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
			document.Add(svg::Text()
				.SetPosition(projected_point)
				.SetOffset(settings_.stop_label_offset_)
				.SetFontSize(settings_.stop_label_font_size_)
				.SetFontFamily("Verdana"s)
				.SetData(stop.name_)
				.SetFillColor("black"s));
		}
	}
}