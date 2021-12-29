#if !defined(__wasi__) && !defined(__EMSCRIPTEN__)
    #include "common/swa.c"
    #include "wasm3/swa_wasm3.c"
#endif