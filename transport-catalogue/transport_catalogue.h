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

#include "geo.h"

namespace transport_catalogue {

struct Stop {
	Stop() = default;
	bool operator==(const Stop& other) const;
	Stop(const std::string_view stop_name, const double lat, const double lng);
	Stop(Stop* other);

	Stop& SetStopName(std::string_view stop_name);
	Stop& SetStopCoordinates(double lat, double lng);
	Stop& SetStopCoordinates(const geo::Coordinates& coordinates);
	Stop& SetStopCoordinates(geo::Coordinates&& coordinates);

	const std::string& GetStopName() const;
	std::string GetStopName();
	const geo::Coordinates& GetStopCoordinates() const;
	geo::Coordinates GetStopCoordinates();

	std::string name_;
	geo::Coordinates coordinates_{ 0L, 0L };
};

struct Route {
	Route() = default;
	Route(Route* other);

	Route& SetRouteName(std::string_view route_name);
	Route& SetStops(std::vector<Stop*>&& stops);
	Route& SetRouteType(bool type);

	const std::string& GetRouteName() const;
	const std::vector<Stop*>& GetStops() const;
	bool GetRouteType() const;

	std::string route_name_;
	std::vector<Stop*> stops_;
	size_t unique_stops_ = 0U;
	double geo_route_length_ = 0L;
	size_t real_route_length_ = 0U;
	double route_curvature_ = 1L;
	bool is_circular_ = false;
};

struct StopStat {
	explicit StopStat(std::string_view name, std::set<std::string_view>& buses);
	std::string_view stop_name_;
	std::set<std::string_view> buses_;
};

struct RouteStat {
	RouteStat() = default;
	explicit RouteStat(std::string_view name, size_t stops,
                       size_t unique_stops, size_t dist, double curvature);
	std::string route_name_;
	size_t route_stops_num_ = 0U;
	size_t unique_stops_num_ = 0U;
	size_t route_length_ = 0U;
	double route_curvature_ = 1L;

};
	class TransportCatalogue {
	public:

		TransportCatalogue() = default;

		class PointersPairHasher
		{
		public:
			std::size_t operator()(const std::pair<Stop*, Stop*> pointers) const noexcept;
		};

		using MapStopsDistances = std::unordered_map<std::pair<Stop*, Stop*>, size_t, PointersPairHasher>;

		void ComputeRouteLength(Route& route);
		void AddStop(Stop&& stop);
		void AddRoute(Route&& route);
		void AddStopsDistance(Stop* from_stop, Stop* to_stop, size_t dist);

		Stop* FindStopByName(const std::string_view stop) const;
		Route* FindRouteByName(const std::string_view route) const;
		RouteStat* GetRouteInfo(const std::string_view route) const;
		StopStat* GetBusesForStopInfo(const std::string_view bus_stop) const;
		const MapStopsDistances& GetStopDistancesRef() const;
		size_t GetDistanceBase(Stop* from_stop, Stop* to_stop) const;
		size_t GetDistance(Stop* from_stop, Stop* to_stop) const;

	private:

		std::deque<Stop> stops_data_;
		std::unordered_map<std::string_view, Stop*> stopnames_to_stops_;

		std::deque<Route> routes_data_;
		std::unordered_map<std::string_view, Route*> routenames_to_routes_;

		MapStopsDistances stops_distances_;
	};
} // namespace transport_catalogue