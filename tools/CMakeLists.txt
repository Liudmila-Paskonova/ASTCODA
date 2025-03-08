add_executable(extract extract.cpp)
target_link_libraries(extract PRIVATE extractor arg_parser)
target_link_libraries(extract PRIVATE nlohmann_json::nlohmann_json)

add_executable(vocab vocabs.cpp)
target_link_libraries(vocab PRIVATE  arg_parser vocabulary)
target_link_libraries(vocab PRIVATE nlohmann_json::nlohmann_json)

add_executable(evaluate evaluate.cpp)
target_link_libraries(evaluate PUBLIC model arg_parser)
target_link_libraries(evaluate PRIVATE nlohmann_json::nlohmann_json)

#set(CMAKE_AUTOMOC ON)
#add_executable(testmarker testmarker.cpp)
#target_link_libraries(testmarker PRIVATE marker db tree_sitter arg_parser)