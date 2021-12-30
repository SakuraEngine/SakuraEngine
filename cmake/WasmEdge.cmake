if(TARGET_WIN OR TARGET_MAC)
    set(USE_WASM_EDGE TRUE)
endif()

if(USE_WASM_EDGE)

if(TARGET_WIN)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/WasmEdge-win.zip ${SAKURA_BIN_DIR}/Debug)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/WasmEdge-win.zip ${SAKURA_BIN_DIR}/Release)
elseif(TARGET_MAC)
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64")
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/WasmEdge-macos-arm.zip ${SAKURA_BIN_DIR}/Debug)
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/WasmEdge-macos-arm.zip ${SAKURA_BIN_DIR}/Release)
    else()
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/WasmEdge-macos-amd64.zip ${SAKURA_BIN_DIR}/Debug)
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/WasmEdge-macos-amd64.zip ${SAKURA_BIN_DIR}/Release)
    endif()
endif(TARGET_WIN)

set(WASM_EDGE_LIBRARIES wasmedge_c)
set(WASM_EDGE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/WasmEdge)

endif(USE_WASM_EDGE)