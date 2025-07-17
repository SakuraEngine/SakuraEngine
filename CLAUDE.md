查看 @README 了解项目概述。

工程比较大，在读取引用语料时要注意引用的 markdown 是否会和业务有关，如果无关的话不要干扰思考。

注意现在引擎处于高速迭代期，从 ./docs 下引入的文档有一定的过时概率，但是模块本身的设计和思想已经固定不会有大的改变了。如果在执行任务时发现和实际有出入时，请你在 code 终端输出警告提示并给出更新建议。

## 构建
目前项目使用的构建系统是采用 C# 自研的 SB，而不是 xmake。xmake 脚本虽然存在，但是已经要被弃用。

- 构建系统的概要文档 @docs/build_system/overview.md
- VS 工程生成的概要文档 @docs/build_system/vs_solution.md

- SB 的代码目录在 tools/SB 下；
- SB 封装了 EntityFramework ，在 Dependency.cs 的类里操作数据库，提供缓存验证和增量构建等功能。尽量使用 Dependency 而不是直接用 EntityFramework 进行数据库读写。
- SB 使用 TaskEmitter 来承托面向目标的功能，例如代码编译，程序链接，等等。实现业务功能时要尽量基于 TaskEmitter；
- SB 的程序运行有一套称为 ArgumentDriver 的设计，这个设计处理输入参数为不同程序所期望的格式。即使不实际运行编译器或是连接器，也可以从 ArgumentDriver 中获得对应的参数，可以用来生成 CompileCommands 或是用来生成 VS/XCode 工程等。

## 着色器

项目使用的 Shader 语言是自研的基于 libTooling 的 C++ 着色器，它通过把 C++ AST 翻译到 Shader AST 再翻译成 HLSL 和 MSL 等语言。

- C++ AST 解析和处理的目标为 CppSLLLVM，代码在 tools/shader_compiler/LLVM 下；
- 中间层 Shader AST 的目标为 CppSLAst，剔除了 C++ 的一些冗余语法，更加容易生成代码，实现在 tools/shader/AST 下；

## 运行时模块

### RTTR

项目使用的记录 C++ 类型信息和注解的库，代码生成工具在上层利用这个库并生成元信息代码。

- @docs/core_systems/rttr.md

### ECS

项目使用的 ECS 框架，有极高的缓存性能和并发调度能力。现阶段接口比较晦涩。

- @docs/core_systems/ecs_sugoi.md

### ResourceSystem

项目使用的资源系统，通过把打包后的资源（或者某些状况下原本的特殊格式资源例如 ini）读取到内存并反序列化、异步初始化至可用状态的系统。

- @docs/core_systems/resource_system.md

### CGPU

高性能跨平台图形 API 抽象层，提供了统一的 D3D12、Vulkan 和 Metal 接口。采用零开销 C 接口设计，支持 DirectStorage、Tiled Resource 等现代 GPU 特性。

- @docs/graphics/cgpu_design.md

### RenderGraph

项目使用的调度渲染用的 RenderGraph 系统，提供了声明式的渲染管线构建方式。系统自动管理资源生命周期、状态转换和内存优化，让开发者专注于渲染逻辑而非底层资源管理。

- @docs/graphics/render_graph.md

### 代码生成系统

基于 Clang LibTooling 和 TypeScript 的元编程框架，用于自动生成 RTTR 注册代码、序列化代码等。通过解析 C++ AST 和自定义属性，大大减少重复代码编写。

- @docs/build_system/codegen.md

### 任务系统

高性能并行计算框架，支持 Fiber 任务模型。采用工作窃取调度器，提供丰富的并行编程原语。项目里也有对 C++20 协程模型的玩具型支持，在分析业务需求时请忽略 task2 这个不完善的系统。

- @docs/core_systems/task_system.md

### I/O 服务系统

提供高性能的异步文件访问能力，支持传统文件系统、DirectStorage 硬件加速以及直接到 GPU 内存的传输。采用组件化设计，支持批处理、压缩、优先级调度等高级特性。

- @docs/core_systems/io_service.md

### 内存管理系统

基于 Microsoft 的 mimalloc 构建的高性能内存管理系统，提供统一的分配接口、智能指针（RC/SP）、内存池以及完善的调试支持。

- @docs/core_systems/memory_management.md

### 模块系统

灵活的代码组织和加载机制，支持静态链接、动态加载和热重载三种模式。通过依赖图管理模块间关系，并提供子系统机制支持模块内的功能扩展。

- @docs/core_systems/module_system.md