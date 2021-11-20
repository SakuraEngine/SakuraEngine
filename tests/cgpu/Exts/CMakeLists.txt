enable_testing()

add_executable(CGPUExts 
   VkDeviceExts.cpp 
   D3DDeviceExts.cpp
)

target_link_libraries(CGPUExts 
   ${TEST_LIBRARIES}
)
gtest_discover_tests(CGPUExts)