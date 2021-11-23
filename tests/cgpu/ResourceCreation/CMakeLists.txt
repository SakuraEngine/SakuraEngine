enable_testing()

add_executable(ResourceCreation 
   ResourceCreation.cpp 
)
target_link_libraries(ResourceCreation 
   ${TEST_LIBRARIES}
)

gtest_discover_tests(ResourceCreation)