#include "SkrGraphics/config.h"
#if _WIN32
    #include "dstorage/windows/dstorage.cpp"
    #include "dstorage/windows/dstorage_decompress.cpp"
#else
    #include "dstorage/null/dstorage.cpp"
#endif