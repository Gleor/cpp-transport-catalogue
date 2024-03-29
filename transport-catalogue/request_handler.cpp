#include "request_handler.h"
#include "transport_catalogue.h"

namespace request_handler {

	void RequestHandler::InitializeMapRenderer()
	{
		if (!map_renderer_) {
			map_renderer_ = std::make_shared<map_renderer::MapRenderer>(renderer_settings_);
		}
	}

	void RequestHandler::InitializeRouter()
	{
		if (!transport_router_) {
			transport_router_ = std::make_unique<transport_catalogue::router::TransportRouter>(transport_catalogue_, router_settings_);
		}
	}

	svg::Document RequestHandler::RenderMap()
	{
		InitializeMapRenderer();

		return map_renderer_.get()->RenderMap(transport_catalogue_.GetBusesMap());
	}

	const std::string RequestHandler::GetMap()
	{
		std::ostringstream strm;
		RenderMap().Render(strm);

		return strm.str();
	}

	void RequestHandler::SetMapRenderSettings(map_renderer::RendererSettings&& settings)
	{
		renderer_settings_ = std::move(settings);

		InitializeMapRenderer();

		map_renderer_.get()->SetSettings(renderer_settings_);
	}

	void RequestHandler::SetRouterSettings(transport_catalogue::router::RouterSettings&& settings)
	{
		router_settings_ = std::move(settings);

		InitializeRouter();

		transport_router_.get()->SetRouterSettings(router_settings_);
	}

	void RequestHandler::InitializeTransportRouterGraph()
	{
		transport_router_->ImportRoutingDataFromCatalogue();
	}

	domain::Stop RequestHandler::MakeStop(domain::Request& request) {
		return { std::string(request.name_), request.coordinates_.lat, request.coordinates_.lng };
	}

	domain::Bus RequestHandler::MakeBus(domain::Request& request) {
		domain::Bus result;
		result.is_circular_ = request.is_circular_;
		result.bus_name_ = request.name_;
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

	void RequestHandler::AddBuses(std::vector<domain::Request>& requests)
	{
		for (domain::Request& request : requests) {
			transport_catalogue_.AddBus(std::move(MakeBus(request)));
		}
	}

	domain::StopStat* RequestHandler::GetStop(const std::string_view stop)
	{
		return transport_catalogue_.GetBusesForStopInfo(stop);
	}

	domain::BusStat* RequestHandler::GetBus(const std::string_view route)
	{
		return transport_catalogue_.GetBusInfo(route);
	}

	domain::RouteStat RequestHandler::GetRoute(const std::string_view from, const std::string_view to)
	{
		return transport_router_.get()->MakeRoute(from, to);
	}

	void RequestHandler::HandleBaseRequests(domain::RequestsMap&& requests)
	{
		if (requests.count(domain::RequestType::add_stop)) {
			AddStops(requests.at(domain::RequestType::add_stop));
		}

		if (requests.count(domain::RequestType::add_bus)) {
			AddBuses(requests.at(domain::RequestType::add_bus));
		}
	}
	
	bool RequestHandler::SerializeData(std::ostream& output)
	{
		if (!serializer_) {                                              
			serializer_ = std::make_shared<transport_catalogue::serialize::Serializator>(
				transport_catalogue_,
				router_settings_,
				renderer_settings_);
		}

		serializer_->GetDataFromCatalogue();                                 

		
		if (transport_router_) {                                         
			serializer_->SetRouter(transport_router_);             
			serializer_->GetDataFromRouter();                                
		}
		
		return serializer_->Serialize(output);
	}
	bool RequestHandler::DeserializeData(std::istream& input)
	{
		if (!serializer_) {
			serializer_ = std::make_shared<transport_catalogue::serialize::Serializator>(
				transport_catalogue_,
				router_settings_,
				renderer_settings_);
		}

		if (serializer_->Deserialize(input)) {

			serializer_->ApplyDataToCatalogue();                         
			InitializeRouter();                                    
			serializer_->SetRouter(transport_router_);         
			serializer_->ApplyDataToRouter();                           

			return true;
		}

		return false;

	}
	
}

