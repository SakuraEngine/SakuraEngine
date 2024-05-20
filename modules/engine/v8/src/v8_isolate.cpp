#include "SkrV8/v8_isolate.hpp"
#include "libplatform/libplatform.h"
#include "v8-initialization.h"
#include <filesystem>

namespace skr::v8
{
void V8Isolate::init()
{
    // init isolate
    // TODO. custom allocator
    _isolate_create_params.array_buffer_allocator = ::v8::ArrayBuffer::Allocator::NewDefaultAllocator();
}

void V8Isolate::shutdown()
{
    if (_isolate)
    {
        // dispose isolate
        _isolate->Dispose();

        // delete array buffer allocator
        // TODO. custom allocator
        delete _isolate_create_params.array_buffer_allocator;
    }
}

static auto& _v8_platform()
{
    static auto _platform = ::v8::platform::NewDefaultPlatform();
    return _platform;
}

void init_v8()
{
    // init flags
    // char Flags[] = "--expose-gc";
    // ::v8::V8::SetFlagsFromString(Flags, sizeof(Flags));

    // init platform
    _v8_platform() = ::v8::platform::NewDefaultPlatform();
    ::v8::V8::InitializePlatform(_v8_platform().get());

    // init v8
    ::v8::V8::Initialize();
}
void shutdown_v8()
{
    // shutdown v8
    ::v8::V8::Dispose();

    // shutdown platform
    ::v8::V8::DisposePlatform();
    _v8_platform().reset();
}
} // namespace skr::v8