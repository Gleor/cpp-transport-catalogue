#include <fstream>
#include <iostream>
#include <string_view>
#include "json_reader.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        json_reader::JsonReader j_reader(std::cin, json_reader::make_base);

    } else if (mode == "process_requests"sv) {

        json_reader::JsonReader j_reader(std::cin, json_reader::process_requests);

    } else {
        PrintUsage();
        return 1;
    }
}