#include "SkrRT/ecs/array.hpp"

extern "C" {
void* sugoiA_begin(sugoi_array_comp_t* array)
{
    return array->BeginX;
}

void* sugoiA_end(sugoi_array_comp_t* array)
{
    return array->EndX;
}
}