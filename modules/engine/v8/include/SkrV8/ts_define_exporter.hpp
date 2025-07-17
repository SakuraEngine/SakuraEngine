#pragma once
#include "SkrRTTR/script/script_tools.hpp"

namespace skr
{
struct TSDefineExporter {
    // generate
    String generate_global();

    // generate module
    String generate_module(StringView module_name);

public:
    // codegen config
    uint32_t indent_size = 2;

    // script binder manager
    const ScriptModule* module = nullptr;

private:
    // codegen unit
    void _gen();
    void _gen_ns_node(const ScriptNamespaceNode& ns_node);
    void _gen_enum(ScriptBinderEnum& enum_binder);
    void _gen_mapping(ScriptBinderMapping& mapping);
    void _gen_object(ScriptBinderObject& object_binder);
    void _gen_value(ScriptBinderValue& value_binder);
    void _gen_record(ScriptBinderRecordBase& record_binder);

    // codegen utils
    String _type_name(ScriptBinderRoot binder) const;
    String _type_name_with_ns(ScriptBinderRoot binder) const;
    String _params_signature(const Vector<ScriptBinderParam>& params) const;
    String _return_signature(
        const Vector<ScriptBinderParam>& params,
        const ScriptBinderReturn&        ret,
        uint32_t                         solved_return_count
    ) const;

    // codegen tools
    void $line();
    template <typename... Args>
    void $line(StringView fmt, Args&&... args);
    template <typename Func>
    void $indent(Func&& func);

private:
    // codegen tools
    uint32_t _cur_indent = 0;
    String   _result;
};
} // namespace skr

namespace skr
{
// generate
inline String TSDefineExporter::generate_global()
{
    SKR_ASSERT(module && "please specify module first");

    _result.clear();
    $line(u8"export {{}};");
    $line(u8"declare global {{");
    $indent([this] {
        _gen();
    });
    $line(u8"}}");
    return _result;
}

// generate module
inline String TSDefineExporter::generate_module(StringView module_name)
{
    SKR_ASSERT(module && "please specify module first");

    _result.clear();
    $line(u8"declare module \"{}\" {{", module_name);
    $indent([this] {
        _gen();
    });
    $line(u8"}}");
    return _result;
}

// codegen unit
inline void TSDefineExporter::_gen()
{
    auto& root_node = module->ns_mapper().root();
    for (const auto& [node_name, child] : root_node.children())
    {
        _gen_ns_node(*child);
    }
}
inline void TSDefineExporter::_gen_ns_node(const ScriptNamespaceNode& ns_node)
{
    if (ns_node.is_namespace())
    {
        $line(u8"export namespace {} {{", ns_node.name());
        $indent([&]() {
            for (const auto& [node_name, child] : ns_node.children())
            {
                _gen_ns_node(*child);
            }
        });
        $line(u8"}}");
    }
    else
    {
        auto binder = ns_node.type();
        switch (binder.kind())
        {
        case ScriptBinderRoot::EKind::Enum: {
            auto* enum_binder = binder.enum_();
            _gen_enum(*enum_binder);
            $line();
            break;
        }
        case ScriptBinderRoot::EKind::Mapping: {
            auto* mapping_binder = binder.mapping();
            _gen_mapping(*mapping_binder);
            $line();
            break;
        }
        case ScriptBinderRoot::EKind::Object: {
            auto* object_binder = binder.object();
            _gen_object(*object_binder);
            $line();
            break;
        }
        case ScriptBinderRoot::EKind::Value: {
            auto* value_binder = binder.value();
            _gen_value(*value_binder);
            $line();
            break;
        }
        case ScriptBinderRoot::EKind::Primitive:
            // won't export primitive
            break;
        default:
            $line(u8"// failed to generate type");
            break;
        }
    }
}
inline void TSDefineExporter::_gen_enum(ScriptBinderEnum& enum_binder)
{
    // print cpp symbol
    $line(
        u8"// cpp symbol: {}::{}",
        String::Join(enum_binder.type->name_space(), u8"::"),
        enum_binder.type->name()
    );

    // enum name
    $line(u8"export enum {} {{", enum_binder.type->name());
    $indent([&] {
        // enum values
        for (auto& [item_name, item_value] : enum_binder.items)
        {
            if (item_value->value.is_signed())
            {
                $line(u8"{} = {},", item_name, item_value->value.value_signed());
            }
            else
            {
                $line(u8"{} = {},", item_name, item_value->value.value_unsigned());
            }
        }
    });
    $line(u8"}}");

    // to_string & from_string
    $line(u8"export namespace {} {{", enum_binder.type->name());
    $indent([&] {
        // to_string
        $line(u8"export function to_string(value: {}): string", enum_binder.type->name());

        // from_string
        $line(u8"export function from_string(value: string): {}", enum_binder.type->name());
    });
    $line(u8"}}");
}
inline void TSDefineExporter::_gen_mapping(ScriptBinderMapping& mapping)
{
    $line(u8"// export as mapping");
    // print cpp symbol
    $line(
        u8"// cpp symbol: {}::{}",
        String::Join(mapping.type->name_space(), u8"::"),
        mapping.type->name()
    );

    // as interface
    $line(u8"export interface {} {{", mapping.type->name());
    $indent([&] {
        // fields
        for (auto& [field_name, field_value] : mapping.fields)
        {
            $line(u8"{}: {};", field_name, _type_name_with_ns(field_value.binder));
        }
    });
    $line(u8"}}");
}
inline void TSDefineExporter::_gen_object(ScriptBinderObject& object_binder)
{
    $line(u8"// export as object");
    _gen_record(object_binder);
}
inline void TSDefineExporter::_gen_value(ScriptBinderValue& value_binder)
{
    $line(u8"// export as value");
    _gen_record(value_binder);
}
inline void TSDefineExporter::_gen_record(ScriptBinderRecordBase& record_binder)
{
    // print cpp symbol
    $line(
        u8"// cpp symbol: {}::{}",
        String::Join(record_binder.type->name_space(), u8"::"),
        record_binder.type->name()
    );

    // as class
    $line(u8"export class {} {{", record_binder.type->name());
    $indent([&] {
        String record_full_name = String::Join(record_binder.type->name_space(), u8"::");
        String record_name      = record_binder.type->name();
        record_full_name.append(u8"::");
        record_full_name.append(record_name);

        // ctors
        if (record_binder.ctor.data)
        {
            $line(
                u8"// cpp symbol: {}::{}",
                record_full_name,
                record_binder.type->name()
            );
            $line(u8"constructor({});", _params_signature(record_binder.ctor.params_binder));
        }

        // fields
        for (auto& [field_name, field_value] : record_binder.fields)
        {
            $line(
                u8"// cpp symbol: {}::{}",
                record_full_name,
                field_value.data->name
            );
            $line(u8"{}: {};", field_name, _type_name_with_ns(field_value.binder));
        }

        // methods
        for (auto& [method_name, method_value] : record_binder.methods)
        {
            if (method_value.mixin_impl_data)
            {
                $line(u8"// [MIXIN]");
            }

            $line(
                u8"// cpp symbol: {}::{}",
                record_full_name,
                method_value.data->name
            );
            $line(
                u8"{}({}): {};",
                method_name,
                _params_signature(method_value.params_binder),
                _return_signature(method_value.params_binder, method_value.return_binder, method_value.return_count)
            );
        }

        // static fields
        for (auto& [static_field_name, static_field_value] : record_binder.static_fields)
        {
            $line(
                u8"// cpp symbol: {}::{}",
                record_full_name,
                static_field_value.data->name
            );
            $line(u8"static {}: {};", static_field_name, _type_name_with_ns(static_field_value.binder));
        }

        // static methods
        for (auto& [static_method_name, static_method_value] : record_binder.static_methods)
        {
            $line(
                u8"// cpp symbol: {}::{}",
                record_full_name,
                static_method_value.data->name
            );
            $line(
                u8"static {}({}): {};",
                static_method_name,
                _params_signature(static_method_value.params_binder),
                _return_signature(static_method_value.params_binder, static_method_value.return_binder, static_method_value.return_count)
            );
        }

        // properties
        for (auto& [property_name, property_value] : record_binder.properties)
        {
            if (property_value.setter.data)
            {
                $line(
                    u8"// cpp symbol: {}::{}",
                    record_full_name,
                    property_value.setter.data->name
                );
                $line(u8"set {}(value: {});", property_name, _type_name_with_ns(property_value.binder));
            }
            if (property_value.getter.data)
            {
                $line(
                    u8"// cpp symbol: {}::{}",
                    record_full_name,
                    property_value.getter.data->name
                );
                $line(u8"get {}(): {};", property_name, _type_name_with_ns(property_value.binder));
            }
        }

        // static properties
        for (auto& [static_property_name, static_property_value] : record_binder.static_properties)
        {
            if (static_property_value.setter.data)
            {
                $line(
                    u8"// cpp symbol: {}::{}",
                    record_full_name,
                    static_property_value.setter.data->name
                );
                $line(u8"static set {}(value: {});", static_property_name, _type_name_with_ns(static_property_value.binder));
            }
            if (static_property_value.getter.data)
            {
                $line(
                    u8"// cpp symbol: {}::{}",
                    record_full_name,
                    static_property_value.getter.data->name
                );
                $line(u8"static get {}(): {};", static_property_name, _type_name_with_ns(static_property_value.binder));
            }
        }
    });
    $line(u8"}}");
}

// codegen utils
inline String TSDefineExporter::_type_name(ScriptBinderRoot binder) const
{
    switch (binder.kind())
    {
    case ScriptBinderRoot::EKind::Enum:
        return binder.enum_()->type->name();
    case ScriptBinderRoot::EKind::Mapping:
        return binder.mapping()->type->name();
    case ScriptBinderRoot::EKind::Object:
        return binder.object()->type->name();
    case ScriptBinderRoot::EKind::Value:
        return binder.value()->type->name();
    case ScriptBinderRoot::EKind::Primitive: {
        auto* primitive_binder = binder.primitive();
        switch (primitive_binder->type_id.get_hash())
        {
        case type_id_of<bool>().get_hash():
            return u8"boolean";
        case type_id_of<int8_t>().get_hash():
        case type_id_of<uint8_t>().get_hash():
        case type_id_of<int16_t>().get_hash():
        case type_id_of<uint16_t>().get_hash():
        case type_id_of<int32_t>().get_hash():
        case type_id_of<uint32_t>().get_hash():
        case type_id_of<float>().get_hash():
        case type_id_of<double>().get_hash():
            return u8"number";
        case type_id_of<int64_t>().get_hash():
        case type_id_of<uint64_t>().get_hash():
            return u8"bigint";
        case type_id_of<StringView>().get_hash():
        case type_id_of<String>().get_hash():
            return u8"string";
        default:
            SKR_UNREACHABLE_CODE()
            return {};
        }
    }
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
}
inline String TSDefineExporter::_type_name_with_ns(ScriptBinderRoot binder) const
{
    switch (binder.kind())
    {
    case ScriptBinderRoot::EKind::Enum:
    case ScriptBinderRoot::EKind::Mapping:
    case ScriptBinderRoot::EKind::Object:
    case ScriptBinderRoot::EKind::Value: {
        auto result = module->ns_mapper().binder_to_ns(binder);
        result.replace(u8"::", u8".");
        return result;
    }
    case ScriptBinderRoot::EKind::Primitive: {
        auto* primitive_binder = binder.primitive();
        switch (primitive_binder->type_id.get_hash())
        {
        case type_id_of<bool>().get_hash():
            return u8"boolean";
        case type_id_of<int8_t>().get_hash():
        case type_id_of<uint8_t>().get_hash():
        case type_id_of<int16_t>().get_hash():
        case type_id_of<uint16_t>().get_hash():
        case type_id_of<int32_t>().get_hash():
        case type_id_of<uint32_t>().get_hash():
        case type_id_of<float>().get_hash():
        case type_id_of<double>().get_hash():
            return u8"number";
        case type_id_of<int64_t>().get_hash():
        case type_id_of<uint64_t>().get_hash():
            return u8"bigint";
        case type_id_of<StringView>().get_hash():
        case type_id_of<String>().get_hash():
            return u8"string";
        default:
            SKR_UNREACHABLE_CODE()
            return {};
        }
    }
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
}
inline String TSDefineExporter::_params_signature(const Vector<ScriptBinderParam>& params) const
{
    String result;
    for (size_t i = 0; i < params.size(); ++i)
    {
        auto& param = params[i];

        // filter pure out
        if (param.inout_flag == ERTTRParamFlag::Out)
        {
            continue;
        }

        // push comma
        if (i > 0)
        {
            result.append(u8", ");
        }

        // push inout flags
        if (param.inout_flag == ERTTRParamFlag::InOut)
        {
            result.append(u8"/*inout*/");
        }

        // push param name
        result.append(param.data->name);

        // push nullable
        if (param.binder.is_object())
        {
            result.append(u8"?");
        }

        result.append(u8": ");

        // push type
        result.append(_type_name_with_ns(param.binder));
    }
    return result;
}
inline String TSDefineExporter::_return_signature(
    const Vector<ScriptBinderParam>& params,
    const ScriptBinderReturn&        ret,
    uint32_t                         solved_return_count
) const
{

    if (solved_return_count == 0)
    {
        return u8"void";
    }
    else if (solved_return_count == 1)
    {
        if (ret.is_void)
        {
            for (auto& param : params)
            {
                if (param.appare_in_return)
                {
                    return format(u8"{} /*{}*/", _type_name_with_ns(param.binder), param.data->name);
                }
            }
            SKR_UNREACHABLE_CODE();
            return {};
        }
        else
        {
            return _type_name_with_ns(ret.binder);
        }
    }
    else
    {
        String result;
        result.append(u8"[");
        if (!ret.is_void)
        {
            result.append(format(u8"{} /*[return]*/", _type_name_with_ns(ret.binder)));
            result.append(u8", ");
        }
        for (auto& param : params)
        {
            if (param.appare_in_return)
            {
                result.append(format(u8"{} /*{}*/", _type_name_with_ns(param.binder), param.data->name));
                result.append(u8", ");
            }
        }
        // remove last comma
        result.first(result.length_buffer() - 2);
        result.append(u8"]");
        return result;
    }
}

// codegen tools
inline void TSDefineExporter::$line()
{
    _result.append(u8'\n');
}
template <typename... Args>
inline void TSDefineExporter::$line(StringView fmt, Args&&... args)
{
    _result.add(u8' ', _cur_indent * indent_size);
    format_to(_result, fmt, std::forward<Args>(args)...);
    _result.append(u8'\n');
}
template <typename Func>
inline void TSDefineExporter::$indent(Func&& func)
{
    ++_cur_indent;
    func();
    --_cur_indent;
}
} // namespace skr