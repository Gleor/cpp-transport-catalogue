#include "json_reader.h"
#include "domain.h"
#include "svg.h"

namespace json_reader {

    JsonReader::JsonReader(std::istream& input, std::ostream& out, ProgramTask task)
        : input_(json::Load(input))
        , out_(out)
    {
        switch (task)
        {
        case json_reader::make_base:
            this->MakeBaseTask();
            break;
        case json_reader::process_requests:
            this->ProcessRequestsTask();
            break;
        default:
            break;
        }
    }

    void JsonReader::RouteParser(domain::Request* request, const json::Dict& node) {
        if (node.count("is_roundtrip") != 0) {
            if (node.at("is_roundtrip").AsBool()) {
                request->is_circular_ = true;
            }
            else {
                request->is_circular_ = false;
            }
        }

        if (node.count("stops") != 0) {
            const json::Array& stops_ = node.at("stops").AsArray();
            for (const json::Node& stop : stops_) {
                request->stops_.push_back(stop.AsString());
            }
        }
    }

    void JsonReader::StopParser(domain::Request* request, const json::Dict& node) {
        if (node.count("latitude") != 0) {
            request->coordinates_.lat = node.at("latitude").AsDouble();
        }

        if (node.count("longitude") != 0) {
            request->coordinates_.lng = node.at("longitude").AsDouble();
        }

        if (node.count("road_distances") != 0) {
            const json::Dict& distances_ = node.at("road_distances").AsDict();
            for (const auto& item : distances_) {
                request->distances_[item.first] = static_cast<int64_t>(item.second.AsInt());
            }
        }

    }
    void JsonReader::ParseBaseRequest(domain::Request* request, const json::Dict& node)
    {
        if (node.count("name") != 0) {
            request->name_ = node.at("name").AsString();
        }

        if (node.count("type") != 0) {
            if (node.at("type").AsString() == "Bus") {
                request->type_ = transport_catalogue::RequestType::add_bus;
                request->key_ = "Bus";
                RouteParser(request, node);
            }
            else {
                request->type_ = transport_catalogue::RequestType::add_stop;
                request->key_ = "Stop";
                StopParser(request, node);
            }
        }
    }

    void JsonReader::ParseStatRequest(domain::Request* request, const json::Dict& node)
    {

        if (node.count("id") != 0) {
            request->id_ = node.at("id").AsInt();
        }

        if (node.count("name") != 0) {
            request->name_ = node.at("name").AsString();
        }

        if (node.count("from") != 0) {
            request->from_ = node.at("from").AsString();
        }

        if (node.count("to") != 0) {
            request->to_ = node.at("to").AsString();
        }

        if (node.count("type") != 0) {
            if (node.at("type").AsString() == "Bus") {
                request->key_ = "Bus";
            }
            else if (node.at("type").AsString() == "Stop") {
                request->key_ = "Stop";
            }
            else if (node.at("type").AsString() == "Map") {
                request->key_ = "Map";
            }
            else if (node.at("type").AsString() == "Route") {
                request->key_ = "Route";
            }
        }
    }

    void JsonReader::ParseRenderRequest(map_renderer::RendererSettings& settings, const json::Dict& node) {

        settings.width_ = node.at("width").AsDouble();
        settings.height_ = node.at("height").AsDouble();
        settings.padding_ = node.at("padding").AsDouble();
        settings.stop_radius_ = node.at("stop_radius").AsDouble();
        settings.line_width_ = node.at("line_width").AsDouble();
        settings.bus_label_font_size_ = node.at("bus_label_font_size").AsInt();
        settings.bus_label_offset_ = ParsePoint(node.at("bus_label_offset"));
        settings.stop_label_font_size_ = node.at("stop_label_font_size").AsInt();
        settings.stop_label_offset_ = ParsePoint(node.at("stop_label_offset"));
        settings.underlayer_color_ = ParseColor(node.at("underlayer_color"s));
        settings.underlayer_width_ = node.at("underlayer_width"s).AsDouble();

        const json::Node& color_palette_node = node.at("color_palette"s);
        size_t arr_size = color_palette_node.AsArray().size();
        if (arr_size != 0) {
            settings.color_palette_.clear();
            settings.color_palette_.reserve(arr_size);
            for (const auto& node : color_palette_node.AsArray()) {
                settings.color_palette_.push_back(ParseColor(node));
            }
        }
    }

    void JsonReader::ParseRouteSettingsRequest(transport_catalogue::router::RouterSettings& settings, const json::Dict& node)
    {
        for (auto& item : node) {
            if (item.first == "bus_wait_time"s)
            {
                settings._bus_wait_time = static_cast<size_t>(item.second.AsInt());
            }
            else if (item.first == "bus_velocity"s)
            {
                settings._bus_velocity = (item.second.AsDouble());
            }
            else
            {
                continue;
            }
        }
    }

    void JsonReader::ProcessBaseRequests(const json::Array& arr)
    {

        domain::RequestsMap request_map;
        for (const auto& request : arr) {
            domain::Request base_request;
            ParseBaseRequest(&base_request, request.AsDict());
            request_map[base_request.type_].emplace_back(base_request);
        }
        if (!request_map.empty()) {
            request_handler_.HandleBaseRequests(std::move(request_map));
        }
    }

    void JsonReader::ProcessStatRequests(const json::Array& arr)
    {
        json::Array result;

        result.reserve(arr.size());
        for (const auto& request : arr) {
            domain::Request stat_request;
            ParseStatRequest(&stat_request, request.AsDict());
            if (stat_request.key_ == "Stop") {
                result.push_back(StopToNode(stat_request.id_, request_handler_.GetStop(stat_request.name_)).GetRoot());
            }
            else if (stat_request.key_ == "Bus") {
                result.push_back(BusToNode(stat_request.id_, request_handler_.GetBus(stat_request.name_)).GetRoot());
            }
            else if (stat_request.key_ == "Map") {
                auto doc = MapToNode(stat_request.id_, request_handler_.GetMap());
                result.push_back(MapToNode(stat_request.id_, request_handler_.GetMap()).GetRoot());
            }
            else if (stat_request.key_ == "Route") {
                result.push_back(RouteToNode(stat_request.id_, request_handler_.GetRoute(stat_request.from_, stat_request.to_)).GetRoot());
            }
        }

        Printer(std::move(result));

    }

    void JsonReader::ProcessRenderRequest(const json::Dict& render_settings)
    {
        map_renderer::RendererSettings settings;

        ParseRenderRequest(settings, render_settings);

        request_handler_.SetMapRenderSettings(std::move(settings));

    }

    void JsonReader::ProcessRouteSettingsRequest(const json::Dict& route_settings)
    {
        transport_catalogue::router::RouterSettings settings;

        ParseRouteSettingsRequest(settings, route_settings);

        {
            request_handler_.SetRouterSettings(std::move(settings));
            request_handler_.InitializeTransportRouterGraph();
        }
    }

    void JsonReader::MakeBaseTask()
    {
        const json::Dict& json_requests = input_.GetRoot().AsDict();

        if (json_requests.count("base_requests"))
        {
            ProcessBaseRequests(json_requests.at("base_requests").AsArray());
        }

        if (json_requests.count("render_settings"))
        {
            ProcessRenderRequest(json_requests.at("render_settings").AsDict());
        }

        if (json_requests.count("routing_settings"))
        {
            ProcessRouteSettingsRequest(json_requests.at("routing_settings").AsDict());
        }

        if (json_requests.count("serialization_settings"))
        {
            std::ofstream output(json_requests.at("serialization_settings").AsDict().at("file").AsString(), std::ios::binary);

            assert(request_handler_.SerializeData(output));
        }
    }

    void JsonReader::ProcessRequestsTask()
    {
        const json::Dict& json_requests = input_.GetRoot().AsDict();

        if (json_requests.count("serialization_settings"))
        {
            std::ifstream input(json_requests.at("serialization_settings").AsDict().at("file").AsString(), std::ios::binary);

            assert(request_handler_.DeserializeData(input));
        }

        if (json_requests.count("stat_requests"))
        {
            ProcessStatRequests(json_requests.at("stat_requests").AsArray());
        }
    }

    const json::Document JsonReader::StopToNode(size_t id, domain::StopStat* stop_stat) const {

        if (stop_stat == nullptr) {
            return json::Document(
                json::Builder()
                .StartDict()
                .Key("request_id"s).Value(static_cast<int>(id))
                .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build());
        }
        else {
            json::Array buses;
            for (auto& bus : stop_stat->buses_) {
                buses.push_back(json::Node{ std::string(bus) });
            }
            return json::Document(
                json::Builder()
                .StartDict()
                .Key("buses"s).Value(buses)
                .Key("request_id"s).Value(static_cast<int>(id))
                .EndDict()
                .Build());
        }

    }
    const json::Document JsonReader::BusToNode(size_t id, domain::BusStat* bus_stat) const {

        if (bus_stat == nullptr) {
            return json::Document(
                json::Builder()
                .StartDict()
                .Key("request_id"s).Value(static_cast<int>(id))
                .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build());
        }
        else {
            return json::Document(
                json::Builder()
                .StartDict()
                .Key("request_id"s).Value(static_cast<int>(id))
                .Key("curvature"s).Value(bus_stat->route_curvature_)
                .Key("route_length"s).Value(static_cast<int>(bus_stat->route_length_))
                .Key("stop_count"s).Value(static_cast<int>(bus_stat->bus_stops_num_))
                .Key("unique_stop_count"s).Value(static_cast<int>(bus_stat->unique_stops_num_))
                .EndDict()
                .Build());
        }
    }

    const json::Document JsonReader::MapToNode(size_t id, const std::string& map) const {

        return json::Document(
            json::Builder()
            .StartDict()
            .Key("map"s).Value(map)
            .Key("request_id"s).Value(static_cast<int>(id))
            .EndDict()
            .Build());
    }

    const json::Document JsonReader::RouteToNode(size_t id, domain::RouteStat route_stat) const
    {
        if (!route_stat.is_found_) {
            return json::Document(
                json::Builder()
                .StartDict()
                .Key("request_id"s).Value(static_cast<int>(id))
                .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build());
        }
        else {
            json::Array route_items;
            for (auto& item : route_stat.route_items_) {

                if (item.GetType() == graph::wait)
                {
                    route_items.push_back(
                        json::Builder()
                        .StartDict()
                        .Key("type"s).Value(std::string("Wait"s))
                        .Key("stop_name"s).Value(std::string(item.GetName()))
                        .Key("time"s).Value(item.GetTime())
                        .EndDict()
                        .Build());
                }
                else
                {
                    route_items.push_back(
                        json::Builder()
                        .StartDict()
                        .Key("type"s).Value(std::string("Bus"s))
                        .Key("bus"s).Value(std::string(item.GetName()))
                        .Key("span_count"s).Value(item.GetSpanCount())
                        .Key("time"s).Value(item.GetTime())
                        .EndDict()
                        .Build());
                }

            }
            return json::Document(
                json::Builder()
                .StartDict()
                .Key("request_id"s).Value(static_cast<int>(id))
                .Key("total_time"s).Value(route_stat.total_time_)
                .Key("items"s).Value(route_items)
                .EndDict()
                .Build());
        }
    }

    svg::Point JsonReader::ParsePoint(const json::Node& node) const {

        const json::Array& array = node.AsArray();
        const json::Node& x = array[0];
        const json::Node& y = array[1];

        return { x.AsDouble(), y.AsDouble() };
    }

    svg::Color JsonReader::ParseColor(const json::Node& node) const {
        if (node.IsArray()) {
            const json::Array& array = node.AsArray();

            const json::Node& red_color = array[0];
            const uint8_t red = static_cast<uint8_t>(red_color.AsInt());

            const json::Node& green_color = array[1];
            const uint8_t green = static_cast<uint8_t>(green_color.AsInt());

            const json::Node& blue_color = array[2];
            const uint8_t blue = static_cast<uint8_t>(blue_color.AsInt());

            if (array.size() == 3U) {
                return svg::Rgb{ red, green, blue };
            }
            else if (array.size() == 4U) {
                const json::Node& alpha = array[3];
                return svg::Rgba{ red, green, blue, alpha.AsDouble() };
            }
        }
        else {
            return node.AsString();
        }

        return {};
    }

    void JsonReader::Printer(json::Array&& result)
    {
        json::Print(json::Document{ result }, out_);
    }

}