add_executable(extract extract.cpp)
target_link_libraries(extract PRIVATE extractor arg_parser nlohmann_json::nlohmann_json Threads::Threads)

add_executable(vocab vocabs.cpp)
target_link_libraries(vocab PRIVATE  arg_parser vocabulary nlohmann_json::nlohmann_json)

add_executable(evaluate evaluate.cpp)
target_link_libraries(evaluate PRIVATE model arg_parser support nlohmann_json::nlohmann_json)

set(CMAKE_AUTOMOC ON)
add_executable(visualize visualize.cpp)
target_link_libraries(visualize PRIVATE visualizer arg_parser)
