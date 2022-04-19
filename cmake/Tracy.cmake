if(TARGET_WIN)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/tracyclient-win.zip 
        ${SAKURA_BIN_DIR}/Debug
    )
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/tracyclient-win.zip 
        ${SAKURA_BIN_DIR}/Release
    )
elseif(TARGET_MAC)
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64")
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/tracyclient-macos-arm.zip
            ${SAKURA_BIN_DIR}/Debug
        )
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/tracyclient-macos-arm.zip
            ${SAKURA_BIN_DIR}/Release
        )
    else()
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/tracyclient-macos-amd64.zip 
            ${SAKURA_BIN_DIR}/Debug
        )
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/tracyclient-macos-amd64.zip 
            ${SAKURA_BIN_DIR}/Release
        )
    endif()
    execute_process(COMMAND chmod a+x ${CMAKE_CURRENT_SOURCE_DIR}/SDKs/tracy/osx/Tracy)
endif(TARGET_WIN)

set(TRACY_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tracy)
set(TRACY_LIBRARIES TracyClient)