#include "input_reader.h"

#include <string>
#include <vector>
#include <cmath>

using namespace std::literals;

namespace transport_catalogue {
namespace input_reader {
std::vector<std::pair<std::string_view, int>> ParseStopDistance(std::string_view text) {

    std::vector<std::pair<std::string_view, int>> result;

    size_t start = text.find(',');
    start = text.find(',', start + 1) + (" "sv).size();
    size_t end = start;

    while (start != std::string_view::npos) {
        end = text.find("m"sv, start);
        int distance = std::stoi(std::string(text.substr(start, end - start)));

        start = end + 5;
        end = text.find(","sv, start);

        std::string_view stop_to = text.substr(start, end - start);
        result.emplace_back(stop_to, distance);

        start = (end == std::string_view::npos) ? end : end + 1;
    }

    return result;
}

std::pair<Stop, bool> ParseStop(const std::string& text) {

    size_t stop_name_end = text.find(": "s, 1);
    std:: string stop_name = text.substr(1, stop_name_end - 1);

    size_t lat_begin = stop_name_end + 2;
    size_t lat_end = text.find(","s, lat_begin);
    double lat = std::stod(text.substr(lat_begin, lat_end - lat_begin));

    size_t long_begin = lat_end + 2;
    size_t long_end = text.find(","s, long_begin);
    double lng = std::stod(text.substr(long_begin, long_end - long_begin));

    Stop stop(stop_name, lat, lng);

    bool info_left = long_end != std::string_view::npos;
    return { std::move(stop), info_left };
}

Route ParseRoute(std::string_view text, TransportCatalogue& catalogue) {

    Route route;

    size_t bus_start = text.find(' ') + 1;
    size_t bus_end = text.find(": "sv, bus_start);
    route.SetRouteName(text.substr(bus_start, bus_end - bus_start));

    route.SetRouteType((text[text.find_first_of("->")] == '>') ? true : false);
    std::string_view stops_separator = (route.GetRouteType() == true) ? " > "sv : " - "sv;

    size_t stop_begin = bus_end + 2;
    std::vector<std::string_view> route_stops;

    while (stop_begin <= text.length()) {
        size_t stop_end = text.find(stops_separator, stop_begin);

        route_stops.push_back(text.substr(stop_begin, stop_end - stop_begin));
        stop_begin = (stop_end == std::string_view::npos) ? stop_end : stop_end + stops_separator.size();
    }
    std::vector<Stop*> bus_stops;
    for (const std::string_view stop : route_stops) {
        bus_stops.push_back(catalogue.FindStopByName(stop));
    }
    route.SetStops(std::move(bus_stops));
    return route;
}

void ParseRequestsToUpdate(std::istream& input, TransportCatalogue& catalogue) {

    int requests_count = 0;
    input >> requests_count;

    std::vector<std::string> bus_requests;
    bus_requests.reserve(requests_count);
    std::vector<std::pair<std::string, std::string>> stop_distances;
    stop_distances.reserve(requests_count);

    std::string query_line;
    for (int id = 0; id < requests_count; ++id) {
        std::string keyword;
        input >> keyword;
        std::getline(input, query_line);
        if (keyword == "Stop"s) {
            auto [stop, is_store_query] = ParseStop(query_line);
            if (is_store_query)
                stop_distances.emplace_back(stop.GetStopName(), query_line);
            catalogue.AddStop(std::move(stop));
        }
        if (keyword == "Bus"s) {
            bus_requests.emplace_back(query_line);
        }
    }

    for (const auto& [stop_from, query] : stop_distances) {
        for (auto [stop_to, distance] : ParseStopDistance(query))
            catalogue.AddStopsDistance(catalogue.FindStopByName(stop_from), catalogue.FindStopByName(stop_to), distance);
    }

    for (const auto& bus_query : bus_requests) {
        catalogue.AddRoute(std::move(ParseRoute(bus_query, catalogue)));
    }
}

}// namespace input_reader
}// namespace transport_catalogue