enable_testing()

add_executable(ShaderCreation 
   ShaderCreation.cpp 
)
target_link_libraries(ShaderCreation 
   gtest_main
)

gtest_discover_tests(ShaderCreation)