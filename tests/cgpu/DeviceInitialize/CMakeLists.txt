enable_testing()

add_executable(DeviceInitialize 
   DeviceInitialize.cpp 
)

target_link_libraries(DeviceInitialize 
   gtest_main
)
gtest_discover_tests(DeviceInitialize)