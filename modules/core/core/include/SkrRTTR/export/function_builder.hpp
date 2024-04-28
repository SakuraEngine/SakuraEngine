#pragma once
#include "SkrRTTR/export/export_data.hpp"

namespace skr::rttr
{
struct FunctionBuilder {
    FunctionBuilder(FunctionData* data);

    // bind
    template <auto func>
    inline FunctionBuilder& bind(String name)
    {
        // TODO. parse namesapce
        _data->name = std::move(name);
        _data->fill_signature(func);
        return *this;
    }

    // param
    inline FunctionBuilder& param(uint64_t index, String name, ParamModifier modifier = ParamModifier::In, ParamData::MakeDefaultFunc default_func = nullptr)
    {
        auto& param_data        = _data->param_data[index];
        param_data.name         = std::move(name);
        param_data.modifier     = modifier;
        param_data.make_default = default_func;
        return *this;
    }

protected:
    FunctionData* _data;
};
} // namespace skr::rttr