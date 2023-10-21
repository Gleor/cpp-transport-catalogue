#include "serialization.h"

namespace transport_catalogue {

	namespace serialize {

		Serializator::Serializator(transport_catalogue::TransportCatalogue& transport_catalogue
			, router::RouterSettings& router_settings, map_renderer::RendererSettings& renderer_settings)
			: transport_catalogue_(transport_catalogue)
			, router_settings_(router_settings)
			, renderer_settings_(renderer_settings) {
		}

		Serializator& Serializator::SetTransporCatalogue
		(transport_catalogue::TransportCatalogue& catalogue) {
			transport_catalogue_ = catalogue;
			return *this;
		}

		Serializator& Serializator::SetRendererSettings
		(map_renderer::RendererSettings& settings) {
			renderer_settings_ = settings;
			return *this;
		}

		Serializator& Serializator::SetRouterSettings
		(router::RouterSettings& settings) {
			router_settings_ = settings;
			return *this;
		}

		Serializator& Serializator::SetRouter(std::shared_ptr<router::TransportRouter> router) {
			transport_router_ = router;
			return *this;
		}

		Serializator& Serializator::GetDataFromCatalogue() {

			Serializator::SerializeStopsData();
			Serializator::SerializeDistancesData();
			Serializator::SerializeBusesData();
			Serializator::SerializeRendererSettings();
			Serializator::SerializeRouterSettings();

			if (transport_router_) {
				Serializator::SerializeRouterData();
			}
			return *this;
		}

		Serializator& Serializator::GetDataFromRouter() {

			Serializator::SerializeRouterData();
			return *this;
		}

		Serializator& Serializator::ApplyDataToCatalogue() {

			Serializator::DeserializeStopsData();
			Serializator::DeserializeDistancesData();
			Serializator::DeserializeBusesData();
			Serializator::DeserializeRendererSettings();
			Serializator::DeserializeRouterSettings();

			return *this;
		}

		Serializator& Serializator::ApplyDataToRouter() {
			bool router = Serializator::DeserializeRouterData(); assert(router);

			return *this;
		}

		bool Serializator::Serialize(std::ostream& output) {

			return serialization_data_.SerializeToOstream(&output);

		}

		bool Serializator::Deserialize(std::istream& input) {

			serialization_data_.Clear();                                                  

			serialization_data_.ParseFromIstream(&input);

			return true;
		}

		void Serializator::SerializeColor(const svg::Color& source_color,
			transport_catalogue_serialize::Color* serial_color) {

			if (std::holds_alternative<svg::Rgb>(source_color)) {

				serial_color->set_is_rgb(true);
				serial_color->set_is_rgba(false);
				serial_color->set_is_title(false);

				svg::Rgb color = std::get<svg::Rgb>(source_color);

				serial_color->clear_color_label();
				serial_color->set_color_component_r(color.red);
				serial_color->set_color_component_g(color.green);
				serial_color->set_color_component_b(color.blue);
				serial_color->clear_color_component_a();
			}
			else if (std::holds_alternative<svg::Rgba>(source_color)) {

				serial_color->set_is_rgb(false);
				serial_color->set_is_rgba(true);
				serial_color->set_is_title(false);

				svg::Rgba color = std::get<svg::Rgba>(source_color);

				serial_color->clear_color_label();
				serial_color->set_color_component_r(color.red);
				serial_color->set_color_component_g(color.green);
				serial_color->set_color_component_b(color.blue);
				serial_color->set_color_component_a(color.alpha);
			}
			else if (std::holds_alternative<std::string>(source_color)) {

				serial_color->set_is_rgb(false);
				serial_color->set_is_rgba(false);
				serial_color->set_is_title(true);

				std::string color = std::get<std::string>(source_color);

				serial_color->set_color_label(color);
				serial_color->clear_color_component_r();
				serial_color->clear_color_component_g();
				serial_color->clear_color_component_b();
				serial_color->clear_color_component_a();
			}
		}
		bool Serializator::SerializeGraphs(transport_catalogue_serialize::RouterData* serial_router_data) {

			const auto& source_graphs = transport_router_->GetRouterGraphs();

			for (size_t i = 0; i != source_graphs.GetEdgeCount(); ++i) {
				auto serial_edge = serial_router_data->add_router_edges();

				(source_graphs.GetEdge(i).GetEdgeType() == graph::wait) ? serial_edge->set_edge_type("wait") : serial_edge->set_edge_type("move");

				serial_edge->set_edge_from(source_graphs.GetEdge(i).GetVertexFromId());
				serial_edge->set_edge_to(source_graphs.GetEdge(i).GetVertexToId());

				serial_edge->set_edge_weight(source_graphs.GetEdge(i).GetEdgeWeight());
				serial_edge->set_edge_name(std::string(source_graphs.GetEdge(i).GetEdgeName()));
				serial_edge->set_span_count(source_graphs.GetEdge(i).GetEdgeSpanCount());

			}
			return true;
		}
		bool Serializator::SerializeStopsData() {

			serialization_data_.clear_stops_data();

			for (auto& source_stop : transport_catalogue_.GetAllStopsData())
			{
				auto serial_stop = serialization_data_.add_stops_data();
				serial_stop->set_stop_name(source_stop->GetStopName());

				auto serial_stop_coords = serial_stop->mutable_stop_coordinates();
				serial_stop_coords->set_latitude(source_stop->GetStopCoordinates().GetLatitude());
				serial_stop_coords->set_longitude(source_stop->GetStopCoordinates().GetLongitude());
			}
			return true;
		}
		bool Serializator::SerializeBusesData() {

			serialization_data_.clear_buses_data();

			for (auto source_bus : transport_catalogue_.GetAllBusesData())
			{
				auto serial_bus = serialization_data_.add_buses_data();
				serial_bus->set_bus_name(source_bus->GetBusName());

				auto serial_bus_stops = serial_bus->mutable_bus_stops();
				for (auto stop : source_bus->GetStops()) {
					serial_bus_stops->Add(std::move(std::string(stop->GetStopName())));
				}

				serial_bus->set_unique_stops_qty(source_bus->GetUniqueStops());

				serial_bus->set_geo_route_length(source_bus->GetGeoRouteLength());

				serial_bus->set_real_route_length(source_bus->GetRealRouteLength());

				serial_bus->set_curvature(source_bus->GetCurvature());

				serial_bus->set_is_circular(source_bus->GetBusType());
			}
			return true;
		}
		bool Serializator::SerializeDistancesData() {

			serialization_data_.clear_distances_data();

			for (auto& source_distance : transport_catalogue_.GetStopDistancesRef())
			{
				auto serial_distance = serialization_data_.add_distances_data();

				serial_distance->set_from(source_distance.first.first->name_);
				serial_distance->set_to(source_distance.first.second->name_);
				serial_distance->set_range(source_distance.second);
			}
			return true;
		}
		bool Serializator::SerializeRendererSettings() {

			serialization_data_.clear_renderer_settings();

			auto serial_renderer_settings = serialization_data_.mutable_renderer_settings();

			serial_renderer_settings->set_width(renderer_settings_.GetWight());
			serial_renderer_settings->set_height(renderer_settings_.GetHeight());

			serial_renderer_settings->set_padding(renderer_settings_.GetPadding());
			serial_renderer_settings->set_line_width(renderer_settings_.GetLineWidth());
			serial_renderer_settings->set_stop_radius(renderer_settings_.GetStopRadius());

			serial_renderer_settings->set_bus_label_font_size(renderer_settings_.GetBusLabelFontSize());
			serial_renderer_settings->set_bus_label_offset_x(renderer_settings_.GetBusLabelOffSet().x);
			serial_renderer_settings->set_bus_label_offset_y(renderer_settings_.GetBusLabelOffSet().y);

			serial_renderer_settings->set_stop_label_font_size(renderer_settings_.GetStopLabelFontSize());
			serial_renderer_settings->set_stop_label_offset_x(renderer_settings_.GetStopLabelOffSet().x);
			serial_renderer_settings->set_stop_label_offset_y(renderer_settings_.GetStopLabelOffSet().y);

			SerializeColor(renderer_settings_.GetUnderlaterColor(), serial_renderer_settings->mutable_underlayer_color());
			serial_renderer_settings->set_underlayer_width(renderer_settings_.GetUnderlayerWidth());

			serial_renderer_settings->clear_color_palette();
			for (const svg::Color& color : renderer_settings_.GetColorPalette()) {
				SerializeColor(color, serial_renderer_settings->add_color_palette());
			}
			return true;
		}
		bool Serializator::SerializeRouterSettings() {
			serialization_data_.clear_router_settings();

			auto serial_router_settings = serialization_data_.mutable_router_settings();

			serial_router_settings->set_bus_wait_time(router_settings_.GetBusWaitTime());
			serial_router_settings->set_bus_velocity(router_settings_.GetBusVelocity());
			return true;
		}
		bool Serializator::SerializeRouterData() {

			if (transport_router_)
			{
				bool graphs = false;

				serialization_data_.clear_router_data();
				auto serial_router_data = serialization_data_.mutable_router_data();

				serial_router_data->set_vertex_count(transport_router_->GetRouterGraphs().GetVertexCount());

				graphs = SerializeGraphs(serial_router_data);

				for (const auto& item : transport_router_->GetRouterWaitPoints()) {
					auto wait_point = serial_router_data->add_router_wait_points();

					wait_point->set_wait_point_name(std::string(item.first));
					wait_point->set_wait_point_id(item.second);
				}

				for (const auto& item : transport_router_->GetRouterMovePoints()) {
					auto move_point = serial_router_data->add_router_move_points();

					move_point->set_move_point_name(std::string(item.first));
					move_point->set_move_point_id(item.second);
				}

				if (graphs) return true;
			}

			return false;
		}

		svg::Color Serializator::DeseserializeColor(
			const transport_catalogue_serialize::Color& serial_color) {

			svg::Color result;

			if (serial_color.is_rgb()) {
				result = std::move(svg::Rgb(
					serial_color.color_component_r(),
					serial_color.color_component_g(),
					serial_color.color_component_b()));
			}
			else if (serial_color.is_rgba()) {
				result = std::move(svg::Rgba(
					serial_color.color_component_r(),
					serial_color.color_component_g(),
					serial_color.color_component_b(),
					serial_color.color_component_a()));
			}
			else if (serial_color.is_title()) {
				result = std::move(std::string(serial_color.color_label()));
			}

			return std::move(result);
		}
		bool Serializator::DeserializeStopsData() {

			for (const auto& stop : serialization_data_.stops_data()) {

				transport_catalogue_.AddStop(std::move(
					transport_catalogue::Stop()
					.SetStopName(stop.stop_name())
					.SetStopCoordinates({
						stop.stop_coordinates().latitude(),
						stop.stop_coordinates().longitude() })
						));
			}
			return true;
		}
		bool Serializator::DeserializeBusesData() {
			for (const auto& bus : serialization_data_.buses_data()) {

				std::vector<Stop*> bus_stops;
				for (const auto& stop : bus.bus_stops()) {
					bus_stops.push_back(transport_catalogue_.FindStopByName(stop));
				}

				transport_catalogue_.AddRouteFromSerializer(std::move(
					transport_catalogue::Bus()
					.SetBusName(bus.bus_name())
					.SetStops(std::move(bus_stops))
					.SetUniqueStops(bus.unique_stops_qty())
					.SetGeoRouteLength(bus.geo_route_length())
					.SetRealRouteLength(bus.real_route_length())
					.SetCurvature(bus.curvature())
					.SetBusType(bus.is_circular())
				));
			}

			return true;
		}
		bool Serializator::DeserializeDistancesData() {

			for (const auto& distance : serialization_data_.distances_data()) {

				transport_catalogue_.AddStopsDistance(
					transport_catalogue_.FindStopByName(distance.from()),
					transport_catalogue_.FindStopByName(distance.to()), distance.range());
			}

			return true;
		}
		bool Serializator::DeserializeRendererSettings() {

			if (serialization_data_.has_renderer_settings())
			{
				auto& serial_renderer_settings = serialization_data_.renderer_settings();

				renderer_settings_.SetWidth(serial_renderer_settings.width());
				renderer_settings_.SetHeight(serial_renderer_settings.height());

				renderer_settings_.SetPadding(serial_renderer_settings.padding());
				renderer_settings_.SetLineWidth(serial_renderer_settings.line_width());
				renderer_settings_.SetStopRadius(serial_renderer_settings.stop_radius());

				renderer_settings_.SetBusLabelFont(serial_renderer_settings.bus_label_font_size());
				renderer_settings_.SetBusLabelOffset(
					{ serial_renderer_settings.bus_label_offset_x(), serial_renderer_settings.bus_label_offset_y() });

				renderer_settings_.SetStopLabelFont(serial_renderer_settings.stop_label_font_size());
				renderer_settings_.SetStopLabelOffset(
					{ serial_renderer_settings.stop_label_offset_x(), serial_renderer_settings.stop_label_offset_y() });

				renderer_settings_.SetUnderlayerColor(
					std::move(DeseserializeColor(serial_renderer_settings.underlayer_color())));

				renderer_settings_.SetUnderlayerWidth(serial_renderer_settings.underlayer_width());

				renderer_settings_.ResetColorPalette();
				for (const auto& color : serial_renderer_settings.color_palette()) {
					renderer_settings_.AddColorInPalette(std::move(DeseserializeColor(color)));
				}

				serialization_data_.clear_renderer_settings();

				return true;
			}

			return true;
		}
		bool Serializator::DeserializeRouterSettings() {

			if (serialization_data_.has_router_settings()) {

				auto& serial_router_settings = serialization_data_.router_settings();

				router_settings_.SetBusWaitTime(serial_router_settings.bus_wait_time());
				router_settings_.SetBusVelocity(serial_router_settings.bus_velocity());

				serialization_data_.clear_router_settings();
				return true;

			}

			return true;
		}
		bool Serializator::DeserializeRouterData() {

			if (serialization_data_.has_router_data()) {

				auto& serial_router_data = serialization_data_.router_data();

				graph::DirectedWeightedGraph<double> graphs(serial_router_data.vertex_count());

				auto& edges = serial_router_data.router_edges();
				for (int i = 0; i != edges.size(); ++i) {
					graphs.AddEdge(graph::Edge<double>()
						.SetEdgeType(
							(edges[i].edge_type() == "wait") ? graph::wait : graph::move)
						.SetVertexFromId(edges[i].edge_from())
						.SetVertexToId(edges[i].edge_to())
						.SetEdgeWeight(edges[i].edge_weight())
						.SetEdgeName(std::string(edges[i].edge_name()))
						.SetEdgeSpanCount(edges[i].span_count()));
				}

				std::unordered_map<std::string_view, size_t> wait_points;
				auto& wait_point = serial_router_data.router_wait_points();
				for (int i = 0; i != wait_point.size(); ++i) {
					wait_points[transport_catalogue_
						.FindStopByName(wait_point[i]
							.wait_point_name())->name_] = wait_point[i].wait_point_id();
				}

				std::unordered_map<std::string_view, size_t> move_points;
				auto& move_point = serial_router_data.router_move_points();
				for (int i = 0; i != move_point.size(); ++i) {
					move_points[transport_catalogue_
						.FindStopByName(move_point[i]
							.move_point_name())->name_] = move_point[i].move_point_id();
				}

				transport_router_->SetRouterGraphs(std::move(graphs));
				transport_router_->SetRouterWaitPoints(std::move(wait_points));
				transport_router_->SetRouterMovePoints(std::move(move_points));
				serialization_data_.clear_router_data();
				return true;

			}

			return true;
		}

	}   

}   