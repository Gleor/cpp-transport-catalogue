#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>

namespace json_reader {

    class JsonReader {
    public:
        JsonReader(std::istream& input, request_handler::RequestHandler& request_handler)
            : input_(json::Load(input)), request_handler_(request_handler)
        {}
        svg::Point ParsePoint(const json::Node& node) const;
        void RouteParser(domain::Request* request, const json::Dict& node);
        void StopParser(domain::Request* request, const json::Dict& node);

        void ParseBaseRequest(domain::Request*, const json::Dict&);
        void ParseStatRequest(domain::Request*, const json::Dict&);
        void ParseRenderRequest(map_renderer::RendererSettings& settings, const json::Dict& node);

        void ProcessBaseRequests(const json::Array& arr);
        void ProcessStatRequests(const json::Array& arr);
        void ProcessRenderRequest(const json::Dict& render_settings);
        void ProcessRequests();

        const json::Node StopToNode(size_t id, domain::StopStat* stop) const;
        const json::Node RouteToNode(size_t id, domain::RouteStat* route) const;
        const json::Node MapToNode(size_t id, svg::Document map) const;

        svg::Color ParseColor(const json::Node& node) const;

        void Printer(json::Array&& result);

    private:
        json::Document input_;

        request_handler::RequestHandler& request_handler_;
        domain::RequestsMap json_requests_ = {};
        std::deque<domain::Request> base_requests_data_;
        std::deque<domain::Request> stat_requests_data_;
    };
}
