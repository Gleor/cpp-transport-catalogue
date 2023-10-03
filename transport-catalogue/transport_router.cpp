#include "transport_router.h"

namespace transport_catalogue {

    namespace router {

        bool operator<(const RouteWeight& left, const RouteWeight& right) {
            return left.total_time < right.total_time;
        }

        RouteWeight operator+(const RouteWeight& left, const RouteWeight& right) {
            RouteWeight result;
            result.total_time = left.total_time + right.total_time;
            return result;
        }

        bool operator>(const RouteWeight& left, const RouteWeight& right) {
            return left.total_time > right.total_time;
        }

        TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue,
            const RouterSettings& settings)
            : catalogue_(catalogue), settings_(settings) {
        }

        void TransportRouter::InitializeRouter() {

            if (!is_initialized_) {
                graph::DirectedWeightedGraph<RouteWeight>graph(ProcessStops());
                graph_ = std::move(graph);

                BuildRouterGraph();

                router_ = std::make_unique<graph::Router<RouteWeight>>(graph_);
                is_initialized_ = true;
            }
        }

        void TransportRouter::SetInitialized() {
            is_initialized_ = true;
        }

        const TransportRouter::RouterSettings& TransportRouter::GetSettings() const {
            return settings_;
        }

        TransportRouter::RouterSettings& TransportRouter::GetSettings() {
            return settings_;
        }

        domain::RouteStat TransportRouter::FindRoute(const std::string_view from,
            const std::string_view to) {
            
            InitializeRouter();

            domain::RouteStat result;

            auto from_id = stop_name_to_id_.at(from);
            auto to_id = stop_name_to_id_.at(to);
            auto route = router_->BuildRoute(from_id, to_id);
            if (!route) {
                return result;
            }

            result.is_found_ = true;
            result.wait_time_ = settings_.wait_time;
            double total_time = 0;

            for (auto edge_id : route->edges) {
                const auto& edge = graph_.GetEdge(edge_id);
                result.route_items_.push_back(transport_catalogue::RouteItem()
                    .SetName(edge.weight.bus_name)
                    .SetStopFrom(id_to_stop_.at(edge.from)->name_)
                    .SetStopTo(id_to_stop_.at(edge.to)->name_)
                    .SetTime(edge.weight.total_time - settings_.wait_time)
                    .SetSpanCount(edge.weight.span_count)
                );
                total_time += edge.weight.total_time;
            }
            result.total_time_ = total_time;
            return result;
        }

        void TransportRouter::BuildRouterGraph() {

            for (const auto& [route_name, route] : catalogue_.GetBusesMap()) {
                int stops_count = static_cast<int>(route->stops_.size());

                for (int from_stop_id = 0; from_stop_id < stops_count - 1; ++from_stop_id) {

                    double route_time = settings_.wait_time;
                    double route_time_reverse = settings_.wait_time;
                    for (int to_stop_id = from_stop_id + 1; to_stop_id < stops_count; ++to_stop_id) {
                        graph::Edge<RouteWeight> edge = BuildEdge(route, from_stop_id, to_stop_id);
                        route_time += ComputeRouteTime(route, to_stop_id - 1, to_stop_id);
                        edge.weight.total_time = route_time;
                        graph_.AddEdge(edge);

                        if (!route->is_circular_) {
                            int i_back = stops_count - 1 - from_stop_id;
                            int j_back = stops_count - 1 - to_stop_id;
                            graph::Edge<RouteWeight> edge = BuildEdge(route, i_back, j_back);
                            route_time_reverse += ComputeRouteTime(route, j_back + 1, j_back);
                            edge.weight.total_time = route_time_reverse;
                            graph_.AddEdge(edge);
                        }
                    }
                }
            }
        }

        size_t TransportRouter::ProcessStops() {

            size_t counter = 0;
            const auto& stops = catalogue_.GetStopsMap();
            stop_name_to_id_.reserve(stops.size());
            id_to_stop_.reserve(stops.size());
            for (auto& stop : stops) {
                stop_name_to_id_.insert({ stop.first, counter });
                id_to_stop_.insert({ counter++, stop.second });
            }
            return counter;
        }

        graph::Edge<RouteWeight> TransportRouter::BuildEdge(const domain::Bus* route,
            int from_stop_index, int to_stop_index) {

            graph::Edge<RouteWeight> edge;
            edge.from = stop_name_to_id_.at(route->stops_.at(static_cast<size_t>(from_stop_index))->name_);
            edge.to = stop_name_to_id_.at(route->stops_.at(static_cast<size_t>(to_stop_index))->name_);
            edge.weight.bus_name = route->bus_name_;
            edge.weight.span_count = static_cast<int>(to_stop_index - from_stop_index);
            return edge;
        }

        double TransportRouter::ComputeRouteTime(const domain::Bus* route, int from_stop_index, int to_stop_index) {
            auto distance =
                catalogue_.GetDistance(route->stops_.at(static_cast<size_t>(from_stop_index)),
                    route->stops_.at(static_cast<size_t>(to_stop_index)));
            return distance / (settings_.velocity * VELOCITY_COEF);
        }

    } // namespace router

} //namespace transport_catalogue

