add_library(arg_parser STATIC ArgParser.cpp)
target_include_directories(arg_parser PUBLIC
    ${CMAKE_SOURCE_DIR}/include/support/ArgParser
)

target_link_libraries(arg_parser PRIVATE nlohmann_json::nlohmann_json)
