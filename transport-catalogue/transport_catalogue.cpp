#include "transport_catalogue.h"

namespace transport_catalogue {

	void TransportCatalogue::ComputeRouteLength(Bus& route)
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

			_all_stops_to_router.push_back(&stop_ref);

			stopnames_to_stops_.insert({ std::string_view(stop_ref.name_), &stop_ref });
		}
	}

	void TransportCatalogue::AddBus(Bus&& bus)
	{
		if (!routenames_to_routes_.count(bus.bus_name_)) {
			//Заполненяем контейнеры маршрутами
			Bus& bus_ref = routes_data_.emplace_back(std::move(bus));

			_all_buses_to_router.push_back(&bus_ref);

			routenames_to_routes_.insert({ std::string_view(bus_ref.bus_name_), &bus_ref });

			//Находим уникальные остоновки
			std::vector<Stop*> tmp = bus_ref.stops_;
			std::sort(tmp.begin(), tmp.end());
			auto last = std::unique(tmp.begin(), tmp.end());

			bus_ref.unique_stops_ = (last != tmp.end() ? std::distance(tmp.begin(), last) : tmp.size());

			//Проверяем тип маршрута
			if (!bus_ref.is_circular_) {
				for (int i = static_cast<int>(bus_ref.stops_.size()) - 2; i >= 0; --i) {
					bus_ref.stops_.push_back(bus_ref.stops_[i]);
				}
			}

			// Рассчитываем длинну маршрута
			if (bus_ref.stops_.size() > 1) {
				TransportCatalogue::ComputeRouteLength(bus_ref);
			}
		}

	}

	void TransportCatalogue::AddBusData(Bus&& bus)
	{
		if (routenames_to_routes_.count(bus.bus_name_) == 0) {
			// заполнение основной базы
			auto& ref = routes_data_.emplace_back(std::move(bus));
			// заполнение базы для роутера
			_all_buses_to_router.push_back(&ref);
			// заполнение мапы для ускоренного поиска
			routenames_to_routes_.insert({ std::string_view(ref.bus_name_), &ref });

		}
	}

	void TransportCatalogue::AddStopsDistance(Stop* from_stop, Stop* to_stop, size_t dist)
	{
		if (from_stop != nullptr && to_stop != nullptr) {
			stops_distances_.insert({ {from_stop, to_stop}, dist });
		}
	}

	void TransportCatalogue::AddRouteFromSerializer(Bus&& bus) {
		// добавляем если такого маршрута нет в базе
		if (routenames_to_routes_.count(bus.bus_name_) == 0) {
			// заполнение основной базы
			auto& ref = routes_data_.emplace_back(std::move(bus));
			// заполнение базы для роутера
			_all_buses_to_router.push_back(&ref);
			// заполнение мапы для ускоренного поиска
			routenames_to_routes_.insert({ std::string_view(ref.bus_name_), &ref });

			// в методе не выполняются математические расчёты расстояний и т.п, так как в базе всё уже есть
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

	Bus* TransportCatalogue::FindRouteByName(const std::string_view route) const
	{
		if (routenames_to_routes_.count(route) == 0) {
			return nullptr;
		}
		else {
			return routenames_to_routes_.at(route);
		}
	}

	BusStat* TransportCatalogue::GetBusInfo(const std::string_view route) const
	{
		Bus* bus_ref = FindRouteByName(route);
		if (bus_ref == nullptr) {
			return nullptr;
		}
		return new BusStat(bus_ref->bus_name_, bus_ref->stops_.size(), bus_ref->unique_stops_,
			bus_ref->real_route_length_, bus_ref->route_curvature_);
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
				buses.insert(route.second->bus_name_);
			}
		}
		return  new StopStat(bus_stop, buses);
	}

	const BusesMap& TransportCatalogue::GetBusesMap() const
	{
		return routenames_to_routes_;
	}

	const StopsMap& TransportCatalogue::GetStopsMap()  const
	{
		return stopnames_to_stops_;
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

	size_t TransportCatalogue::GetStopsCount() const
	{
		return stops_data_.size();
	}

	const std::deque<Stop*>& TransportCatalogue::GetAllStopsData() const
	{
		return _all_stops_to_router;
	}

	const std::deque<Bus*>& TransportCatalogue::GetAllBusesData() const
	{
		return _all_buses_to_router;
	}

}