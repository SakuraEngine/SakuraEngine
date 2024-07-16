#pragma once
#include "SkrBase/config.h"
#include "SkrContainers/map.hpp"
#include "v8-isolate.h"
#include "v8-platform.h"

namespace skr::rttr
{
struct Type;
}

// 1. deserialize objects
// 2. link deps
// 3. install resource

namespace skr::v8
{
struct V8Context;

// TODO. 内容物
//  1. Map<GUID, FunctionTemplate> 存储类型导出
//  2. Map<TypeSignature, FunctionTemplate> 存储 GenericType 导出（比如 ResourceHandle）
//  [maybe context] 3. Map<void*, UniquePersistent> 存储导出对象的引用，用弱指针存
//   4. Pool<ExportProxy> 导出 UserData 的中间数据，做个 Pooling 用来防止频繁创建销毁
//  [maybe context] 5. 各种 promise
struct SKR_V8_API V8Isolate {
    // ctor & dtor
    V8Isolate();
    ~V8Isolate();

    // delate copy & move
    V8Isolate(const V8Isolate&)            = delete;
    V8Isolate(V8Isolate&&)                 = delete;
    V8Isolate& operator=(const V8Isolate&) = delete;
    V8Isolate& operator=(V8Isolate&&)      = delete;

    // init & shutdown
    void init();
    void shutdown();

    // getter
    ::v8::Isolate* isolate() const { return _isolate; }

    // make context
    V8Context* make_context();

    // register type
    void make_record_template(::skr::rttr::Type* type);
    void inject_templates_into_context(::v8::Local<::v8::Context> context);

private:
    // isolate data
    ::v8::Isolate*              _isolate;
    ::v8::Isolate::CreateParams _isolate_create_params;

    // templates
    Map<::skr::rttr::Type*, ::v8::Global<::v8::FunctionTemplate>> _record_templates;
};
} // namespace skr::v8

// global init
namespace skr::v8
{
SKR_V8_API void init_v8();
SKR_V8_API void shutdown_v8();
} // namespace skr::v8