#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace transport_catalogue {

    namespace router {


        constexpr static double VELOCITY_COEF = 1000.0 / 60.0;

        struct RouteWeight {
            std::string_view bus_name;
            double total_time = 0;
            int span_count = 0;
        };

        bool operator<(const RouteWeight& left, const RouteWeight& right);
        bool operator>(const RouteWeight& left, const RouteWeight& right);
        RouteWeight operator+(const RouteWeight& left, const RouteWeight& right);

        class TransportRouter {
        public:

            struct RouterSettings {
                int wait_time = 0;
                double velocity = 100;
            };

            using Router = graph::Router<RouteWeight>;
            using Graph = graph::DirectedWeightedGraph<RouteWeight>;
            using IdToStopMap = std::unordered_map<size_t, const domain::Stop*>;
            using StopNameToIdMap = std::unordered_map<std::string_view, size_t>;

            TransportRouter(const transport_catalogue::TransportCatalogue& catalogue,
                const RouterSettings& settings);

            void InitializeRouter();
            void SetInitialized();

            const RouterSettings& GetSettings() const;
            RouterSettings& GetSettings();

            domain::RouteStat FindRoute(const std::string_view from, const std::string_view to);

        private:

            bool is_initialized_ = false;

            const transport_catalogue::TransportCatalogue& catalogue_;
            RouterSettings settings_;

            IdToStopMap id_to_stop_;
            StopNameToIdMap stop_name_to_id_;

            Graph graph_;
            std::unique_ptr<Router> router_;

            void BuildRouterGraph();
            size_t ProcessStops();
            graph::Edge<RouteWeight> BuildEdge(const domain::Bus* route, int from_stop_index, int to_stop_index);
            double ComputeRouteTime(const domain::Bus* route, int from_stop_index, int to_stop_index);
        };

    } // namespace transport_router
}
