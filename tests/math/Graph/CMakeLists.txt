enable_testing()

add_executable(Graph 
    Graph.cpp 
)
target_link_libraries(Graph 
    ${TEST_LIBRARIES}
)

gtest_discover_tests(Graph)