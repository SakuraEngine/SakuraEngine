#pragma once

// TODO. better assert
#ifndef SKR_ASSERT
    #pragma warning "SKR_ASSERT is not defined, use default assert"
    #ifdef _DEBUG
        #include <assert.h>
        #ifndef SKR_ASSERT
        #define SKR_ASSERT(expr) assert(expr)
        #endif
    #else
        #ifndef assert
            #define SKR_ASSERT(expr) (void)(expr);
        #endif
    #endif
#endif