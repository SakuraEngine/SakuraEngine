enable_testing()

add_executable(CommonMath 
    CommonMath.cpp 
)
target_link_libraries(CommonMath 
    ${TEST_LIBRARIES}
)

gtest_discover_tests(CommonMath)