#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

using namespace transport_catalogue;

int main() {

    TransportCatalogue catalogue;
    map_renderer::MapRenderer map_renderer;
    request_handler::RequestHandler rh(catalogue, map_renderer);
    json_reader::JsonReader json_doc(std::cin, rh);
    json_doc.ProcessRequests();
    //rh.RenderMap().Render(std::cout);
}