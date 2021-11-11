add_executable(WindowExts win_ext.cpp ${SAMPLE_SHARED_SOURCE})
target_link_libraries(WindowExts SakuraRuntime)
target_compile_definitions(WindowExts PRIVATE -D_UNICODE -DUNICODE)