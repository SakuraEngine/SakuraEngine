if(TARGET_WIN)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/amd_ags.zip
        ${SAKURA_BIN_DIR}/Debug
        ${SAKURA_BIN_DIR}/Release
    )
    
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/nvapi.zip 
        ${SAKURA_BIN_DIR}/Debug
        ${SAKURA_BIN_DIR}/Release
    )
endif()