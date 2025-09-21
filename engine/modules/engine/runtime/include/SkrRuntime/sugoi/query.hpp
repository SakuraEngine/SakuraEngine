#pragma once
#include "SkrRuntime/sugoi/sugoi.h"

struct sugoi_query_t
{
    struct Impl;
    Impl* pimpl = nullptr;
};