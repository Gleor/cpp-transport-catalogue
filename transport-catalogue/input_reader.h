#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <iostream>

namespace transport_catalogue {

namespace input_reader {

std::vector<std::pair<std::string_view, int>> ParseStopDistance(std::string_view);

std::pair<Stop, bool> ParseStop(const std::string&);

Route ParseRoute(std::string_view);

void ParseRequestsToUpdate(std::istream&, TransportCatalogue&);

}// namespace input_reader
}// namespace transport_catalogue