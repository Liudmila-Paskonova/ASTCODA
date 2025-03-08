add_library(vocabulary STATIC Vocabulary.cpp)
target_include_directories(vocabulary PUBLIC
    ${CMAKE_SOURCE_DIR}/include/vocab
)

target_link_libraries(vocabulary PUBLIC support)
target_link_libraries(vocabulary PRIVATE nlohmann_json::nlohmann_json)
