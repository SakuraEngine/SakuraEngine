enable_testing()

add_executable(ShaderCreation 
   ShaderCreation.cpp 
)
target_link_libraries(ShaderCreation 
   ${TEST_LIBRARIES}
)

gtest_discover_tests(ShaderCreation)