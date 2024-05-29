#include "SkrV8/v8_isolate.hpp"
#include "SkrContainers/set.hpp"
#include "SkrV8/v8_bind_tools.hpp"
#include "SkrV8/v8_bind_data.hpp"
#include "libplatform/libplatform.h"
#include "v8-initialization.h"
#include "SkrRTTR/type.hpp"
#include "v8-template.h"
#include "v8-external.h"
#include "v8-function.h"

// allocator
namespace skr::v8
{
struct V8Allocator final : ::v8::ArrayBuffer::Allocator {
    static constexpr const char* kV8DefaultPoolName = "v8-allocate";

    void* AllocateUninitialized(size_t length) override
    {
#if defined(TRACY_TRACE_ALLOCATION)
        SkrCZoneNCS(z, "v8::allocate", SKR_ALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* p = sakura_malloc_alignedN(length, alignof(size_t), kV8DefaultPoolName);
        SkrCZoneEnd(z);
        return p;
#else
        return reinterpret_cast<T*>(sakura_malloc_aligned(length, alignof(size_t)));
#endif
    }
    void Free(void* data, size_t length) override
    {
        if (data)
        {
#if defined(TRACY_TRACE_ALLOCATION)
            SkrCZoneNCS(z, "v8::free", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
            sakura_free_alignedN(data, alignof(size_t), kV8DefaultPoolName);
            SkrCZoneEnd(z);
#else
            sakura_free_aligned(data, alignof(size_t));
#endif
        }
    }
    void* Reallocate(void* data, size_t old_length, size_t new_length) override
    {
        SkrCZoneNCS(z, "v8::realloc", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* new_mem = sakura_realloc_alignedN(data, new_length, alignof(size_t), kV8DefaultPoolName);
        SkrCZoneEnd(z);
        return new_mem;
    }
    void* Allocate(size_t length) override
    {
        void* p = AllocateUninitialized(length);
        memset(p, 0, length);
        return p;
    }
};
} // namespace skr::v8

namespace skr::v8
{
V8Isolate::V8Isolate()
{
}
V8Isolate::~V8Isolate()
{
}

void V8Isolate::init()
{
    using namespace ::v8;

    // init isolate
    _isolate_create_params.array_buffer_allocator = SkrNew<V8Allocator>();
    _isolate                                      = Isolate::New(_isolate_create_params);
    _isolate->SetData(0, this);
}
void V8Isolate::shutdown()
{
    if (_isolate)
    {
        // clear templates
        _record_templates.clear();

        // dispose isolate
        _isolate->Dispose();

        // delete array buffer allocator
        SkrDelete(_isolate_create_params.array_buffer_allocator);
    }
}

void V8Isolate::make_record_template(::skr::rttr::Type* type)
{
    using namespace ::v8;
    SKR_ASSERT(type->type_category() == ::skr::rttr::ETypeCategory::Record);
    Isolate::Scope isolate_scope(_isolate);
    HandleScope    handle_scope(_isolate);

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

            if (info.IsConstructCall())
            {
                // get ctor info
                Local<Object> self = info.This();

                // get type info
                Local<External>  data = info.Data().As<External>();
                skr::rttr::Type* type = reinterpret_cast<skr::rttr::Type*>(data->Value());

                // match ctor
                for (const auto& ctor_data : type->record_data().ctor_data)
                {
                    if (V8BindTools::match_params(ctor_data, info))
                    {
                        V8Isolate* skr_isolate = reinterpret_cast<V8Isolate*>(Isolate->GetData(0));

                        // make bind data
                        V8RecordBindData* bind_data = SkrNew<V8RecordBindData>();
                        bind_data->type             = type;

                        // make data memory
                        bind_data->data = sakura_malloc_aligned(type->size(), type->alignment());

                        // call ctor
                        V8BindTools::call_ctor(bind_data->data, ctor_data, info, Context, Isolate);

                        // add extern data
                        self->SetInternalField(0, External::New(Isolate, bind_data));

                        // TODO. add weakref for listen GC event

                        return;
                    }
                }

                // no ctor matched
                Isolate->ThrowError("no ctor matched");
            }
            else
            {
                Isolate->ThrowError("must be called with new");
            }
        },
        External::New(_isolate, type));

    // setup internal field count
    ctor_template->InstanceTemplate()->SetInternalFieldCount(1);

    auto proto_type_template = ctor_template->PrototypeTemplate();

    // TODO. 收集某个类型的所有方法，生成绑定信息，然后再生成绑定代码, 数据通过 External 传递, 内存管理交给绑定数据

    // bind field
    // TODO. recursive bind field
    for (const auto& field : type->record_data().fields)
    {
        proto_type_template->SetAccessor(
            ::v8::String::NewFromUtf8(_isolate, field.name.c_str()).ToLocalChecked(),
            +[](Local<::v8::String> property, const PropertyCallbackInfo<Value>& info) {
                // get data
                Isolate*          isolate = info.GetIsolate();
                Local<Context>    context = isolate->GetCurrentContext();
                Local<Object>     self    = info.This();
                Local<External>   wrap    = Local<External>::Cast(self->GetInternalField(0));
                V8RecordBindData* data    = reinterpret_cast<V8RecordBindData*>(wrap->Value());

                // get field data
                auto   name_len = property->Utf8Length(isolate);
                String field_name;
                field_name.raw().append('\0', name_len);
                property->WriteUtf8(isolate, reinterpret_cast<char*>(field_name.raw().data()));
                auto& field_data = data->type->record_data().fields.find_if([&](const auto& f) {
                                                                       return f.name == field_name;
                                                                   })
                                       .ref();

                // return field
                Local<Value> result;
                if (V8BindTools::native_to_v8_primitive(
                        context,
                        isolate,
                        field_data.type,
                        field_data.get_address(data->data),
                        result))
                {
                    info.GetReturnValue().Set(result);
                }
                else
                {
                    isolate->ThrowError("field type not supported");
                }
            },
            +[](Local<::v8::String> property, Local<Value> value, const PropertyCallbackInfo<void>& info) {
                // get data
                Isolate*          isolate = info.GetIsolate();
                Local<Context>    context = isolate->GetCurrentContext();
                Local<Object>     self    = info.This();
                Local<External>   wrap    = Local<External>::Cast(self->GetInternalField(0));
                V8RecordBindData* data    = reinterpret_cast<V8RecordBindData*>(wrap->Value());

                // get field data
                auto   name_len = property->Utf8Length(isolate);
                String field_name;
                field_name.raw().append('\0', name_len);
                property->WriteUtf8(isolate, reinterpret_cast<char*>(field_name.raw().data()));
                auto& field_data = data->type->record_data().fields.find_if([&](const auto& f) {
                                                                       return f.name == field_name;
                                                                   })
                                       .ref();

                // set field
                if (V8BindTools::v8_to_native_primitive(
                        context,
                        isolate,
                        field_data.type,
                        value,
                        field_data.get_address(data->data)))
                {
                    info.GetReturnValue().Set(value);
                }
                else
                {
                    isolate->ThrowError("field type not supported");
                }
            });
    }

    // bind method
    // TODO. recursive bind method
    for (const auto& method : type->record_data().methods)
    {
    }

    // bind static field
    // TODO. recursive bind static field

    // bind static method
    // TODO. recursive bind static method

    // add to templates
    auto& template_ref = _record_templates.try_add_default(type).value();
    template_ref.Reset(_isolate, ctor_template);
}
void V8Isolate::inject_templates_into_context(::v8::Local<::v8::Context> context)
{
    ::v8::Isolate::Scope isolate_scope(_isolate);
    ::v8::HandleScope    handle_scope(_isolate);

    for (const auto& pair : _record_templates)
    {
        const auto& type         = pair.key;
        const auto& template_ref = pair.value;

        // make function template
        auto function = template_ref.Get(_isolate)->GetFunction(context).ToLocalChecked();

        // set to context
        context->Global()->Set(
                             context,
                             ::v8::String::NewFromUtf8(_isolate, type->name().c_str()).ToLocalChecked(),
                             function)
            .Check();
    }
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