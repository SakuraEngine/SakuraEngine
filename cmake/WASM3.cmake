if(TARGET_MAC OR TARGET_WIN)
    set(USE_WASM3 TRUE)
endif()

if(USE_WASM3)

if(TARGET_WIN)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/m3_d-win.zip ${SAKURA_BIN_DIR}/Debug)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/m3-win.zip ${SAKURA_BIN_DIR}/Release)
elseif(TARGET_MAC)
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64")
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/m3_d-macos-arm.zip ${SAKURA_BIN_DIR}/Debug)
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/m3-macos-arm.zip ${SAKURA_BIN_DIR}/Release)
    else()
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/m3_d-macos-amd64.zip ${SAKURA_BIN_DIR}/Debug)
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/m3-macos-amd64.zip ${SAKURA_BIN_DIR}/Release)
    endif()
endif(TARGET_WIN)

set(WASM3_LIBRARIES m3 uv_a uvwasi_a)
set(WASM3_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/wasm3)

endif(USE_WASM3)