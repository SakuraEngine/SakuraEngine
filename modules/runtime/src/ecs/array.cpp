#include "ecs/array.hpp"

extern "C" {
void* dualA_begin(dual_array_component_t* array)
{
    return array->BeginX;
}

void* dualA_end(dual_array_component_t* array)
{
    return array->EndX;
}
}