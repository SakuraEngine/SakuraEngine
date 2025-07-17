#pragma once
#include "SkrContainers/set.hpp"
#include <SkrRTTR/script/script_binder.hpp>

namespace skr
{
// namespace mapping
struct ScriptNamespaceNode {
    enum class EKind
    {
        Namespace,
        Type,
    };

    // ctor & dtor
    inline ScriptNamespaceNode(String name_space)
        : _name(name_space)
        , _kind(EKind::Namespace)
        , _children{}
    {
    }
    inline ScriptNamespaceNode(ScriptBinderRoot type)
        : _name{ _get_type_name(type) }
        , _kind(EKind::Type)
        , _type(type)
    {
        SKR_ASSERT(!type.is_primitive());
    }
    inline ~ScriptNamespaceNode()
    {
        if (_kind == EKind::Namespace)
        {
            for (auto& child : _children)
            {
                SkrDelete(child.value);
            }
            _children.release();
        }
    }

    // getter
    inline String           name() const { return _name; }
    inline EKind            kind() const { return _kind; }
    inline bool             is_namespace() const { return _kind == EKind::Namespace; }
    inline bool             is_type() const { return _kind == EKind::Type; }
    inline ScriptBinderRoot type() const
    {
        SKR_ASSERT(_kind == EKind::Type);
        return _type;
    }
    inline const Map<String, ScriptNamespaceNode*>& children() const
    {
        SKR_ASSERT(_kind == EKind::Namespace);
        return _children;
    }

    // child tools
    inline ScriptNamespaceNode* find_child(StringView name) const
    {
        SKR_ASSERT(_kind == EKind::Namespace);
        return _children.find(name).value_or(nullptr);
    }
    inline ScriptNamespaceNode* add_child(String name_space)
    {
        SKR_ASSERT(_kind == EKind::Namespace);
        SKR_ASSERT(!_children.contains(name_space));
        auto* new_node = SkrNew<ScriptNamespaceNode>(name_space);
        _children.add(name_space, new_node);
        return new_node;
    }
    inline void add_child(ScriptBinderRoot type)
    {
        auto type_name = _get_type_name(type);
        SKR_ASSERT(_kind == EKind::Namespace);
        SKR_ASSERT(!_children.contains(type_name));
        _children.add(type_name, SkrNew<ScriptNamespaceNode>(type));
    }

private:
    inline static String _get_type_name(ScriptBinderRoot type)
    {
        switch (type.kind())
        {
        case ScriptBinderRoot::EKind::Object:
            return type.object()->type->name();
        case ScriptBinderRoot::EKind::Value:
            return type.value()->type->name();
        case ScriptBinderRoot::EKind::Enum:
            return type.enum_()->type->name();
        case ScriptBinderRoot::EKind::Mapping:
            return type.mapping()->type->name();
        default:
            SKR_UNREACHABLE_CODE()
        }
        return {};
    }

private:
    String                            _name;
    EKind                             _kind;
    ScriptBinderRoot                  _type;
    Map<String, ScriptNamespaceNode*> _children;
};
struct ScriptNamespaceMapper {
    // register type
    inline bool register_type(ScriptBinderRoot type)
    {
        SKR_ASSERT(!type.is_empty());

        // need not export primitive
        if (type.is_primitive())
        {
            auto* primitive = type.primitive();
            auto* type      = get_type_from_guid(primitive->type_id);
            SKR_LOG_FMT_WARN(u8"primitive type not need export, when register '{}::{}'", type->name_space_str(), type->name());
            return true;
        }

        return register_type(type, _get_name_space_str(type));
    }
    inline bool register_type(ScriptBinderRoot type, StringView name_space)
    {
        SKR_ASSERT(!type.is_empty());

        // need not export primitive
        if (type.is_primitive())
        {
            auto* primitive = type.primitive();
            auto* type      = get_type_from_guid(primitive->type_id);
            SKR_LOG_FMT_WARN(u8"primitive type not need export, when register '{}::{}'", type->name_space_str(), type->name());
            return true;
        }

        // save mapping
        _save_mapping(name_space, type);

        // find namespace node
        ScriptNamespaceNode* node = &_root;
        name_space.split_each([&](StringView name_space) {
            if (!node) { return; } // failed
            if (auto found = node->find_child(name_space))
            { // found node and check type
                if (found->is_namespace())
                {
                    node = found;
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"namespace '{}' conflict with type, when export '{}::{}'", name_space, _get_name_space_str(type), _get_name(type));
                    node = nullptr;
                }
            }
            else
            { // add new node
                node = node->add_child(name_space);
            }
            // clang-format off
        }, u8"::", true);
        // clang-format on
        if (!node) { return false; } // failed

        // add type
        if (auto found = node->find_child(_get_name(type)))
        { // conflict
            if (found->is_type() && found->type() == type)
            { // success
                return true;
            }
            else
            {
                SKR_LOG_FMT_ERROR(u8"type '{}' conflict, when export '{}::{}'", _get_name(type), _get_name_space_str(type), _get_name(type));
                return false;
            }
        }
        else
        {
            node->add_child(type);
            return true;
        }
    }

    // getter
    inline const ScriptNamespaceNode& root() const { return _root; }

    // mapping
    inline String binder_to_ns(ScriptBinderRoot type) const
    {
        SKR_ASSERT(_to_ns.contains(type));
        return _to_ns.find(type).value_or({});
    }
    inline ScriptBinderRoot ns_to_binder(StringView name_space) const
    {
        SKR_ASSERT(_to_binder.contains(name_space));
        return _to_binder.find(name_space).value_or({});
    }

private:
    // helper
    inline static String _get_name_space_str(ScriptBinderRoot type)
    {
        switch (type.kind())
        {
        case ScriptBinderRoot::EKind::Object:
            return type.object()->type->name_space_str();
        case ScriptBinderRoot::EKind::Value:
            return type.value()->type->name_space_str();
        case ScriptBinderRoot::EKind::Enum:
            return type.enum_()->type->name_space_str();
        case ScriptBinderRoot::EKind::Mapping:
            return type.mapping()->type->name_space_str();
        default:
            SKR_UNREACHABLE_CODE()
            return {};
        }
    }
    inline static String _get_name(ScriptBinderRoot type)
    {
        switch (type.kind())
        {
        case ScriptBinderRoot::EKind::Object:
            return type.object()->type->name();
        case ScriptBinderRoot::EKind::Value:
            return type.value()->type->name();
        case ScriptBinderRoot::EKind::Enum:
            return type.enum_()->type->name();
        case ScriptBinderRoot::EKind::Mapping:
            return type.mapping()->type->name();
        default:
            SKR_UNREACHABLE_CODE()
            return {};
        }
    }
    inline void _save_mapping(String ns, ScriptBinderRoot binder)
    {
        if (ns.is_empty())
        {
            ns = _get_name(binder);
        }
        else
        {
            ns.append(u8"::");
            ns.append(_get_name(binder));
        }
        _to_binder.add(ns, binder);
        _to_ns.add(binder, ns);
    }

private:
    ScriptNamespaceNode           _root      = { u8"[ROOT]" };
    Map<String, ScriptBinderRoot> _to_binder = {};
    Map<ScriptBinderRoot, String> _to_ns     = {};
};

// module concept
struct ScriptModule {
    // register type
    inline bool register_type(const RTTRType* type)
    {
        SKR_ASSERT(type);
        SKR_ASSERT(manager);
        return register_type(type, type->name_space_str());
    }
    inline bool register_type(const RTTRType* type, StringView name_space)
    {
        SKR_ASSERT(type);
        SKR_ASSERT(manager);
        auto binder = manager->get_or_build(type->type_id());
        if (!binder.is_empty())
        {
            _exported_types.add(binder);
            return _ns_mapper.register_type(binder, name_space);
        }
        else
        {
            return false;
        }
    }

    // template register
    template <typename T>
    inline bool register_type()
    {
        return register_type(type_of<T>());
    }
    template <typename T>
    inline bool register_type(StringView name_space)
    {
        return register_type(type_of<T>(), name_space);
    }

    // finalize
    inline bool check_full_export()
    {
        _lost_types.clear();

        for (const auto& type : _exported_types)
        {
            switch (type.kind())
            {
            case ScriptBinderRoot::EKind::Object:
                _check_full_export(type.object());
                break;
            case ScriptBinderRoot::EKind::Value:
                _check_full_export(type.value());
                break;
            case ScriptBinderRoot::EKind::Enum:
            case ScriptBinderRoot::EKind::Primitive:
            case ScriptBinderRoot::EKind::Mapping:
                break;
            default:
                SKR_UNREACHABLE_CODE()
                break;
            }
        }

        return _lost_types.is_empty();
    }

    // getter
    inline const ScriptNamespaceMapper& ns_mapper() const { return _ns_mapper; }
    inline const Set<ScriptBinderRoot>& exported_types() const { return _exported_types; }
    inline const Set<ScriptBinderRoot>& lost_types() const { return _lost_types; }

public:
    ScriptBinderManager* manager = nullptr;

private:
    // check helper
    inline void _check_full_export(ScriptBinderRecordBase* record)
    {
        // each ctor
        _check_full_export(record->ctor.params_binder);

        // each fields
        for (const auto& [key, field] : record->fields)
        {
            _check_exported(field.binder);
        }

        // each static fields
        for (const auto& [key, field] : record->static_fields)
        {
            _check_exported(field.binder);
        }

        // each methods
        for (const auto& [key, method] : record->methods)
        {
            _check_full_export(method.params_binder);
            _check_exported(method.return_binder.binder);
        }

        // each static methods
        for (const auto& [key, method] : record->static_methods)
        {
            _check_full_export(method.params_binder);
            _check_exported(method.return_binder.binder);
        }

        // each properties
        for (const auto& [key, prop] : record->properties)
        {
            _check_full_export(prop.getter.params_binder);
            _check_exported(prop.getter.return_binder.binder);

            _check_full_export(prop.setter.params_binder);
            _check_exported(prop.setter.return_binder.binder);
        }

        // each static properties
        for (const auto& [key, prop] : record->static_properties)
        {
            _check_full_export(prop.getter.params_binder);
            _check_exported(prop.getter.return_binder.binder);

            _check_full_export(prop.setter.params_binder);
            _check_exported(prop.setter.return_binder.binder);
        }
    }
    inline void _check_full_export(const Vector<ScriptBinderParam>& params)
    {
        for (const auto& param : params)
        {
            _check_exported(param.binder);
        }
    }
    inline void _check_exported(ScriptBinderRoot type)
    {
        SKR_ASSERT(!type.is_empty());
        if (!(type.is_primitive() || type.is_mapping()))
        {
            if (!_exported_types.contains(type))
            {
                _lost_types.add(type);
            }
        }
    }

private:
    ScriptNamespaceMapper _ns_mapper      = {};
    Set<ScriptBinderRoot> _exported_types = {};
    Set<ScriptBinderRoot> _lost_types     = {};
};
} // namespace skr