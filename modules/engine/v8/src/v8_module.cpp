#include <SkrV8/v8_module.hpp>
#include <SkrV8/v8_isolate.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrRTTR/script/script_binder.hpp>

namespace skr
{
// setup by isolate
void V8Module::_init_basic(V8Isolate* isolate, StringView name)
{
    _isolate             = isolate;
    _module_info.manager = &_isolate->script_binder_manger();
    _name                = name;
}

// ctor & dtor
V8Module::V8Module()
{
}
V8Module::~V8Module()
{
    if (is_built())
    {
        shutdown();
    }
}

// build api
bool V8Module::build(FunctionRef<void(ScriptModule& module)> build_func)
{
    SKR_ASSERT(!is_built());
    SKR_ASSERT(_isolate != nullptr && "please setup isolate first");

    // build
    build_func(_module_info);

    // finalize
    {
        v8::Isolate::Scope isolate_scope(_isolate->v8_isolate());
        v8::HandleScope    handle_scope(_isolate->v8_isolate());

        if (!_module_info.check_full_export())
        {
            // dump lost types
            for (const auto& lost_item : _module_info.lost_types())
            {
                String type_name;
                switch (lost_item.kind())
                {
                case ScriptBinderRoot::EKind::Object: {
                    auto* type = lost_item.object()->type;
                    type_name  = format(u8"{}::{}", type->name_space_str(), type->name());
                    break;
                }
                case ScriptBinderRoot::EKind::Value: {
                    auto* type = lost_item.value()->type;
                    type_name  = format(u8"{}::{}", type->name_space_str(), type->name());
                    break;
                }
                case ScriptBinderRoot::EKind::Enum: {
                    auto* type = lost_item.enum_()->type;
                    type_name  = format(u8"{}::{}", type->name_space_str(), type->name());
                    break;
                }
                default:
                    SKR_UNREACHABLE_CODE()
                    break;
                }

                SKR_LOG_FMT_ERROR(u8"lost type {} when register module {}", type_name, _name);
            }
            return false;
        }
        else
        {
            auto            isolate     = _isolate->v8_isolate();
            auto            skr_isolate = reinterpret_cast<V8Isolate*>(isolate->GetData(0));
            v8::HandleScope handle_scope(isolate);

            // build import items
            std::vector<v8::Local<v8::String>> imports;
            for (const auto& [k, v] : _module_info.ns_mapper().root().children())
            {
                imports.push_back(V8Bind::to_v8(k, true));
            }

            // create module
            v8::Local<v8::Module> module = v8::Module::CreateSyntheticModule(
                isolate,
                V8Bind::to_v8(_name, true),
                imports,
                _eval_callback
            );
            _module.Reset(isolate, module);

            // add to skr module map
            skr_isolate->register_cpp_module_id(
                this,
                module->GetIdentityHash()
            );

            return true;
        }
    }
}
void V8Module::shutdown()
{
    SKR_ASSERT(is_built());

    // handle scope
    v8::Isolate::Scope isolate_scope(_isolate->v8_isolate());
    v8::HandleScope    handle_scope(_isolate->v8_isolate());

    // remove from skr module map
    auto isolate     = _isolate->v8_isolate();
    auto skr_isolate = reinterpret_cast<V8Isolate*>(isolate->GetData(0));
    auto module      = _module.Get(isolate);
    skr_isolate->unregister_cpp_module_id(
        this,
        module->GetIdentityHash()
    );

    // destroy module
    _module.Reset();
}
bool V8Module::is_built() const
{
    return !_module.IsEmpty();
}

// getter
v8::Local<v8::Module> V8Module::v8_module() const
{
    auto isolate = _isolate->v8_isolate();
    return _module.Get(isolate);
}

// eval callback
v8::MaybeLocal<v8::Value> V8Module::_eval_callback(
    v8::Local<v8::Context> context,
    v8::Local<v8::Module>  module
)
{
    auto  isolate    = v8::Isolate::GetCurrent();
    auto* skr_iolate = reinterpret_cast<V8Isolate*>(v8::Isolate::GetCurrent()->GetData(0));

    if (auto* result = skr_iolate->find_cpp_module(module->GetIdentityHash()))
    {
        // export namespace roots
        for (const auto& [k, v] : result->_module_info.ns_mapper().root().children())
        {
            // clang-format off
            module->SetSyntheticModuleExport(
                isolate,
                V8Bind::to_v8(k, true),
                V8Bind::export_namespace_node(v, skr_iolate)
            ).Check();
            // clang-format on
        }

        return V8Bind::to_v8(true);
    }
    else
    {
        return {};
    }
}
} // namespace skr