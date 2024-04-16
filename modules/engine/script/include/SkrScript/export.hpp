#pragma once
#include "SkrScript/export_data.hpp"

namespace skr::script
{
template <typename T, typename Backend>
struct RecordBuilder {

private:
    RecordData _data;
};

template <typename T, typename Backend>
struct FunctionBuilder {
};

template <typename T, typename Backend>
struct EnumBuilder {
};

} // namespace skr::script