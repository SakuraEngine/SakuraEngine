#ifdef SKR_USE_WASM

#if !defined(__EMSCRIPTEN__) && !defined(__wasi__)
    #include "common/swa.cpp"
    #ifdef USE_M3
        #include "wasm3/swa_wasm3.cpp"
    #endif
#endif

#endif