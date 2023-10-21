#include "domain.h"

namespace domain {

	bool Stop::operator==(const Stop& other) const {
		return name_ == other.name_ &&
			coordinates_.lat == other.coordinates_.lat && coordinates_.lng == other.coordinates_.lng;
	}
	Stop::Stop(const std::string_view stop_name, const double lat, const double lng) :
		name_(stop_name), coordinates_(geo::Coordinates{ lat, lng })
	{
	}

	Stop::Stop(Stop* other) :
		name_(other->name_), coordinates_(other->coordinates_)
	{
	}

	Stop& Stop::SetStopName(std::string_view stop_name)
	{
		name_ = std::string(stop_name);
		return *this;
	}

	Stop& Stop::SetStopCoordinates(double lat, double lng)
	{
		coordinates_.lat = lat;
		coordinates_.lng = lng;
		return *this;
	}

	Stop& Stop::SetStopCoordinates(const geo::Coordinates& coordinates)
	{
		coordinates_ = coordinates;
		return *this;
	}

	Stop& Stop::SetStopCoordinates(geo::Coordinates&& coordinates)
	{
		coordinates_ = std::move(coordinates);
		return *this;
	}

	const std::string& Stop::GetStopName() const
	{
		return name_;
	}

	std::string Stop::GetStopName()
	{
		return name_;
	}

	const geo::Coordinates& Stop::GetStopCoordinates() const
	{
		return coordinates_;
	}

	geo::Coordinates Stop::GetStopCoordinates()
	{
		return coordinates_;
	}

	Bus::Bus(Bus* other) :
		bus_name_(other->bus_name_), stops_(other->stops_), is_circular_(other->is_circular_)
	{
	}

	Bus& Bus::SetBusName(std::string_view route_name)
	{
		bus_name_ = std::string(route_name);
		return *this;
	}

	Bus& Bus::SetStops(std::vector<Stop*>&& stops)
	{
		stops_ = std::move(stops);
		return *this;
	}

	Bus& Bus::SetBusType(bool type)
	{
		is_circular_ = type;
		return *this;
	}

	Bus& Bus::SetUniqueStops(size_t stops)
	{
		unique_stops_ = stops;
		return *this;
	}

	Bus& Bus::SetGeoRouteLength(double length)
	{
		geo_route_length_ = length;
		return *this;
	}

	Bus& Bus::SetRealRouteLength(size_t length)
	{
		real_route_length_ = length;
		return *this;
	}

	Bus& Bus::SetCurvature(double curvature)
	{
		route_curvature_ = curvature;
		return *this;
	}

	const std::string& Bus::GetBusName() const
	{
		return bus_name_;
	}

	const std::vector<Stop*>& Bus::GetStops() const
	{
		return stops_;
	}

	bool Bus::GetBusType() const
	{
		return is_circular_;
	}

	size_t Bus::GetUniqueStops() const
	{
		return unique_stops_;
	}

	double Bus::GetGeoRouteLength() const
	{
		return geo_route_length_;
	}

	size_t Bus::GetRealRouteLength() const
	{
		return real_route_length_;
	}

	double Bus::GetCurvature() const
	{
		return route_curvature_;
	}

	StopStat::StopStat(std::string_view name, std::set<std::string_view>& buses) :
		stop_name_(name), buses_(buses)
	{
	}

	BusStat::BusStat(std::string_view name, size_t stops, size_t unique_stops, size_t dist, double curvature) :
		bus_name_(name), bus_stops_num_(stops), unique_stops_num_(unique_stops),
		route_length_(dist), route_curvature_(curvature)
	{
	}

	std::size_t PointersPairHasher::operator()(const std::pair<Stop*, Stop*> pointers) const noexcept
	{
		static const size_t shift = (size_t)log2(1 + sizeof(Stop));
		return ((size_t)(pointers.first) >> shift) + ((size_t)(pointers.second) >> shift * 7);

	}

	RouteItem& RouteItem::SetName(std::string_view name) {
		name_ = name;
		return *this;
	}

	RouteItem& RouteItem::SetSpanCount(int count) {
		span_count_ = count;
		return *this;
	}

	RouteItem& RouteItem::SetTime(double time) {
		time_ = time;
		return *this;
	}

	RouteItem& RouteItem::SetEdgeType(graph::EdgeType type)
	{
		type_ = type;
		return *this;
	}

	std::string_view RouteItem::GetName() const {
		return name_;
	}

	int RouteItem::GetSpanCount() const {
		return span_count_;
	}

	double RouteItem::GetTime() const {
		return time_;
	}

	graph::EdgeType RouteItem::GetType() const
	{
		return type_;
	}
	
}