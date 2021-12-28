enable_testing()

add_executable(BasicExecution 
   BasicExecution.cpp 
)

target_link_libraries(BasicExecution 
   ${TEST_LIBRARIES}
)
gtest_discover_tests(BasicExecution)