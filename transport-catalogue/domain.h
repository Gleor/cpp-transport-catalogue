#pragma once

#include "geo.h"
#include "graph.h"

#include <string>
#include <string_view>
#include <set>
#include <map>
#include <vector>
#include <unordered_map>

using namespace std::literals;

namespace domain {
	
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

	struct Bus {
		Bus() = default;
		Bus(Bus* other);

		Bus& SetBusName(std::string_view Bus_name);
		Bus& SetStops(std::vector<Stop*>&& stops);
		Bus& SetBusType(bool type);
		Bus& SetUniqueStops(size_t stops);
		Bus& SetGeoRouteLength(double length);
		Bus& SetRealRouteLength(size_t length);
		Bus& SetCurvature(double curvature);

		const std::string& GetBusName() const;
		const std::vector<Stop*>& GetStops() const;
		bool GetBusType() const;
		size_t GetUniqueStops() const;
		double GetGeoRouteLength() const;
		size_t GetRealRouteLength() const;
		double GetCurvature() const;

		std::string bus_name_;
		std::vector<Stop*> stops_;
		size_t unique_stops_ = 0U;
		double geo_route_length_ = 0L;
		size_t real_route_length_ = 0U;
		double route_curvature_ = 1L;
		bool is_circular_ = false;
	};

	using StopsMap = std::unordered_map<std::string_view, Stop*>;
	using BusesMap = std::unordered_map<std::string_view, Bus*>;

	struct StopStat {
		explicit StopStat(std::string_view name, std::set<std::string_view>& buses);
		std::string_view stop_name_;
		std::set<std::string_view> buses_;
	};

	struct BusStat {
		BusStat() = default;
		explicit BusStat(std::string_view name, size_t stops,
			size_t unique_stops, size_t dist, double curvature);
		std::string bus_name_;
		size_t bus_stops_num_ = 0U;
		size_t unique_stops_num_ = 0U;
		size_t route_length_ = 0U;
		double route_curvature_ = 1L;

	};

	class PointersPairHasher
	{
	public:
		std::size_t operator()(const std::pair<Stop*, Stop*> pointers) const noexcept;
	};

	enum RequestType {
		null = 0,
		add_stop,
		add_bus,
		find_stop,
		find_bus
	};

	struct EnumClassHash
	{
		template <typename T>
		std::size_t operator()(T t) const
		{
			return static_cast<std::size_t>(t);
		}
	};

	struct Request {
		Request() = default;

		size_t id_ = 0;
		RequestType type_ = RequestType::null;
		std::string key_ = ""s;
		std::string name_ = ""s;
		std::string from_ = ""s;
		std::string to_ = ""s;
		geo::Coordinates coordinates_ = { 0L, 0L };
		std::vector<std::string> stops_ = {};
		std::map<std::string, int64_t> distances_ = {};
		bool is_circular_ = true;

	};

	using RequestsMap = std::unordered_map<RequestType, std::vector<Request>, EnumClassHash>;

	struct RouteItem {

		RouteItem& SetName(std::string_view bus_name);
		RouteItem& SetSpanCount(int count);
		RouteItem& SetTime(double time);
		RouteItem& SetEdgeType(graph::EdgeType);

		std::string_view GetName() const;
		int GetSpanCount() const;
		double GetTime() const;
		graph::EdgeType GetType() const;

		std::string name_ = {};
		int span_count_ = 0;
		double time_ = 0.0;
		graph::EdgeType type_;
	};

	struct RouteStat
	{
		double total_time_ = 0.0;
		std::vector<RouteItem> route_items_;
		bool is_found_ = false;
	};
}
