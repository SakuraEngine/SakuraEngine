#pragma once
#include "SkrBase/meta.h"
// #ifndef __meta__
//     #include "MetaTest/test_check_structure.generated.h"
// #endif

namespace test_ns sreflect
{
// test expand path
sreflect_struct(
    "test_expand_path": {
        "a": "expand_a",
        "b::b": "expand_b",
        "c::c::c": "expand_c"
    }
) TestExpandPath {
};

// test shorthand expand

// test type error
sreflect_struct(
    "test_check_structure": {
        "int": false,
        "float": "f",
        "str": 100,
        "bool": "shit",
        "sub" : 10
    },
    "test_check_structure::sub::count": "100"
)
TestCheckStructure {
};

// test unrecognized attr
sreflect_struct(
    "test_unrecognized_attr": {
        "a": "a",
        "b": "b",
        "c": "c",
        "sub": {
            "sub_a": "sub_a",
            "sub_b": "sub_b",
            "sub_c": "sub_c"
        },
        "sub::sub_a!": "sub_a!",
        "SuB::sUb_A": "FuCK",
        "sub::sub_bbbb": "sub_bbb"
    }
) TestUnrecognizedAttr {
};

//

} // namespace test_ns sreflect
