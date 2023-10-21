#pragma once

#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "serialization.h"

using namespace std::literals;

namespace request_handler {

    class RequestHandler {
    public:

        using MapDistanses = std::map<std::string, std::map<std::string, int64_t>>;

        RequestHandler() = default;

        svg::Document RenderMap();
        const std::string GetMap();

        void InitializeMapRenderer();
        void InitializeRouter();

        void SetMapRenderSettings(map_renderer::RendererSettings&& settings);
        void SetRouterSettings(transport_catalogue::router::RouterSettings&& settings);

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
        
        bool SerializeData(std::ostream& output);
        bool DeserializeData(std::istream& input);

    private:

        transport_catalogue::TransportCatalogue transport_catalogue_;
        std::shared_ptr<map_renderer::MapRenderer> map_renderer_ = nullptr;
        std::shared_ptr<transport_catalogue::serialize::Serializator> serializer_ = nullptr;
        std::shared_ptr<transport_catalogue::router::TransportRouter> transport_router_ = nullptr;

        transport_catalogue::router::RouterSettings router_settings_;
        map_renderer::RendererSettings renderer_settings_;
        MapDistanses map_distances_ = {};

    };
}
 