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

		void ComputeRouteLength(Bus& route);
		void AddStop(Stop&& stop);
		void AddBus(Bus&& route);
		void AddBusData(Bus&& bus);
		void AddStopsDistance(Stop* from_stop, Stop* to_stop, size_t dist);
		void AddRouteFromSerializer(Bus&& bus);

		Stop* FindStopByName(const std::string_view stop) const;
		Bus* FindRouteByName(const std::string_view route) const;
		BusStat* GetBusInfo(const std::string_view route) const;
		StopStat* GetBusesForStopInfo(const std::string_view bus_stop) const;
		const BusesMap& GetBusesMap() const;
		const StopsMap& GetStopsMap() const;
		const MapStopsDistances& GetStopDistancesRef() const;
		size_t GetDistanceBase(Stop* from_stop, Stop* to_stop) const;
		size_t GetDistance(Stop* from_stop, Stop* to_stop) const;
		size_t GetStopsCount() const;
		
		const std::deque<Stop*>& GetAllStopsData() const;
		const std::deque<Bus*>& GetAllBusesData() const;
	private:

		std::deque<Stop> stops_data_;
		std::deque<Stop*> _all_stops_to_router;
		StopsMap stopnames_to_stops_;

		std::deque<Bus> routes_data_;
		std::deque<Bus*> _all_buses_to_router;
		BusesMap routenames_to_routes_;

		MapStopsDistances stops_distances_;

	};
} // namespace transport_catalogue