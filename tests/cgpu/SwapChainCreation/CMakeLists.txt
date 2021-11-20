enable_testing()

add_executable(SwapChainCreation 
    SwapChainCreation.cpp 
)
target_link_libraries(SwapChainCreation 
   ${TEST_LIBRARIES}
)

gtest_discover_tests(SwapChainCreation)