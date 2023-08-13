#include "transport_catalogue.h"
#include "stat_reader.h"
#include "input_reader.h"

using namespace transport_catalogue;

int main() {
    TransportCatalogue catalogue;
    input_reader::ParseRequestsToUpdate(std::cin, catalogue);
    stat_reader::ProcessRequests(std::cout, catalogue);
}