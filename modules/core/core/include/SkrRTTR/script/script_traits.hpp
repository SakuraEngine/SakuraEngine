#pragma once
#include "SkrRTTR/export/export_data.hpp"
#include "SkrBase/meta.h"

namespace skr::rttr
{
// TODO. 似乎不需要这个东西，直接混在 RTTR 内，纯动态导出是一个 Fallback 选择项，是全能且必须支持的，具体的 Script Export 是一个优化项，用于优化脚本胶水层
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