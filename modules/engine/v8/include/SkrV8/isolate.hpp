#pragma once
#include <v8.h>

namespace skr::v8
{
// TODO. 内容物
//  1. Map<GUID, FunctionTemplate> 存储类型导出
//  2. Map<TypeSignature, FunctionTemplate> 存储 GenericType 导出（比如 ResourceHandle）
//  [maybe context] 3. Map<void*, UniquePersistent> 存储导出对象的引用，用弱指针存
//   4. Pool<ExportProxy> 导出 UserData 的中间数据，做个 Pooling 用来防止频繁创建销毁
//  [maybe context] 5. 各种 promise
struct V8Isolate {
};
} // namespace skr::v8