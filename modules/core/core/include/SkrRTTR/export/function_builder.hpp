#pragma once
#include "SkrRTTR/export/export_data.hpp"

namespace skr::rttr
{
template <typename Backend>
struct FunctionBuilder {
    FunctionBuilder(FunctionData* data);

    // bind
    template <auto func>
    inline FunctionBuilder& bind(String name)
    {
        // split namespace
        sequence<text_view> splitted;
        auto                count = name.split(u8"::", splitted);

        // last part is name
        _data->name = splitted.access_at(splitted.size() - 1);

        // fill namespace
        if (count > 1)
        {
            _data->name_space.reserve(count - 1);
            for (auto i = 0; i < count - 1; ++i)
            {
                _data->name_space.push_back(splitted.access_at(i));
            }
        }

        // fill signature
        _data->fill_signature(func);

        // fill invoke
        _data->invoke = Backend::template export_function<func>();

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