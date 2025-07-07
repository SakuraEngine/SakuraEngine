# SakuraEngine 模块系统

## 概述

SakuraEngine 的模块系统提供了灵活的代码组织和加载机制，支持静态链接、动态加载和热重载三种模式。系统通过依赖图管理模块间关系，并提供子系统机制支持模块内的功能扩展。

## 核心概念

### 模块类型

```cpp
// 基础模块接口
class IModule {
public:
    // 生命周期回调
    virtual void on_load(ModuleManager* manager) = 0;
    virtual void on_unload() = 0;
    
    // 主模块执行入口
    virtual int main_module_exec(int argc, char** argv) { return 0; }
    
    // 元数据访问
    virtual const ModuleMetaData* get_meta_data() = 0;
};

// 静态模块（编译时链接）
class IStaticModule : public IModule {
    // 直接链接到主程序
};

// 动态模块（运行时加载）
class IDynamicModule : public IModule {
    // 从 DLL/SO 文件加载
    SharedLibrary* sharedLib;
};

// 热重载模块
class IHotfixModule : public IDynamicModule {
    // 额外的热重载支持
    virtual void on_reload_begin(HotfixState* state) = 0;
    virtual void on_reload_finish(HotfixState* state) = 0;
};
```

### 模块依赖

```
┌─────────────┐     ┌─────────────┐
│   ModuleA   │────>│   ModuleB   │
└─────────────┘     └─────────────┘
       │                    │
       │                    ▼
       │            ┌─────────────┐
       └──────────>│   ModuleC   │
                    └─────────────┘
```

## 总结

SakuraEngine 的模块系统提供了灵活而强大的代码组织机制。通过支持多种加载模式、自动依赖管理、子系统扩展和热重载功能，系统能够满足大型游戏项目的开发需求。合理使用模块系统可以提高代码的可维护性、可扩展性，并加速开发迭代。