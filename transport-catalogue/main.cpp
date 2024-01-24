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

        std::ifstream in("make_base.json"s);
        json_reader::JsonReader json_reader(in, std::cout, json_reader::make_base);

    }
    else if (mode == "process_requests"sv) {

        std::ifstream in("process_requests.json"s);
        std::ofstream out("result.json"s);

        json_reader::JsonReader json_reader(in, out, json_reader::process_requests);

    }
    else {
        PrintUsage();
        return 1;
    }
}