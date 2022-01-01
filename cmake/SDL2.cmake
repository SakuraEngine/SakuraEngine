if(TARGET_MAC OR TARGET_WIN OR TARGET_EMSCRIPTON)
    set(CAN_USE_SDL2 TRUE)
endif()

if(CAN_USE_SDL2)

    if(TARGET_EMSCRIPTON)
        set(USE_FLAGS "-s USE_SDL=2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    elseif(UNIX)
        find_package(SDL2 REQUIRED)
    elseif(TARGET_WIN)
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/SDL2-win.zip 
            ${SAKURA_BIN_DIR}/Debug
            ${SAKURA_BIN_DIR}/Release
        )
        set(SDL2_LIBRARIES SDL2 SDL2main)
    endif(TARGET_EMSCRIPTON)

    set(SDL2_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2)

endif(CAN_USE_SDL2)