#pragma once
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.h"

#include <transport_catalogue.pb.h>

#include <iostream>
#include <fstream>
#include <cassert>

namespace transport_catalogue {

	namespace serialize {

		class Serializator {
		private:
			transport_catalogue::TransportCatalogue& transport_catalogue_;                             
			transport_catalogue::router::RouterSettings& router_settings_;                             
			map_renderer::RendererSettings& renderer_settings_;                   
			std::shared_ptr<router::TransportRouter> transport_router_ = nullptr;                         

		public:

			Serializator() = delete;                                                                    
			Serializator(transport_catalogue::TransportCatalogue&
				, router::RouterSettings&, map_renderer::RendererSettings&);                             

			Serializator& SetTransporCatalogue(transport_catalogue::TransportCatalogue&);            
			Serializator& SetRendererSettings(map_renderer::RendererSettings&);                      
			Serializator& SetRouterSettings(router::RouterSettings&);                                
			Serializator& SetRouter(std::shared_ptr<router::TransportRouter>);              

			Serializator& GetDataFromCatalogue();                                                      
			Serializator& GetDataFromRouter();                                                         
			Serializator& ApplyDataToCatalogue();                                                     
			Serializator& ApplyDataToRouter();                                                        

			bool Serialize(std::ostream&);                                                    
			bool Deserialize(std::istream&);                                                                                                  

		private:
                                            
			transport_catalogue_serialize::TransportCatalogue serialization_data_;                             

			void SerializeColor(
				const svg::Color&, transport_catalogue_serialize::Color*);                             
			bool SerializeGraphs(transport_catalogue_serialize::RouterData*);                      

			bool SerializeStopsData();                                                            
			bool SerializeBusesData();                                                            
			bool SerializeDistancesData();                                                        
			bool SerializeRendererSettings();                                                      
			bool SerializeRouterSettings();                                                        
			bool SerializeRouterData();                                                            

			svg::Color DeseserializeColor(
				const transport_catalogue_serialize::Color&);
			bool DeserializeStopsData();                                                         
			bool DeserializeBusesData();                                                         
			bool DeserializeDistancesData();                                                     
			bool DeserializeRendererSettings();                                                   
			bool DeserializeRouterSettings();                                                     
			bool DeserializeRouterData();                                                         

		};

	}   

}   