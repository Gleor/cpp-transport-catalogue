cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

set(SYSTEM_LIBS -lstdc++)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

set(PROTO_FILES transport_catalogue.proto map_renderer.proto svg.proto transport_router.proto graph.proto)
set(HEADER_FILES json.h domain.h json_reader.h json_builder.h geo.h svg.h map_renderer.h serialization.h ranges.h router.h graph.h transport_router.h transport_catalogue.h request_handler.h)
set(SRC_FILES json.cpp json_builder.cpp json_reader.cpp geo.cpp svg.cpp map_renderer.cpp serialization.cpp transport_router.cpp transport_catalogue.cpp request_handler.cpp domain.cpp main.cpp)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${PROTO_FILES} ${HEADER_FILES} ${SRC_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)