find_package(GTest)

if(GTest_FOUND)
  set(TEST_LIBRARIES GTest::Main)
else()
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    message(STATUS ${CMAKE_CURRENT_BINARY_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                    ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                    EXCLUDE_FROM_ALL)

    # The gtest/gtest_main targets carry header search path
    # dependencies automatically when using CMake 2.8.11 or
    # later. Otherwise we have to add them here ourselves.
    if (CMAKE_VERSION VERSION_LESS 2.8.11)
      include_directories("${gtest_SOURCE_DIR}/include")
    endif()
  set(TEST_LIBRARIES gtest_main)
endif(GTest_FOUND)

if(BUILD_TEST)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared)
    file(GLOB_RECURSE SAMPLE_SHARED_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/*.h ${CMAKE_CURRENT_SOURCE_DIR}/shared/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/*.c ${CMAKE_CURRENT_SOURCE_DIR}/shared/*.cpp
    )

    add_subdirectory(math)
    add_subdirectory(memory)
    add_subdirectory(render-setup)
    add_subdirectory(window-exts)
    add_subdirectory(cgpu)
endif()