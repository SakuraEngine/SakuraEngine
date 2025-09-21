#include <SkrV8/v8_virtual_module.hpp>
#include <SkrV8/v8_isolate.hpp>

namespace skr
{
bool V8VirtualModule::raw_register_type(V8BindTemplate* bind_tp, StringView ns)
{
    if (!bind_tp->has_v8_export_obj())
    {
        SKR_LOG_FMT_ERROR(u8"bind tp '{}' has no v8 export obj, cannot register", bind_tp->type_name());
        return false;
    }

    // save mapping
    _save_mapping(ns, bind_tp);

    // find namespace node
    V8VirtualModuleNode* node = &_root_node;
    ns.split_each([&](StringView ns_part) {
        if (!node) { return; } // failed
        if (auto found = node->find_child(ns_part))
        { // found node and check type
            if (found->is_namespace())
            {
                node = found;
            }
            else
            {
                SKR_LOG_FMT_ERROR(
                    u8"namespace '{}' conflict with type, when export '{}' with namespace '{}'",
                    ns_part,
                    bind_tp->type_name(),
                    bind_tp->cpp_namespace()
                );
                node = nullptr;
            }
        }
        else
        { // add new node
            node = node->add_child(ns_part);
        }
        // clang-format off
        }, u8"::", true);
    // clang-format on
    if (!node) { return false; } // failed

    // add bind tp
    if (auto found = node->find_child(bind_tp->type_name()))
    { // conflict
        if (found->is_bind_tp() && found->bind_tp() == bind_tp)
        { // success
            return true;
        }
        else
        {
            String conflict_type = found->is_bind_tp() ? u8"bind tp" : u8"namespace";
            String name = found->is_bind_tp() ? found->bind_tp()->type_name() : found->name();

            SKR_LOG_FMT_ERROR(
                u8"bind tp '{}' conflict, when export '{}' with namespace '{}', conflict with {} '{}'",
                bind_tp->type_name(),
                ns,
                conflict_type,
                name
            );
            return false;
        }
    }
    else
    {
        node->add_child(bind_tp);
        return true;
    }
}
bool V8VirtualModule::raw_register_type(const GUID& type_id)
{
    SKR_ASSERT(_isolate);

    auto bind_tp = _isolate->solve_bind_tp(type_id);
    if (!bind_tp)
    {
        SKR_LOG_FMT_ERROR(u8"bind tp with type id '{}' not found", type_id);
        return false;
    }

    return raw_register_type(bind_tp);
}
bool V8VirtualModule::raw_register_type(const GUID& type_id, StringView ns)
{
    SKR_ASSERT(_isolate);

    auto bind_tp = _isolate->solve_bind_tp(type_id);
    if (!bind_tp)
    {
        SKR_LOG_FMT_ERROR(u8"bind tp with type id '{}' not found", type_id);
        return false;
    }

    return raw_register_type(bind_tp, ns);
}

// dump bind tp error
void V8VirtualModule::dump_bind_tp_error()
{
    for (const auto& [ns, bind_tp] : _ns_to_bind_tp)
    {
        if (bind_tp->any_error())
        {
            V8ErrorBuilderTreeStyle builder;
            bind_tp->dump_error(builder);
            auto errors = builder.build();
            SKR_LOG_FMT_ERROR(u8"error detected when export v8\n{}", errors);
        }
    }
}
} // namespace skr