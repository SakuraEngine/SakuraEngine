enable_testing()

link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})

add_executable(SwapChainCreation 
    SwapChainCreation.cpp 
)
target_link_libraries(SwapChainCreation 
   gtest_main
)

gtest_discover_tests(SwapChainCreation)