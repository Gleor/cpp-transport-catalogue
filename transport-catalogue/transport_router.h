
#pragma once
#include "router.h"                                                          
#include "transport_catalogue.h"                                               

#include <vector>
#include <memory>
#include <unordered_map>
#include <string_view>
#include <future>

namespace transport_catalogue {

	namespace router {

		constexpr static double VELOCITY_COEF = 1000.0 / 60.0;

		struct RouterSettings {

			RouterSettings() = default;
			RouterSettings(size_t, double);

			RouterSettings& SetBusWaitTime(size_t);
			RouterSettings& SetBusVelocity(double);

			size_t GetBusWaitTime() const;
			double GetBusVelocity() const;

			size_t _bus_wait_time = {};
			double _bus_velocity = {};
		};

		class TransportRouter {
		private:
			transport_catalogue::TransportCatalogue& transport_catalogue_;
		public:
			TransportRouter() = default;
			TransportRouter(transport_catalogue::TransportCatalogue&);
			TransportRouter(transport_catalogue::TransportCatalogue&, const RouterSettings&);
			TransportRouter(transport_catalogue::TransportCatalogue&, RouterSettings&&);

			TransportRouter& SetRouterSettings(const RouterSettings&);
			TransportRouter& SetRouterSettings(RouterSettings&&);
			TransportRouter& SetRouterTransportCatalogue(transport_catalogue::TransportCatalogue&);

			TransportRouter& SetRouterGraphs(graph::DirectedWeightedGraph<double>&&);
			TransportRouter& SetRouterWaitPoints(std::unordered_map<std::string_view, size_t>&&);
			TransportRouter& SetRouterMovePoints(std::unordered_map<std::string_view, size_t>&&);

			const graph::DirectedWeightedGraph<double>& GetRouterGraphs() const;
			const std::unordered_map<std::string_view, size_t>& GetRouterWaitPoints() const;
			const std::unordered_map<std::string_view, size_t>& GetRouterMovePoints() const;

			TransportRouter& ImportRoutingDataFromCatalogue();

			template <typename Iterator>
			void BuidEdgeTask(Iterator first, Iterator last);

			transport_catalogue::RouteStat MakeRoute(std::string_view, std::string_view);

		private:
			RouterSettings _settings;

			graph::DirectedWeightedGraph<double> graphs_;
			std::unique_ptr<graph::Router<double>> _router = nullptr;
			std::unordered_map<std::string_view, size_t> wait_points_;
			std::unordered_map<std::string_view, size_t> move_points_;

		};

		template<typename Iterator>
		inline void TransportRouter::BuidEdgeTask(Iterator first, Iterator last)
		{
			for (; first != last; ++first) {

				for (size_t from_stop_id = 0; from_stop_id != first->stops_.size(); ++from_stop_id) {

					int span_count = 0;

					for (size_t to_stop_id = from_stop_id + 1; to_stop_id != first->stops_.size(); ++to_stop_id) {

						double route_distance = 0.0;

						for (size_t current_point = from_stop_id + 1; current_point <= to_stop_id; ++current_point) {
							route_distance += static_cast<double>(transport_catalogue_
								.GetDistance(first->stops_[current_point - 1], first->stops_[current_point]));
						}

						graphs_.AddEdge(graph::Edge<double>()
							.SetEdgeType(graph::EdgeType::move)
							.SetVertexFromId(move_points_.at(first->stops_[from_stop_id]->name_))
							.SetVertexToId(wait_points_.at(first->stops_[to_stop_id]->name_))
							.SetEdgeWeight(route_distance / (_settings.GetBusVelocity() * VELOCITY_COEF))
							.SetEdgeName(first->bus_name_)
							.SetEdgeSpanCount(++span_count));
					}

				}

			}
		}

	}

}