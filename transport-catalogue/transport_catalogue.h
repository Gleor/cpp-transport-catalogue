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
	bool operator==(const Stop&) const;
	Stop(const std::string_view, const double, const double);
	Stop(Stop*);

	Stop& SetStopName(std::string_view);
	Stop& SetStopCoordinates(double, double);
	Stop& SetStopCoordinates(const geo::Coordinates&);
	Stop& SetStopCoordinates(geo::Coordinates&&);

	const std::string& GetStopName() const;
	std::string GetStopName();
	const geo::Coordinates& GetStopCoordinates() const;
	geo::Coordinates GetStopCoordinates();

	std::string name_;
	geo::Coordinates coordinates_{ 0L, 0L };
};

struct Route {
	Route() = default;
	Route(Route*);

	Route& SetRouteName(std::string_view);
	Route& SetStops(std::vector<Stop*>&&);
	Route& SetRouteType(bool);

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
	explicit StopStat(std::string_view, std::set<std::string_view>&);
	std::string_view stop_name_;
	std::set<std::string_view> buses_;
};

struct RouteStat {
	RouteStat() = default;
	explicit RouteStat(std::string_view, size_t, size_t, size_t, double);
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
			std::size_t operator()(const std::pair<Stop*, Stop*>) const noexcept;
		};

		using MapStopsDistances = std::unordered_map<std::pair<Stop*, Stop*>, size_t, PointersPairHasher>;

		void ComputeRouteLength(Route&);
		void AddStop(Stop&&);
		void AddRoute(Route&&);
		void AddStopsDistance(Stop*, Stop*, size_t);

		Stop* FindStopByName(const std::string_view) const;
		Route* FindRouteByName(const std::string_view) const;
		RouteStat* GetRouteInfo(const std::string_view) const;
		StopStat* GetBusesForStopInfo(const std::string_view) const;
		const MapStopsDistances& GetStopDistancesRef() const;
		size_t GetDistanceBase(Stop*, Stop*) const;
		size_t GetDistance(Stop*, Stop*) const;

	private:

		std::deque<Stop> stops_data_;
		std::unordered_map<std::string_view, Stop*> stopnames_to_stops_;

		std::deque<Route> routes_data_;
		std::unordered_map<std::string_view, Route*> routenames_to_routes_;

		MapStopsDistances stops_distances_;
	};
} // namespace transport_catalogue