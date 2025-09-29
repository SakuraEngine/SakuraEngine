#include <SkrV8/v8_value.hpp>
#include <SkrV8/v8_context.hpp>
#include <SkrV8/v8_isolate.hpp>

namespace skr
{
// ctor & dtor
V8Value::V8Value() = default;
V8Value::V8Value(V8Context* context)
    : _context(context)
{
    SKR_ASSERT(_context != nullptr);
}
V8Value::V8Value(v8::Global<v8::Value> v8_value, V8Context* context)
    : _v8_value(std::move(v8_value))
    , _context(context)
{
    SKR_ASSERT(!_v8_value.IsEmpty());
    SKR_ASSERT(_context != nullptr);
}
V8Value::~V8Value() = default;

// get kind
bool V8Value::is_object() const
{
    using namespace ::v8;

    // scopes
    auto* isolate = _context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    if (_v8_value.IsEmpty()) { return false; }
    auto local_value = _v8_value.Get(isolate);
    return !local_value->IsNullOrUndefined() && local_value->IsObject();
}
bool V8Value::is_function() const
{
    using namespace ::v8;

    // scopes
    auto* isolate = _context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    if (_v8_value.IsEmpty()) { return false; }
    auto local_value = _v8_value.Get(isolate);

    // check function
    return local_value->IsFunction();
}

// get field
V8Value V8Value::get_field(StringView name) const
{
    using namespace ::v8;

    // scopes
    auto* isolate = _context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    auto context = _context->v8_context().Get(isolate);

    // check object
    if (!_v8_value.Get(isolate)->IsObject())
    {
        return {};
    }

    // get object
    Local<v8::Object> obj = _v8_value.Get(isolate)->ToObject(context).ToLocalChecked();

    // get field
    auto found = obj->Get(context, V8Bind::to_v8(name, true));
    if (found.IsEmpty())
    {
        return {};
    }
    else
    {
        auto value = found.ToLocalChecked();
        if (value->IsNullOrUndefined()) { return {}; }
        return {
            { isolate, value },
            _context
        };
    }
}
bool V8Value::set_field_value(StringView name, const V8Value& value) const
{
    using namespace ::v8;

    // scopes
    auto* isolate = _context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    auto context = _context->v8_context().Get(isolate);

    // check object
    if (!_v8_value.Get(isolate)->IsObject()) { return false; }

    // get object
    Local<v8::Object> obj = _v8_value.Get(isolate)->ToObject(context).ToLocalChecked();

    auto result = obj->Set(
        context,
        V8Bind::to_v8(name, true),
        value.v8_value().Get(isolate)
    );
    return result.IsJust();
}

// helper
void V8Value::_get(TypeSignatureView sig, void* ptr) const
{
    using namespace ::v8;

    // scopes
    auto* isolate = _context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    // find bind template
    auto* bind_tp = _context->isolate()->solve_bind_tp(sig);
    SKR_ASSERT(bind_tp);

    // do convert
    bind_tp->to_native(
        ptr,
        _v8_value.Get(isolate),
        false
    );
}
bool V8Value::_is(TypeSignatureView sig) const
{
    using namespace ::v8;

    // scopes
    auto* isolate = _context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    // find bind template
    auto* bind_tp = _context->isolate()->solve_bind_tp(sig);
    if (!bind_tp) { return false; }

    return bind_tp->match_param(_v8_value.Get(isolate));
}
bool V8Value::_set(TypeSignatureView sig, void* ptr)
{
    using namespace ::v8;

    // scopes
    auto* isolate = _context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    // solve context
    Local<Context> solved_context = _context->v8_context().Get(isolate);
    Context::Scope context_scope(solved_context);

    // find bind template
    auto* bind_tp = _context->isolate()->solve_bind_tp(sig);
    if (!bind_tp) { return false; }

    // do export
    auto local_value = bind_tp->to_v8(ptr);
    _v8_value.Reset(isolate, local_value);
    return true;
}
bool V8Value::_call(
    const Span<const StackProxy> params,
    StackProxy return_value
) const
{
    SKR_ASSERT(is_function());

    using namespace ::v8;

    // scopes
    auto* isolate = _context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    // solve context
    Local<Context> solved_context = _context->v8_context().Get(isolate);
    Context::Scope context_scope(solved_context);

    // solve function
    auto v8_value_local = _v8_value.Get(isolate);
    auto v9_function = v8_value_local.As<v8::Function>();

    return _context->isolate()->invoke_v8(
        solved_context->Global(),
        v9_function,
        params,
        return_value
    );
}
bool V8Value::_call_method(
    const StringView name,
    const Span<const StackProxy> params,
    StackProxy return_value
) const
{
    SKR_ASSERT(is_object());

    using namespace ::v8;

    // scopes
    auto* isolate = _context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    // solve context
    Local<Context> solved_context = _context->v8_context().Get(isolate);
    Context::Scope context_scope(solved_context);

    // solve object
    auto v8_value_local = _v8_value.Get(isolate);
    auto v8_object = v8_value_local.As<v8::Object>();

    // find method
    auto maybe_found_value = v8_object->Get(
        solved_context,
        V8Bind::to_v8(name, true)
    );
    if (maybe_found_value.IsEmpty()) { return false; }
    auto found_value = maybe_found_value.ToLocalChecked();
    if (!found_value->IsFunction()) { return false; }
    auto found_func = found_value.As<Function>();

    return _context->isolate()->invoke_v8(
        v8_object,
        found_func,
        params,
        return_value
    );
}
} // namespace skr