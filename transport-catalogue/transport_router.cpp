#include "transport_router.h"

namespace transport_catalogue {

	namespace router {

		RouterSettings::RouterSettings(size_t time, double velocity)
			: _bus_wait_time(time), _bus_velocity(velocity) {
		}

		RouterSettings& RouterSettings::SetBusWaitTime(size_t time) {
			_bus_wait_time = time;
			return *this;
		}
		RouterSettings& RouterSettings::SetBusVelocity(double velocity) {
			_bus_velocity = velocity;
			return *this;
		}

		size_t RouterSettings::GetBusWaitTime() const {
			return _bus_wait_time;
		}
		double RouterSettings::GetBusVelocity() const {
			return _bus_velocity;
		}

		TransportRouter::TransportRouter(transport_catalogue::TransportCatalogue& tc)
			: transport_catalogue_(tc), graphs_(tc.GetStopsCount() * 2) {
		}

		TransportRouter::TransportRouter(transport_catalogue::TransportCatalogue& tc, const RouterSettings& settings)
			: transport_catalogue_(tc), _settings(settings), graphs_(tc.GetStopsCount() * 2) {
		}

		TransportRouter::TransportRouter(transport_catalogue::TransportCatalogue& tc, RouterSettings&& settings)
			: transport_catalogue_(tc), _settings(std::move(settings)), graphs_(tc.GetStopsCount() * 2) {
		}

		TransportRouter& TransportRouter::SetRouterSettings(const RouterSettings& settings) {
			_settings = settings;
			return *this;
		}

		TransportRouter& TransportRouter::SetRouterSettings(RouterSettings&& settings) {
			_settings = std::move(settings);
			return *this;
		}

		TransportRouter& TransportRouter::SetRouterTransportCatalogue(transport_catalogue::TransportCatalogue& tc) {
			transport_catalogue_ = tc;
			return *this;
		}

		TransportRouter& TransportRouter::SetRouterGraphs(graph::DirectedWeightedGraph<double>&& graphs) {
			graphs_ = std::move(graphs);
			return *this;
		}

		TransportRouter& TransportRouter::SetRouterWaitPoints(std::unordered_map<std::string_view, size_t>&& wait_points) {
			wait_points_ = std::move(wait_points);
			return *this;
		}

		TransportRouter& TransportRouter::SetRouterMovePoints(std::unordered_map<std::string_view, size_t>&& move_points) {
			move_points_ = std::move(move_points);
			return *this;
		}

		const graph::DirectedWeightedGraph<double>& TransportRouter::GetRouterGraphs() const {
			return graphs_;
		}

		const std::unordered_map<std::string_view, size_t>& TransportRouter::GetRouterWaitPoints() const {
			return wait_points_;
		}

		const std::unordered_map<std::string_view, size_t>& TransportRouter::GetRouterMovePoints() const {
			return move_points_;
		}

		transport_catalogue::RouteStat TransportRouter::MakeRoute(std::string_view from, std::string_view to) {
			if (!_router) {
				ImportRoutingDataFromCatalogue();
			}

			transport_catalogue::RouteStat result;
			if (wait_points_.count(from) && move_points_.count(to))
			{
				auto data = _router.get()->BuildRoute(
					wait_points_.at(from), move_points_.at(to));

				if (data.has_value())    
				{
					result.is_found_ = true;
					for (auto& item_id : data.value().edges) {
						const auto& edge = graphs_.GetEdge(item_id);
						result.route_items_.push_back(transport_catalogue::RouteItem()
							.SetName(edge.GetEdgeName())
							.SetEdgeType(edge.GetEdgeType())
							.SetTime(edge.GetEdgeWeight())
							.SetSpanCount(edge.GetEdgeSpanCount()));

						result.total_time_ += edge.weight;
					}
				}
			}
			return result;
		}

		TransportRouter& TransportRouter::ImportRoutingDataFromCatalogue() {

			size_t points_counter = 0;
			for (const auto& stop : transport_catalogue_.GetAllStopsData()) {

				wait_points_.insert({ stop->name_, points_counter });
				move_points_.insert({ stop->name_, ++points_counter });

				graphs_.AddEdge(graph::Edge<double>()
					.SetEdgeType(graph::EdgeType::wait)
					.SetVertexFromId(wait_points_.at(stop->name_))
					.SetVertexToId(move_points_.at(stop->name_))
					.SetEdgeWeight(static_cast<double>(_settings.GetBusWaitTime()))
					.SetEdgeName(stop->name_)
					.SetEdgeSpanCount(0));
				++points_counter;
			}

			for (const auto& bus : transport_catalogue_.GetAllBusesData()) {

				// движемся по маршруту от первой остановки
				for (size_t from_stop_id = 0; from_stop_id != bus->stops_.size(); ++from_stop_id) {

					int span_count = 0;

					// до каждой из последующих остановок
					for (size_t to_stop_id = from_stop_id + 1; to_stop_id != bus->stops_.size(); ++to_stop_id) {

						double route_distance = 0.0;

						// считаем расстояния по тем же точкам
						for (size_t current_point = from_stop_id + 1; current_point <= to_stop_id; ++current_point) {
							route_distance += static_cast<double>(transport_catalogue_
								.GetDistance(bus->stops_[current_point - 1], bus->stops_[current_point]));
						}

						graphs_.AddEdge(graph::Edge<double>()
							.SetEdgeType(graph::EdgeType::move)
							.SetVertexFromId(move_points_.at(bus->stops_[from_stop_id]->name_))
							.SetVertexToId(wait_points_.at(bus->stops_[to_stop_id]->name_))
							.SetEdgeWeight(route_distance / (_settings.GetBusVelocity() * VELOCITY_COEF))
							.SetEdgeName(bus->bus_name_)
							.SetEdgeSpanCount(++span_count));
					}

				}

			}
			_router = std::make_unique<graph::Router<double>>(graphs_);
			return *this;
		}

	}   

}   