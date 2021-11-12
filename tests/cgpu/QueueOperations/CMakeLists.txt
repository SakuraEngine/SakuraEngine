enable_testing()

add_executable(QueueOperations 
    QueueOperations.cpp 
)

target_link_libraries(QueueOperations 
   gtest_main
)
gtest_discover_tests(QueueOperations)