#pragma once

#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

using namespace std::literals;

namespace request_handler {

    class RequestHandler {
    public:

        using MapDistanses = std::map<std::string, std::map<std::string, int64_t>>;

        RequestHandler(transport_catalogue::TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer);

        svg::Document RenderMap() const;
        const std::string GetMap() const;

        void SetMapRenderSettings(map_renderer::RendererSettings&& settings);
        void SetRouterSettings(transport_catalogue::router::TransportRouter::RouterSettings&& settings);

        void InitializeTransportRouterGraph();

        domain::Stop MakeStop(domain::Request& stop);
        domain::Bus MakeBus(domain::Request& route);

        void ProcessDistances();

        void AddStops(std::vector<domain::Request>& requests);
        void AddBuses(std::vector<domain::Request>& requests);

        domain::StopStat* GetStop(const std::string_view request);
        domain::BusStat* GetBus(const std::string_view request);
        domain::RouteStat GetRoute(const std::string_view from, const std::string_view to);

        void HandleBaseRequests(domain::RequestsMap&& requests);

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        transport_catalogue::TransportCatalogue& transport_catalogue_;
        map_renderer::MapRenderer& map_renderer_;
        MapDistanses map_distances_ = {};

        std::unique_ptr<transport_catalogue::router::TransportRouter> transport_router_ = nullptr;
    };
}
 