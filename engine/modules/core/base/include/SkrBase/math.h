#pragma once

#ifdef __cplusplus
    #include "./math/gen/gen_math.hpp" // IWYU pragma: export

    #include "./math/manual/quat.hpp"              // IWYU pragma: export
    #include "./math/manual/transform.hpp"         // IWYU pragma: export
    #include "./math/manual/rotator.hpp"           // IWYU pragma: export
    #include "./math/manual/matrix.hpp"            // IWYU pragma: export
    #include "./math/manual/vector.hpp"            // IWYU pragma: export
    #include "./math/manual/matrix_utils.hpp"      // IWYU pragma: export
    #include "./math/manual/color_utils.hpp"       // IWYU pragma: export
    #include "./math/manual/camera_utils.hpp"      // IWYU pragma: export
    #include "./math/manual/coordinate_system.hpp" // IWYU pragma: export
    #include "./math/manual/ease_curve.hpp"        // IWYU pragma: export
    #include "./math/manual/math_type_info.hpp"    // IWYU pragma: export

    #include "./math/gen/gen_math_c_decl.hpp"        // IWYU pragma: export
    #include "./math/gen/gen_math_traits.hpp"        // IWYU pragma: export
    #include "./math/gen/gen_math_memory_traits.hpp" // IWYU pragma: export

#else
    #include "./math/gen//gen_math_c_decl.h" // IWYU pragma: export
#endif