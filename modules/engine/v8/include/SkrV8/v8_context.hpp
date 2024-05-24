#pragma once
#include "SkrBase/config.h"
#include "v8-context.h"
#include "v8-persistent-handle.h"

namespace skr::v8
{
struct V8Isolate;

struct SKR_V8_API V8Context {
    // ctor & dtor
    V8Context(V8Isolate* isolate);
    ~V8Context();

    // delete copy & move
    V8Context(const V8Context&)            = delete;
    V8Context(V8Context&&)                 = delete;
    V8Context& operator=(const V8Context&) = delete;
    V8Context& operator=(V8Context&&)      = delete;

    // init & shutdown
    void init();
    void shutdown();

private:
    // owner
    V8Isolate* _isolate;

    // context data
    ::v8::Persistent<::v8::Context> _context;
};
} // namespace skr::v8