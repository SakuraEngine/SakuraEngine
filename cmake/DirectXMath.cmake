set(DXMATH_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/DirectXMath/DirectXMath CACHE STRING INTERNAL FORCE)
file(GLOB_RECURSE dxmath_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/DirectXMath/DirectXMath/*.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/DirectXMath/DirectXMath/*.hpp
)

if(UNIX)
    set(DXMATH_INCLUDES_DIR ${DXMATH_INCLUDES_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/DirectXMath/Unix CACHE STRING INTERNAL FORCE)
    file(GLOB_RECURSE dxmath_sal_headers
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/DirectXMath/Unix/*.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/DirectXMath/Unix/*.hpp
    )
endif(UNIX)

set(dxmath_headers ${dxmath_headers} ${dxmath_sal_headers})
set(DXMATH_HEADERS ${dxmath_headers} CACHE STRING INTERNAL FORCE)