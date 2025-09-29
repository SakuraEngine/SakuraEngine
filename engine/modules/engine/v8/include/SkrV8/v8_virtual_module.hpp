#pragma once
#include <SkrV8/v8_fwd.hpp>
#include <SkrV8/bind_template/v8_bind_template.hpp>
#include <SkrV8/v8_bind.hpp>

namespace skr
{
struct V8VirtualModuleNode
{
    enum class EKind
    {
        Namespace,
        BindTp,
    };

    // ctor & dtor
    inline V8VirtualModuleNode(String name_space)
        : _name(name_space)
        , _kind(EKind::Namespace)
        , _children{}
    {
    }
    inline V8VirtualModuleNode(V8BindTemplate* bind_tp)
        : _name{ bind_tp->type_name() }
        , _kind(EKind::BindTp)
        , _bind_tp(bind_tp)
    {
        SKR_ASSERT(bind_tp->has_v8_export_obj());
    }
    inline ~V8VirtualModuleNode()
    {
        if (_kind == EKind::Namespace)
        {
            clear_children();
        }
    }

    // getter
    inline String name() const { return _name; }
    inline EKind kind() const { return _kind; }
    inline bool is_namespace() const { return _kind == EKind::Namespace; }
    inline bool is_bind_tp() const { return _kind == EKind::BindTp; }
    inline V8BindTemplate* bind_tp() const
    {
        SKR_ASSERT(is_bind_tp());
        return _bind_tp;
    }
    inline const Map<String, V8VirtualModuleNode*>& children() const
    {
        SKR_ASSERT(is_namespace());
        return _children;
    }

    // child tools
    inline V8VirtualModuleNode* find_child(StringView name) const
    {
        SKR_ASSERT(is_namespace());
        return _children.find(name).value_or(nullptr);
    }
    inline V8VirtualModuleNode* add_child(String name_space)
    {
        SKR_ASSERT(is_namespace());
        SKR_ASSERT(!_children.contains(name_space));
        auto* new_node = SkrNew<V8VirtualModuleNode>(name_space);
        _children.add(name_space, new_node);
        return new_node;
    }
    inline void add_child(V8BindTemplate* bind_tp)
    {
        auto type_name = bind_tp->type_name();
        SKR_ASSERT(_kind == EKind::Namespace);
        SKR_ASSERT(!_children.contains(type_name));
        _children.add(type_name, SkrNew<V8VirtualModuleNode>(bind_tp));
    }
    inline void clear_children()
    {
        SKR_ASSERT(is_namespace());
        for (auto& child : _children)
        {
            SkrDelete(child.value);
        }
        _children.clear();
    }

    // export
    v8::Local<v8::Value> export_v8(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context
    )
    {
        using namespace v8;

        if (is_namespace())
        {
            Local<v8::Object> result = v8::Object::New(isolate);
            export_ns_to(isolate, context, result);
            return result;
        }
        else
        {
            return _bind_tp->get_v8_export_obj();
        }
    }
    void export_ns_to(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::Local<v8::Object> target
    )
    {
        SKR_ASSERT(is_namespace());

        for (const auto& [node_name, node] : _children)
        {
            // clang-format off
            target->Set(
                context,
                V8Bind::to_v8(node_name, true),
                node->export_v8(isolate, context)
            ).Check();
            // clang-format on
        }
    }
    void erase_export(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::Local<v8::Object> target
    )
    {
        SKR_ASSERT(is_namespace());

        for (const auto& [node_name, node] : _children)
        {
            // clang-format off
            target->Delete(
                context,
                V8Bind::to_v8(node_name, true)
            ).Check();
            // clang-format on
        }
    }

private:
    String _name = {};
    EKind _kind = {};
    V8BindTemplate* _bind_tp = nullptr;
    Map<String, V8VirtualModuleNode*> _children = {};
};

struct SKR_V8_API V8VirtualModule
{
    // getter & setter
    inline const V8VirtualModuleNode& root_node() const { return _root_node; }
    inline bool is_empty() const { return _root_node.children().is_empty(); }
    inline V8Isolate* isolate() const { return _isolate; }
    inline void set_isolate(V8Isolate* isolate) { _isolate = isolate; }

    // mapping
    inline String bind_tp_to_ns(const V8BindTemplate* bind_tp) const
    {
        // SKR_ASSERT(_bind_tp_to_ns.contains(bind_tp));
        return _bind_tp_to_ns.find(bind_tp).value_or({});
    }
    inline V8BindTemplate* ns_to_bind_tp(StringView name_space) const
    {
        // SKR_ASSERT(_ns_to_bind_tp.contains(name_space));
        return _ns_to_bind_tp.find(name_space).value_or(nullptr);
    }

    // register type
    template <typename T>
    inline bool register_type()
    {
        return raw_register_type(type_id_of<T>());
    }
    template <typename T>
    inline bool register_type(StringView ns)
    {
        return raw_register_type(type_id_of<T>(), ns);
    }

    // export
    v8::Local<v8::Value> export_v8(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context
    )
    {
        return _root_node.export_v8(isolate, context);
    }
    void export_v8_to(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::Local<v8::Object> target
    )
    {
        _root_node.export_ns_to(isolate, context, target);
    }
    void erase_export(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::Local<v8::Object> target
    )
    {
        _root_node.erase_export(isolate, context, target);
    }

    // ops
    inline void clear()
    {
        _root_node.clear_children();
    }

    // register helper
    inline bool raw_register_type(V8BindTemplate* bind_tp)
    {
        return raw_register_type(bind_tp, bind_tp->cpp_namespace());
    }
    bool raw_register_type(V8BindTemplate* bind_tp, StringView ns);
    bool raw_register_type(const GUID& type_id);
    bool raw_register_type(const GUID& type_id, StringView ns);

    // dump bind tp error
    void dump_bind_tp_error();

private:
    inline void _save_mapping(String ns, V8BindTemplate* bind_tp)
    {
        if (ns.is_empty())
        {
            ns = bind_tp->type_name();
        }
        else
        {
            ns.append(u8"::");
            ns.append(bind_tp->type_name());
        }
        _ns_to_bind_tp.add(ns, bind_tp);
        _bind_tp_to_ns.add(bind_tp, ns);
    }

private:
    V8Isolate* _isolate = nullptr;
    V8VirtualModuleNode _root_node = { u8"Root" };
    Map<String, V8BindTemplate*> _ns_to_bind_tp = {};
    Map<V8BindTemplate*, String> _bind_tp_to_ns = {};
};
} // namespace skr