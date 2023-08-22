#pragma once

#include <deque>
#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <numeric>

#include "domain.h"

namespace transport_catalogue {

	using namespace domain;

	class TransportCatalogue {
	public:

		TransportCatalogue() = default;

		using MapStopsDistances = std::unordered_map<std::pair<Stop*, Stop*>, size_t, PointersPairHasher>;

		void ComputeRouteLength(Route& route);
		void AddStop(Stop&& stop);
		void AddRoute(Route&& route);
		void AddStopsDistance(Stop* from_stop, Stop* to_stop, size_t dist);

		Stop* FindStopByName(const std::string_view stop) const;
		Route* FindRouteByName(const std::string_view route) const;
		RouteStat* GetRouteInfo(const std::string_view route) const;
		StopStat* GetBusesForStopInfo(const std::string_view bus_stop) const;
		RoutesMap& GetRoutesMap();
		const MapStopsDistances& GetStopDistancesRef() const;
		size_t GetDistanceBase(Stop* from_stop, Stop* to_stop) const;
		size_t GetDistance(Stop* from_stop, Stop* to_stop) const;

	private:

		std::deque<Stop> stops_data_;
		StopsMap stopnames_to_stops_;

		std::deque<Route> routes_data_;
		RoutesMap routenames_to_routes_;

		MapStopsDistances stops_distances_;
	};
} // namespace transport_catalogue