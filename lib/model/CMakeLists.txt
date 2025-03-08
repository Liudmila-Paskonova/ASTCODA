add_library(model STATIC Model.cpp)
target_include_directories(model PUBLIC
    ${CMAKE_SOURCE_DIR}/include/model
)

target_link_libraries(model PUBLIC support tree_sitter Eigen3::Eigen)