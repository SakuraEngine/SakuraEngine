if(TARGET_WA)
else()
FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/*)

link_libraries(SakuraRuntime)

FOREACH(child ${children})
IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${child})
ENDIF()
ENDFOREACH()
endif()