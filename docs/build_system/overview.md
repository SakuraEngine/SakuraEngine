# SakuraEngine 构建系统 (SB) 概述

## 简介

SakuraEngine 使用自研的构建系统 **SB** (Sakura Build)，这是一个基于 C# 开发的高性能构建系统，专门为大型游戏引擎项目设计。

## 核心特性

### 目标导向架构
- **Target**: 构建的基本单元，代表一个可编译的模块或应用
- **TargetScript**: 使用特性标记的 C# 类，用于定义构建目标
- **TargetCategory**: 目标分类系统，支持模块、工具、示例等多种类型

### 任务发射器 (TaskEmitter)
构建系统的核心执行单元，负责特定类型的构建任务：

- **CppCompileEmitter**: C++ 编译任务
- **CppLinkEmitter**: 链接任务
- **CodegenMetaEmitter**: 代码生成元数据
- **CodegenRenderEmitter**: 代码生成渲染
- **VSEmitter**: Visual Studio 工程生成
- **ISPCEmitter**: ISPC 编译任务
- **UnityBuildEmitter**: Unity Build 优化

### 依赖管理
- **EntityFramework**: 使用 EF Core 管理构建依赖关系
- **DependDatabase**: 持久化依赖数据库，支持增量构建
- **DependencyModel**: 
  - `PerTarget`: 目标内部依赖
  - `ExternalTarget`: 跨目标依赖

### 工具链支持
- **IToolchain**: 工具链抽象接口
- **VisualStudioDoctor**: Windows 平台 MSVC 工具链
- **XCodeDoctor**: macOS 平台工具链
- **ArgumentDriver**: 编译器参数管理

## 构建流程

1. **Bootstrap**: 初始化构建环境，检测工具链
2. **LoadTargets**: 扫描并加载所有构建目标
3. **RunDoctors**: 执行工具链检查和配置
4. **AddTaskEmitters**: 注册任务发射器
5. **RunBuild**: 执行构建图

## 目录结构

``` txt
.sb/                    # 临时文件目录
  VisualStudio/        # VS 工程文件
  tools/               # 下载的工具
  downloads/           # 下载缓存

.build/                # 构建输出目录
  [ToolchainName]/     # 按工具链分类
    bin/               # 可执行文件
    lib/               # 库文件
    obj/               # 对象文件

.pkgs/                 # 包管理目录
  .build/              # 包构建缓存
```

## 配置系统

- 支持全局配置和目标特定配置
- 通过 `Target.Arguments` 传递编译参数
- 支持条件编译和平台特定设置

## 创建新模块

``` c#
  // 在相应目录创建 build.cs 文件
  [TargetScript(TargetCategory.Engine)]
  public static class MyModule
  {
      static MyModule()
      {
          Engine.Module("MyModule", "MY_MODULE_API")
              .TargetType(TargetType.Dynamic)
              .IncludeDirs(Visibility.Public, "include")
              .AddCppFiles("src/**.cpp")
              .Depend(Visibility.Private, "SkrCore");
      }
  }
```

## 配置编译选项

```c#
  Target.CppVersion("20")              // C++20
      .Exception(false)                // 禁用异常
      .RTTI(false)                     // 禁用 RTTI
      .WarningLevel(WarningLevel.All)  // 警告级别
      .Optimization(OptimizationLevel.Fastest); // 优化级别
```

## 重要注意事项

### ArgumentDriver 的作用

ArgumentDriver 是 SB 的核心设计之一，它：

  1. 抽象编译器差异：不同编译器的参数格式不同，ArgumentDriver 提供统一接口
  2. 支持查询模式：即使不实际编译，也能获取编译参数
  3. 用于工程生成：VS/XCode 工程生成器通过它获取正确的编译配置

### 依赖系统

  - 使用 Depend 方法添加模块依赖
  - 使用 Visibility 控制依赖传播：
    - Public: 传播给依赖者
    - Private: 仅当前目标使用
    - Interface: 仅传播，自己不使用

### 性能考虑

  1. 增量构建：充分利用缓存系统，避免重复构建
  2. 并行构建：SB 自动并行化独立任务
  3. Unity Build：通过 UnityBuildEmitter 支持统一构建

### 扩展性

  新的 TaskEmitter 可以通过以下方式添加：

```c#
  Engine.AddTaskEmitter("TaskName", new CustomEmitter())
      .AddDependency("OtherTask", DependencyModel.PerTarget);
```