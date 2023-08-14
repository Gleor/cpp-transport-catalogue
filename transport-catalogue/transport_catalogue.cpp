#include "transport_catalogue.h"

namespace transport_catalogue {

bool Stop::operator==(const Stop& other) const {
	return name_ == other.name_ &&
		coordinates_.lat == other.coordinates_.lat && coordinates_.lng == other.coordinates_.lng;
}
Stop::Stop(const std::string_view stop_name, const double lat, const double lng) : 
	name_(stop_name), coordinates_(geo::Coordinates{lat, lng})
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

Route::Route(Route* other) :
	route_name_(other->route_name_), stops_(other->stops_), is_circular_(other->is_circular_)
{
}

Route& Route::SetRouteName(std::string_view route_name)
{
	route_name_ = std::string(route_name);
	return *this;
}

Route& Route::SetStops(std::vector<Stop*>&& stops)
{
	stops_ = std::move(stops);
	return *this;
}

Route& Route::SetRouteType(bool type)
{
	is_circular_ = type;
	return *this;
}

const std::string& Route::GetRouteName() const
{
	return route_name_;
}

const std::vector<Stop*>& Route::GetStops() const
{
	return stops_;
}

bool Route::GetRouteType() const
{
	return is_circular_;
}

StopStat::StopStat(std::string_view name, std::set<std::string_view>& buses) :
	stop_name_(name), buses_(buses)
{
}

RouteStat::RouteStat(std::string_view name, size_t stops, size_t unique_stops, size_t dist, double curvature) :
	route_name_(name), route_stops_num_(stops), unique_stops_num_(unique_stops),
	route_length_(dist), route_curvature_(curvature)
{
}

std::size_t TransportCatalogue::PointersPairHasher::operator()(const std::pair<Stop*, Stop*> pointers) const noexcept
{
	static const size_t shift = (size_t)log2(1 + sizeof(Stop));
	return ((size_t)(pointers.first) >> shift) + ((size_t)(pointers.second) >> shift * 7);

}

void TransportCatalogue::ComputeRouteLength(Route& route)
{
	const auto& stops_ref = route.GetStops();
	//Вычисляем длину маршрута двумя способами
	route.geo_route_length_ = std::transform_reduce(
		stops_ref.begin(),
		stops_ref.end() - 1,
		stops_ref.begin() + 1,
		0.0,
		std::plus<double>(),
		[](const auto stop_l, const auto stop_r) {
			return geo::ComputeDistance(stop_l->coordinates_, stop_r->coordinates_);
		});
	route.real_route_length_ = std::transform_reduce(
		stops_ref.begin(),
		stops_ref.end() - 1,
		stops_ref.begin() + 1,
		0ULL,
		std::plus<size_t>(),
		[&](const auto stop_l, const auto stop_r) {
			return GetDistance(stop_l, stop_r);
		});
	//Вычисляем извилистость
	route.route_curvature_ = route.real_route_length_ / route.geo_route_length_;
}

void TransportCatalogue::AddStop(Stop&& stop)
{
	if (!stopnames_to_stops_.count(stop.name_)) {
		Stop& stop_ref = stops_data_.emplace_back(std::move(stop));
		stopnames_to_stops_.insert({ std::string_view(stop_ref.name_), &stop_ref });
	}
}

void TransportCatalogue::AddRoute(Route&& route)
{
	if (!routenames_to_routes_.count(route.route_name_)) {
		//Заполненяем контейнеры маршрутами
		Route& route_ref = routes_data_.emplace_back(std::move(route));
		routenames_to_routes_.insert({std::string_view(route_ref.route_name_), &route_ref});

		//Находим уникальные остоновки
		std::vector<Stop*> tmp = route_ref.stops_;
		std::sort(tmp.begin(), tmp.end());
		auto last = std::unique(tmp.begin(), tmp.end());

		route_ref.unique_stops_ = (last != tmp.end() ? std::distance(tmp.begin(), last) : tmp.size());

		//Проверяем тип маршрута
		if (!route_ref.is_circular_) {
			for (int i = static_cast<int>(route_ref.stops_.size()) - 2; i >= 0; --i) {
				route_ref.stops_.push_back(route_ref.stops_[i]);
			}
		}

		// Рассчитываем длинну маршрута
		if (route_ref.stops_.size() > 1) {
			TransportCatalogue::ComputeRouteLength(route_ref);
		}
	}

}

void TransportCatalogue::AddStopsDistance(Stop* from_stop, Stop* to_stop, size_t dist)
{
	if (from_stop != nullptr && to_stop != nullptr) {
		stops_distances_.insert({ {from_stop, to_stop}, dist });
	}
}

Stop* TransportCatalogue::FindStopByName(const std::string_view stop) const
{
	if (stopnames_to_stops_.count(stop) == 0) {
		return nullptr;
	}
	else {
		return stopnames_to_stops_.at(stop);
	}
}

Route* TransportCatalogue::FindRouteByName(const std::string_view route) const
{
	if (routenames_to_routes_.count(route) == 0) {
		return nullptr;
	}
	else {
		return routenames_to_routes_.at(route);
	}
}

RouteStat* TransportCatalogue::GetRouteInfo(const std::string_view route) const
{
	Route* route_ref = FindRouteByName(route);
	if (route_ref == nullptr) {
		return nullptr;
	}
	return new RouteStat(route_ref->route_name_, route_ref->stops_.size(), route_ref->unique_stops_,
		route_ref->real_route_length_, route_ref->route_curvature_);
}

StopStat* TransportCatalogue::GetBusesForStopInfo(const std::string_view bus_stop) const
{
	Stop* stop = FindStopByName(bus_stop);
	if (stop == nullptr) return nullptr;
	std::set<std::string_view> buses;
	for (const auto& route : routenames_to_routes_) {
		auto tmp = std::find_if(route.second->stops_.begin(), route.second->stops_.end(),
			[bus_stop](Stop* curr_stop) {
				return (curr_stop->name_ == bus_stop);
			});
		if (tmp != route.second->stops_.end()) {
			buses.insert(route.second->route_name_);
		}
	}
	return  new StopStat(bus_stop, buses);
}

const TransportCatalogue::MapStopsDistances& TransportCatalogue::GetStopDistancesRef() const
{
	return stops_distances_;
}

size_t TransportCatalogue::GetDistanceBase(Stop* from_stop, Stop* to_stop) const
{
	if (stops_distances_.count({ from_stop, to_stop }) > 0) {
		return stops_distances_.at({ from_stop, to_stop });
	}
	else {
		return 0U;
	}
}

size_t TransportCatalogue::GetDistance(Stop* from_stop, Stop* to_stop) const
{
	size_t result = GetDistanceBase(from_stop, to_stop);

    return (result > 0 ? result : GetDistanceBase(to_stop, from_stop));
}

}