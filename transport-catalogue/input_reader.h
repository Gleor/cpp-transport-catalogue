#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <iostream>

namespace transport_catalogue {

namespace input_reader {

std::vector<std::pair<std::string_view, int>> ParseStopDistance(std::string_view text);

std::pair<Stop, bool> ParseStop(const std::string& text);

Route ParseRoute(std::string_view text, TransportCatalogue& catalogue);

void ParseRequestsToUpdate(std::istream& input, TransportCatalogue& catalogue);

}// namespace input_reader
}// namespace transport_catalogue