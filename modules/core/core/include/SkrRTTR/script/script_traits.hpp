#pragma once
#include "SkrRTTR/export/export_data.hpp"
#include "SkrBase/meta.h"

namespace skr::rttr
{
template <typename T>
struct ScriptTraits {
    template <typename Backend>
    static void export_record(RecordData* record_data)
    {
        unimplemented_no_meta(T, "ScriptTraits::export_record is not implemented");
    }
    template <typename Backend>
    static void export_enum(FieldData* field_data)
    {
        unimplemented_no_meta(T, "ScriptTraits::export_enum is not implemented");
    }
    template <typename Backend>
    static void export_function(FunctionData* function_data)
    {
        unimplemented_no_meta(T, "ScriptTraits::export_function is not implemented");
    }
};

// TODO. primitive type export
//  类型可以指定 prefer export type，一般用于如 float3 这类的导出
//  类型可以直接关闭某种类型的导出，主要用于完全 primitive 的情况
} // namespace skr::rttr