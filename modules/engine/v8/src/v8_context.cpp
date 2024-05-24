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
    // create context
    auto new_context = ::v8::Context::New(_isolate->isolate());
    _context.Reset(_isolate->isolate(), new_context);

    // bind this
    new_context->SetAlignedPointerInEmbedderData(1, this);
}
void V8Context::shutdown()
{
    // destroy context
    _context.Reset();
}

} // namespace skr::v8