#pragma once
#include "SkrBase/meta.h"
// #ifndef __meta__
//     #include "MetaTest/test_check_structure.generated.h"
// #endif

namespace test_ns sreflect
{
sreflect_struct(
    "test": {
        "int": false,
        "float": "f",
        "str": 100,
        "bool": "shit",
        "sub" : 10
    },
    "test::sub::count": "100"
)
TestRecord {
};

} // namespace test_ns sreflect
