#pragma once

#include "transport_catalogue.h"

#include <iostream>

namespace transport_catalogue {
namespace stat_reader {

void ProcessRequests(std::ostream& out, TransportCatalogue& catalogue);

}// namespace stat_reader
}// namespace transport_catalogue