enable_testing()

add_executable(CGPUExts 
   VkDeviceExts.cpp 
   D3DDeviceExts.cpp
)

target_link_libraries(CGPUExts 
   gtest_main
)
gtest_discover_tests(CGPUExts)