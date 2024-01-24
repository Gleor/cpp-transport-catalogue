#pragma once

#include "request_handler.h"
#include "json_builder.h"

#include <sstream>
#include <cassert>
#include <optional>
#include <fstream>

using namespace std::literals;

namespace json_reader {

    enum ProgramTask {
        make_base,
        process_requests
    };

    class JsonReader {
    public:
        JsonReader(std::istream& input, std::ostream& out, json_reader::ProgramTask task);

        svg::Point ParsePoint(const json::Node& node) const;
        void RouteParser(domain::Request* request, const json::Dict& node);
        void StopParser(domain::Request* request, const json::Dict& node);

        void ParseBaseRequest(domain::Request*, const json::Dict&);
        void ParseStatRequest(domain::Request*, const json::Dict&);
        void ParseRenderRequest(map_renderer::RendererSettings& settings, const json::Dict& node);
        void ParseRouteSettingsRequest(transport_catalogue::router::RouterSettings& settings, const json::Dict&);

        void ProcessBaseRequests(const json::Array& arr);
        void ProcessStatRequests(const json::Array& arr);
        void ProcessRenderRequest(const json::Dict& render_settings);
        void ProcessRouteSettingsRequest(const json::Dict& route_settings);

        void MakeBaseTask();
        void ProcessRequestsTask();

        const json::Document StopToNode(size_t id, domain::StopStat* stop_stat) const;
        const json::Document BusToNode(size_t id, domain::BusStat* bus_stat) const;
        const json::Document MapToNode(size_t id, const std::string& map) const;
        const json::Document RouteToNode(size_t id, domain::RouteStat route) const;

        svg::Color ParseColor(const json::Node& node) const;

        void Printer(json::Array&& result);

    private:
        json::Document input_;
        std::ostream& out_;

        request_handler::RequestHandler request_handler_;
        domain::RequestsMap json_requests_ = {};
        std::deque<domain::Request> base_requests_data_;
        std::deque<domain::Request> stat_requests_data_;
    };
}