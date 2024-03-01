#if !defined(__wasi__) && !defined(__EMSCRIPTEN__)
    #include "common/swa.c"
    #ifdef USE_M3
        #include "wasm3/swa_wasm3.c"
    #endif
    #ifdef USE_WASM_EDGE
        #include "wasmedge/swa_wasmedge.c"
    #endif
#endif