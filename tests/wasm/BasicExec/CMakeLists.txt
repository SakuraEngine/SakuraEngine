add_executable(BasicExecution 
   BasicExecution.cpp 
)
add_executable(ExecutionX 
   ExecutionX.cpp 
)

target_link_libraries(ExecutionX 
   ${TEST_LIBRARIES}
)
target_link_libraries(BasicExecution 
   ${TEST_LIBRARIES}
)
gtest_discover_tests(BasicExecution)
gtest_discover_tests(ExecutionX)