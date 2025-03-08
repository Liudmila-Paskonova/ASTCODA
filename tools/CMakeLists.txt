add_executable(extract extract.cpp)
target_link_libraries(extract PRIVATE extractor arg_parser)

add_executable(vocab vocabs.cpp)
target_link_libraries(vocab PRIVATE  arg_parser vocabulary)

add_executable(evaluate evaluate.cpp)
target_link_libraries(evaluate PUBLIC model arg_parser)

#set(CMAKE_AUTOMOC ON)
#add_executable(testmarker testmarker.cpp)
#target_link_libraries(testmarker PRIVATE marker db tree_sitter arg_parser)