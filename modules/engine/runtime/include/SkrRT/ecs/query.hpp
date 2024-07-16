#pragma once
#include "SkrRT/ecs/sugoi.h"

struct sugoi_query_t
{
    struct Impl;
    Impl* pimpl = nullptr;
};