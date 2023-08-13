#include "stat_reader.h"

#include <iomanip>

namespace transport_catalogue {
namespace stat_reader {

void ProcessRequests(std::ostream& out, TransportCatalogue& catalogue)
{
    size_t requests_count;
    std::cin >> requests_count;
    for (size_t i = 0; i < requests_count; ++i) {
        std::string keyword, line;
        std::cin >> keyword;
        std::getline(std::cin, line);

        if (keyword == "Bus") {
            std::string route_name = line.substr(1, line.npos);
            RouteStat* route_stat = catalogue.GetRouteInfo(route_name);
            if (route_stat != nullptr) {
                out << "Bus " << route_stat->route_name_ << ": " << route_stat->route_stops_num_ << " stops on route, "
                    << route_stat->unique_stops_num_ << " unique stops, " << std::setprecision(6)
                    << route_stat->route_length_ << " route length, " << route_stat->route_curvature_ << " curvature\n";
            }
            else {
                out << "Bus " << route_name << ": not found\n";
            }
        }
        if (keyword == "Stop") {
            std::string stop_name = line.substr(1, line.npos);
            StopStat* stop_stat = catalogue.GetBusesForStopInfo(stop_name);
            out << "Stop " << stop_name << ": ";
            if (stop_stat != nullptr) {
                if (!stop_stat->buses_.empty()) {
                    out << "buses ";
                    bool first = true;
                    for (const auto& bus : stop_stat->buses_) {
                        if (first) {
                            out << bus;
                            first = false;
                        }
                        else {
                            out << " " << bus;
                        }
                    }
                    out << "\n";
                }
                else {
                    out << "no buses\n";
                }
            }
            else {
                out << "not found\n";
            }
        }
    }
}
}// namespace stat_reader
}// namespace transport_catalogue