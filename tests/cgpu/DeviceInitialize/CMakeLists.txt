enable_testing()

add_executable(DeviceInitialize 
   DeviceInitialize.cpp 
)

target_link_libraries(DeviceInitialize 
   ${TEST_LIBRARIES}
)
gtest_discover_tests(DeviceInitialize)