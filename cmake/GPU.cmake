if(TARGET_WIN)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/GPU/amd_ags.zip ${SAKURA_BIN_DIR}/Debug)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/GPU/amd_ags.zip ${SAKURA_BIN_DIR}/Release)
endif()