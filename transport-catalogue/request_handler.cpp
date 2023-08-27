#include "request_handler.h"
#include "transport_catalogue.h"

namespace request_handler {

	RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer)
		: transport_catalogue_(catalogue), map_renderer_(renderer)
	{
	}

	svg::Document RequestHandler::RenderMap() const
	{
		return map_renderer_.RenderMap(transport_catalogue_.GetRoutesMap());
	}

	const std::string RequestHandler::GetMap() const
	{
		std::ostringstream strm;
		RenderMap().Render(strm);

		return strm.str();
	}

	void RequestHandler::SetMapRenderSettings(map_renderer::RendererSettings&& settings)
	{
		map_renderer_.SetSettings(std::move(settings));
	}

	domain::Stop RequestHandler::MakeStop(domain::Request& request) {
		return { std::string(request.name_), request.coordinates_.lat, request.coordinates_.lng };
	}

	domain::Route RequestHandler::MakeRoute(domain::Request& request) {
		domain::Route result;
		result.is_circular_ = request.is_circular_;
		result.route_name_ = request.name_;
		for (const std::string& stop : request.stops_) {
			result.stops_.push_back(transport_catalogue_.FindStopByName(stop));
		}
		return result;
	}

	void RequestHandler::ProcessDistances() {
		if (map_distances_.size() != 0) {
			for (auto& [first_stop, distances] : map_distances_) {
				domain::Stop* first_stop_ptr = transport_catalogue_.FindStopByName(first_stop);
				for (auto& dist : distances) {
					transport_catalogue_.AddStopsDistance(first_stop_ptr, transport_catalogue_.FindStopByName(dist.first), dist.second);
				}
			}
		}
	}

	void RequestHandler::AddStops(std::vector<domain::Request>& requests)
	{
		for (domain::Request& request : requests) {
			transport_catalogue_.AddStop(MakeStop(request));
			if (!request.distances_.empty()) {
				map_distances_[std::move(request.name_)] = std::move(request.distances_);
			}
		}
		ProcessDistances();
	}

	void RequestHandler::AddRoutes(std::vector<domain::Request>& requests)
	{
		for (domain::Request& request : requests) {
			transport_catalogue_.AddRoute(std::move(MakeRoute(request)));
		}
	}

	domain::StopStat* RequestHandler::GetStop(const std::string_view stop)
	{
		return transport_catalogue_.GetBusesForStopInfo(stop);
	}

	domain::RouteStat* RequestHandler::GetRoute(const std::string_view route)
	{
		return transport_catalogue_.GetRouteInfo(route);
	}

	void RequestHandler::HandleBaseRequests(domain::RequestsMap&& requests)
	{
		if (requests.count(domain::RequestType::add_stop)) {
			AddStops(requests.at(domain::RequestType::add_stop));
		}

		if (requests.count(domain::RequestType::add_route)) {
			AddRoutes(requests.at(domain::RequestType::add_route));
		}
	}
}

