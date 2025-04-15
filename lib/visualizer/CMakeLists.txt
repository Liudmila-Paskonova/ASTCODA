#set(CMAKE_AUTOMOC ON)
#add_library(marker STATIC ${CMAKE_SOURCE_DIR}/include/visualizer/Marker.h Marker.cpp)
#target_link_libraries(marker PUBLIC Qt6::Widgets)
#target_compile_definitions(marker PUBLIC QT_QML_DEBUG)


set(CMAKE_AUTOMOC ON)
add_library(visualizer STATIC ${CMAKE_SOURCE_DIR}/include/visualizer/Visualizer.h Visualizer.cpp)
target_link_libraries(visualizer PUBLIC Qt6::Widgets)
target_compile_definitions(visualizer PUBLIC QT_QML_DEBUG)
