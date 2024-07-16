#include "SkrV8/v8_context.hpp"
#include "SkrV8/v8_isolate.hpp"

namespace skr::v8
{
// ctor & dtor
V8Context::V8Context(V8Isolate* isolate)
    : _isolate(isolate)
{
}
V8Context::~V8Context()
{
}

// init & shutdown
void V8Context::init()
{
    using namespace ::v8;
    Isolate::Scope isolate_scope(_isolate->isolate());
    HandleScope    handle_scope(_isolate->isolate());

    // create context
    auto new_context = Context::New(_isolate->isolate());
    _context.Reset(_isolate->isolate(), new_context);

    // bind this
    new_context->SetAlignedPointerInEmbedderData(1, this);
}
void V8Context::shutdown()
{
    // destroy context
    _context.Reset();
}

// take template
void V8Context::install_templates()
{
    ::v8::Isolate::Scope isolate_scope(_isolate->isolate());
    ::v8::HandleScope    handle_scope(_isolate->isolate());

    // inject templates
    _isolate->inject_templates_into_context(_context.Get(_isolate->isolate()));
}

// exec script
void V8Context::exec_script(StringView script)
{
    ::v8::Isolate::Scope       isolate_scope(_isolate->isolate());
    ::v8::HandleScope          handle_scope(_isolate->isolate());
    ::v8::Local<::v8::Context> solved_context = _context.Get(_isolate->isolate());
    ::v8::Context::Scope       context_scope(solved_context);

    // compile script
    ::v8::Local<::v8::String> source = ::v8::String::NewFromUtf8(
                                           _isolate->isolate(),
                                           reinterpret_cast<const char*>(script.raw().data()),
                                           ::v8::NewStringType::kNormal,
                                           script.size())
                                           .ToLocalChecked();
    ::v8::Local<::v8::Script> compiled_script = ::v8::Script::Compile(solved_context, source).ToLocalChecked();

    // run script
    ::v8::Local<::v8::Value> result = compiled_script->Run(solved_context).ToLocalChecked();
}

} // namespace skr::v8