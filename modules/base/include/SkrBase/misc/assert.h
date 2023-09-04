#pragma once

// TODO. better assert
#ifndef SKR_ASSERT
    #warning "SKR_ASSERT is not defined, use default assert"
    #ifdef _DEBUG
        #include <assert.h>
        #define SKR_ASSERT(expr) assert(expr)
    #else
        #ifndef assert
            #define SKR_ASSERT(expr) (void)(expr);
        #endif
    #endif
#endif