#pragma once
#include "SkrBase/meta.h"
// #ifndef __meta__
//     #include "MetaTest/test_check_structure.generated.h"
// #endif

namespace test_path_shorthand
{
sreflect_struct(
    "test_expand_path": {
        "a": "expand_a",
        "b": {"b": "bad_value"},
        "b::b!": "expand_b",
        "c": {"c": {"c": "bad_value"}},
        "c::c::c!": "expand_c"
    }
) PassCheck {
};

// sreflect_struct(
//     "test_expand_path": {
//         "a::a::a": "error",
//         "b": {"b": "error"},
//         "c": "error"
//     }
// ) ErrorCase {
// };
} // namespace test_path_shorthand

namespace test_functional_shorthand
{
sreflect_struct(
    "test_functional_shorthand": {
        "test_enable": false,
        "test_enable::enable!": true,
        "test_usual": "all",
        "test_override": ["all_cat", "all_dog"],
        "test_override!": "except_c",
        "test_override::dog_c": true
    }
) PassCheck {
};

// sreflect_struct(
//     "test_functional_shorthand" : {
//         "test_usual": "all",
//         "test_usual::a": false
//     }
// )
// sattr("test_functional_shorthand::test_usual": "None"
// ) ErrorCase {
// };

}; // namespace test_functional_shorthand
