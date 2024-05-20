#pragma once
#include "SkrBase/config.h"
#include "v8-isolate.h"
#include "v8-platform.h"

namespace skr::rttr
{
struct Type;
}

namespace skr::v8
{
// TODO. 内容物
//  1. Map<GUID, FunctionTemplate> 存储类型导出
//  2. Map<TypeSignature, FunctionTemplate> 存储 GenericType 导出（比如 ResourceHandle）
//  [maybe context] 3. Map<void*, UniquePersistent> 存储导出对象的引用，用弱指针存
//   4. Pool<ExportProxy> 导出 UserData 的中间数据，做个 Pooling 用来防止频繁创建销毁
//  [maybe context] 5. 各种 promise
struct SKR_V8_API V8Isolate {

    // init & shutdown
    void init();
    void shutdown();

private:
    void _make_type_template(::skr::rttr::Type* type);
    // TODO. 没法自定义 module 导出，默认全导

private:
    // isolate data
    std::unique_ptr<::v8::Platform> _platform;
    ::v8::Isolate*                  _isolate;
    ::v8::Isolate::CreateParams     _isolate_create_params;
};

SKR_V8_API void init_v8();
SKR_V8_API void shutdown_v8();

} // namespace skr::v8