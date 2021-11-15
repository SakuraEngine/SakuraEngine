enable_testing()

add_executable(SwapChainCreation 
    SwapChainCreation.cpp 
)
target_link_libraries(SwapChainCreation 
   gtest_main
)

gtest_discover_tests(SwapChainCreation)