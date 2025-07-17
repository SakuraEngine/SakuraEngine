# SakuraEngine 资源系统

## 概述

资源系统是 SakuraEngine 的核心模块之一，负责游戏资源的加载、管理和生命周期控制。系统设计强调异步加载、内存效率和热重载支持。

## 核心概念

### 资源句柄 (Resource Handle)

资源句柄是访问资源的主要接口，包含 GUID 或已解析的资源指针：

```cpp
// 基础资源句柄
skr_resource_handle_t handle;

// 类型安全的资源句柄
skr::resource::TResourceHandle<TextureResource> texture_handle;

// 句柄状态检查
bool resolved = handle.is_resolved();
void* resource_ptr = handle.get_resolved(true); // 需要手动转换类型

// 类型安全访问
TextureResource* texture = texture_handle.get_resolved(true);
```

## 总结

SakuraEngine 的资源系统提供了强大而灵活的资源管理能力。通过异步加载、智能依赖管理和类型安全的接口，开发者可以高效地管理游戏资源。记住始终正确管理资源生命周期，合理使用请求者系统，充分利用异步加载特性。