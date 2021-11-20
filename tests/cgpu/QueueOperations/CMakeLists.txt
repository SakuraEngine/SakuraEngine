enable_testing()

add_executable(QueueOperations 
    QueueOperations.cpp 
)

target_link_libraries(QueueOperations 
   ${TEST_LIBRARIES}
)
gtest_discover_tests(QueueOperations)