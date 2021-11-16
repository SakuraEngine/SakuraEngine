enable_testing()

add_executable(CommonMath 
    CommonMath.cpp 
)
target_link_libraries(CommonMath 
    gtest_main
)

gtest_discover_tests(CommonMath)