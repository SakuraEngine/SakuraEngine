if(TARGET_MAC OR TARGET_WIN)
    set(CAN_USE_SDL2 TRUE)
endif()

if(CAN_USE_SDL2)

if(UNIX)
    set(SDL2_SYSTEM_PACKAGE TRUE)
else()
    set(SDL2_SYSTEM_PACKAGE FALSE)
endif(UNIX)

if(SDL2_SYSTEM_PACKAGE)
    find_package(SDL2 REQUIRED)
elseif(TARGET_WIN)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/SDL2-win.zip ${SAKURA_BIN_DIR}/Debug)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/SDL2-win.zip ${SAKURA_BIN_DIR}/Release)
    set(SDL2_LIBRARIES SDL2 SDL2main)
    set(SDL2_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2)
endif()

link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})
endif(CAN_USE_SDL2)