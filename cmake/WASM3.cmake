if(TARGET_MAC OR TARGET_WIN)
    set(USE_WASM3 TRUE)
endif()

if(USE_WASM3)


if(TARGET_WIN)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/m3_d-win.zip ${SAKURA_BIN_DIR}/Debug)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/m3-win.zip ${SAKURA_BIN_DIR}/Release)
endif(TARGET_WIN)

set(WASM3_LIBRARIES m3 uv_a uvwasi_a)
set(WASM3_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/wasm3)

endif(USE_WASM3)