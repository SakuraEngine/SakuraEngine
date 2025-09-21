#pragma once
#include <SkrV8/bind_template/v8_bind_template.hpp>

namespace skr
{
struct V8BTPrimitive final : V8BindTemplate
{
    inline static constexpr EV8BTKind kKind = EV8BTKind::Primitive;

    static V8BTPrimitive* TryCreate(V8Isolate* isolate, GUID type_id);

    inline bool is_string() const
    {
        return _type_id == type_id_of<String>() ||
            _type_id == type_id_of<StringView>();
    }

    // basic info
    EV8BTKind kind() const override;
    String type_name() const override;
    String cpp_namespace() const override;

    // error process
    bool any_error() const override final;
    void dump_error(V8ErrorBuilderTreeStyle& builder) const override final;

    // convert helper
    v8::Local<v8::Value> to_v8(
        void* native_data
    ) const override final;
    bool to_native(
        void* native_data,
        v8::Local<v8::Value> v8_value,
        bool is_init
    ) const override final;

    // invoke native api
    bool match_param(
        v8::Local<v8::Value> v8_param
    ) const override final;
    void push_param_native(
        DynamicStack& stack,
        const V8BTDataParam& param_bind_tp,
        v8::Local<v8::Value> v8_value
    ) const override final;
    void push_param_native_pure_out(
        DynamicStack& stack,
        const V8BTDataParam& param_bind_tp
    ) const override final;
    v8::Local<v8::Value> read_return_native(
        DynamicStack& stack,
        const V8BTDataReturn& return_bind_tp
    ) const override final;
    v8::Local<v8::Value> read_return_from_out_param(
        DynamicStack& stack,
        const V8BTDataParam& param_bind_tp
    ) const override final;

    // invoke v8 api
    v8::Local<v8::Value> make_param_v8(
        void* native_data,
        const V8BTDataParam& param_bind_tp
    ) const override final;

    // field api
    v8::Local<v8::Value> get_field(
        void* obj,
        const RTTRType* obj_type,
        const V8BTDataField& field_bind_tp
    ) const override final;
    void set_field(
        v8::Local<v8::Value> v8_value,
        void* obj,
        const RTTRType* obj_type,
        const V8BTDataField& field_bind_tp
    ) const override final;
    v8::Local<v8::Value> get_static_field(
        const V8BTDataStaticField& field_bind_tp
    ) const override final;
    void set_static_field(
        v8::Local<v8::Value> v8_value,
        const V8BTDataStaticField& field_bind_tp
    ) const override final;

    // check api
    void solve_invoke_behaviour(
        const V8BTDataParam& param_bind_tp,
        bool& appare_in_return,
        bool& appare_in_param
    ) const override final;
    bool check_param(
        const V8BTDataParam& param_bind_tp,
        V8ErrorCache& errors
    ) const override final;
    bool check_return(
        const V8BTDataReturn& return_bind_tp,
        V8ErrorCache& errors
    ) const override final;
    bool check_field(
        const V8BTDataField& field_bind_tp,
        V8ErrorCache& errors
    ) const override final;
    bool check_static_field(
        const V8BTDataStaticField& field_bind_tp,
        V8ErrorCache& errors
    ) const override final;

    // v8 export
    bool has_v8_export_obj() const override final;
    v8::Local<v8::Value> get_v8_export_obj() const override final;
    void dump_ts_def(
        TSDefBuilder& builder
    ) const override final;
    String get_ts_type_name() const override final;
    bool ts_is_nullable() const override final;

private:
    template <typename T>
    void _setup()
    {
        if constexpr (std::is_same_v<T, void>)
        {
            _size = 0;
            _alignment = 0;
            _type_id = type_id_of<void>();
            _dtor = nullptr;
        }
        else
        {
            _size = sizeof(T);
            _alignment = alignof(T);
            _type_id = type_id_of<T>();
            if constexpr (std::is_trivially_destructible_v<T>)
            {
                _dtor = nullptr;
            }
            else
            {
                _dtor = +[](void* p) -> void {
                    reinterpret_cast<T*>(p)->~T();
                };
            }
        }

        _rttr_type = type_of<T>();
    }
    template <typename T>
    static V8BTPrimitive* _make(V8Isolate* isolate)
    {
        V8BTPrimitive* result = SkrNew<V8BTPrimitive>();
        result->set_isolate(isolate);
        result->_setup<T>();
        return result;
    }
    void _init_native(
        void* native_data
    ) const;
    bool _basic_type_check(
        const V8BTDataModifier& modifiers,
        V8ErrorCache& errors
    ) const;

private:
    uint32_t _size = 0;
    uint32_t _alignment = 0;
    GUID _type_id = {};
    const RTTRType* _rttr_type = nullptr;
    DtorInvoker _dtor = nullptr;
};
} // namespace skr
