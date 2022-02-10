#include "cgpu/cgpux.hpp"
#include "math/scalarmath.h"
#include "math/vectormath.hpp"
#include <iostream>

int main(void)
{
    sakura::math::float4x4 M = sakura::math::look_at_matrix(
        sakura::math::Vector3f::vector_zero(),
        sakura::math::Vector3f::vector_one());
    std::cout << M.data_view()[0] << " " << M.data_view()[1] << " " << M.data_view()[2] << " " << M.data_view()[3] << std::endl;
    std::cout << M.data_view()[4] << " " << M.data_view()[5] << " " << M.data_view()[6] << " " << M.data_view()[7] << std::endl;
    std::cout << M.data_view()[8] << " " << M.data_view()[9] << M.data_view()[10] << " " << M.data_view()[11] << " " << std::endl;
    std::cout << M.data_view()[12] << " " << M.data_view()[13] << " " << M.data_view()[14] << " " << M.data_view()[15] << std::endl;
    return 0;
}