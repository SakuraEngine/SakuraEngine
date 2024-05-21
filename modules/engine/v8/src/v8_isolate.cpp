#include "SkrV8/v8_isolate.hpp"
#include "SkrV8/v8_bind_tools.hpp"
#include "SkrV8/v8_bind_data.hpp"
#include "libplatform/libplatform.h"
#include "v8-initialization.h"
#include "SkrRTTR/type.hpp"
#include "v8-template.h"
#include "v8-external.h"

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
        _isolate->SetData(0, this);

        // delete array buffer allocator
        // TODO. custom allocator
        delete _isolate_create_params.array_buffer_allocator;
    }
}

void V8Isolate::_make_record_template(::skr::rttr::Type* type)
{
    using namespace ::v8;
    SKR_ASSERT(type->type_category() == ::skr::rttr::ETypeCategory::Record);

    // ctor template
    auto ctor_template = FunctionTemplate::New(
        _isolate,
        +[](const FunctionCallbackInfo<Value>& info) {
            // get v8 basic info
            Isolate*       Isolate = info.GetIsolate();
            Isolate::Scope IsolateScope(Isolate);
            HandleScope    HandleScope(Isolate);
            Local<Context> Context = Isolate->GetCurrentContext();
            Context::Scope ContextScope(Context);

            // get type info
            Local<External>  data = info.Data().As<External>();
            skr::rttr::Type* type = reinterpret_cast<skr::rttr::Type*>(data->Value());

            // match ctor
            for (const auto& ctor_data : type->record_data().ctor_data)
            {
                if (V8BindTools::match_param(ctor_data, info))
                {
                    V8Isolate* skr_isolate = reinterpret_cast<V8Isolate*>(Isolate->GetData(0));

                    // make bind data
                    V8BindData* bind_data = SkrNew<V8BindData>();
                    bind_data->type       = type;

                    // make data memory
                    bind_data->data = sakura_malloc_aligned(type->size(), type->alignment());

                    // call ctor
                    V8BindTools::call_ctor(bind_data->data, ctor_data, info);

                    // TODO. add weakref for listen GC event
                }
            }
        },
        External::New(_isolate, type));

    // bind field

    // bind method

    // bind static field

    // bind static method
}
} // namespace skr::v8

namespace skr::v8
{
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