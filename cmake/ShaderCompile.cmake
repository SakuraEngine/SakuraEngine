if(TARGET_MAC OR TARGET_WIN)
    set(USE_SHADER_COMPILER TRUE)
endif()

if(USE_SHADER_COMPILER)

if(TARGET_WIN)
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/dxc-win.zip 
        ${SAKURA_BIN_DIR}/Debug
    )
    extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/dxc-win.zip 
        ${SAKURA_BIN_DIR}/Release
    )
elseif(TARGET_MAC)
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64")
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/dxc-macos-arm.zip
            ${SAKURA_BIN_DIR}/Debug
        )
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/dxc-macos-arm.zip
            ${SAKURA_BIN_DIR}/Release
        )
    else()
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/dxc-macos-amd64.zip 
            ${SAKURA_BIN_DIR}/Debug
        )
        extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/dxc-macos-amd64.zip 
            ${SAKURA_BIN_DIR}/Release
        )
    endif()
    execute_process(COMMAND chmod a+x ${SAKURA_BIN_DIR}/Release/dxc)
    execute_process(COMMAND chmod a+x ${SAKURA_BIN_DIR}/Debug/dxc)
endif(TARGET_WIN)

endif(USE_SHADER_COMPILER)

# generates a build target that will compile shaders for a given config file
#
# usage: sakura_compile_shaders(TARGET <generated build target name>
#                               SOURCES ...
#                              [DXIL <dxil-output-path>]
#                              [SPIRV_DXC <spirv-output-path>])
function(sakura_compile_shaders)
    set(options "")
    set(oneValueArgs TARGET CONFIG FOLDER DXIL SPIRV_DXC CFLAGS)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(params "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT params_TARGET)
        message(FATAL_ERROR "sakura_compile_shaders: TARGET argument missing")
    endif()

    # just add the source files to the project as documents, they are built by the script
    set_source_files_properties(${params_SOURCES} PROPERTIES VS_TOOL_OVERRIDE "None") 

    add_custom_target(${params_TARGET} SOURCES ${params_SOURCES})

    if (params_DXIL AND TARGET_WIN)
        if (NOT params_CFLAGS)
            set(CFLAGS "$<IF:$<CONFIG:Debug>,-Zi -Qembed_debug,-Qstrip_debug -Qstrip_reflect> -O3 -WX -Zpr")
        else()
            set(CFLAGS ${params_CFLAGS})
        endif()

        foreach(source ${params_SOURCES})
            get_filename_component(OUTPUT_FILE_WLE ${source} NAME_WLE)
            get_filename_component(OUTPUT_FILE_WLE ${OUTPUT_FILE_WLE} EXT)
            get_filename_component(PURE_FILE_NAME ${source} NAME_WE)
            string(REPLACE "." "" TargetProp "${OUTPUT_FILE_WLE}")
            add_custom_command(TARGET ${params_TARGET} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${params_DXIL}
                COMMAND ${SAKURA_BIN_DIR}/Release/dxc 
                        -T ${TargetProp}
                        -Fo ${params_DXIL}/${PURE_FILE_NAME}.dxil
                        -Fh ${params_DXIL}/${PURE_FILE_NAME}.dxil.h
                        ${source}
                WORKING_DIRECTORY ${SAKURA_BIN_DIR}/Release)
        endforeach(source ${params_SOURCES})
    endif()

    if (params_SPIRV_DXC AND BUILD_SPIRV_SHADER)
        if (NOT params_CFLAGS)
            set(CFLAGS "$<IF:$<CONFIG:Debug>,-Zi,> -fspv-target-env=vulkan1.2 -O3 -WX -Zpr")
        else()
            set(CFLAGS ${params_CFLAGS})
        endif()

        foreach(source ${params_SOURCES})
            get_filename_component(OUTPUT_FILE_WLE ${source} NAME_WLE)
            get_filename_component(OUTPUT_FILE_WLE ${OUTPUT_FILE_WLE} EXT)
            get_filename_component(PURE_FILE_NAME ${source} NAME_WE)
            string(REPLACE "." "" TargetProp "${OUTPUT_FILE_WLE}")
            add_custom_command(TARGET ${params_TARGET} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${params_SPIRV_DXC}
                COMMAND ${SAKURA_BIN_DIR}/Release/dxc 
                        -T ${TargetProp}
                        -Fo ${params_SPIRV_DXC}/${PURE_FILE_NAME}.spv
                        -Fh ${params_SPIRV_DXC}/${PURE_FILE_NAME}.spv.h
                        -spirv
                        -fspv-target-env=vulkan1.1
                        ${source}
                WORKING_DIRECTORY ${SAKURA_BIN_DIR}/Release)
        endforeach(source ${params_SOURCES})
    endif()

    if(params_FOLDER)
        set_target_properties(${params_TARGET} PROPERTIES FOLDER ${params_FOLDER})
    endif()
endfunction()