** memory bank 的路径在 ./memory_bank 而不是 ./.memory_bank 或是其他的路径。**

使用 UTF-8 编码进行编程。在根目录下构建：
dotnet run SB build --category=tools # 一般不需要，构建着色器编译器和反射工具开发用
dotnet run SB build # 构建引擎
dotnet run SB build --target= # 构建引擎模块

工程比较大，在读取引用语料时要注意引用的 markdown 是否会和业务有关，如果无关的话不要干扰思考。

注意现在引擎处于高速迭代期，从 ./docs 下引入的文档有一定的过时概率，但是模块本身的设计和思想已经固定不会有大的改变了。如果在执行任务时发现和实际有出入时，请你在 code 终端输出警告提示并给出更新建议。

项目对并发安全性的要求不是无限的，而是每个 API 针对特定的需求语境上下文，一些不需要并发的业务 API 天生是线程不安全的。
或者一些并发的系统会有数据准备阶段，那么即使这个系统是并发的，准备阶段接口一般也不安全。

## 构建

目前项目使用的构建系统是采用 C# 自研的 SB。

- 构建系统的概要文档 @docs/build_system/overview.md
- SB 的代码目录在 tools/SB 下；
- SB 封装了 EntityFramework ，在 Dependency.cs 的类里操作数据库，提供缓存验证和增量构建等功能。尽量使用 Dependency 而不是直接用 EntityFramework 进行数据库读写。
- SB 使用 TaskEmitter 来承托面向目标的功能，例如代码编译，程序链接，等等。实现业务功能时要尽量基于 TaskEmitter；
- SB 的程序运行有一套称为 ArgumentDriver 的设计，这个设计处理输入参数为不同程序所期望的格式。即使不实际运行编译器或是连接器，也可以从 ArgumentDriver 中获得对应的参数，可以用来生成 CompileCommands 或是用来生成 VS/XCode 工程等。

## 着色器

项目使用的 Shader 语言是自研的基于 libTooling 的 C++ 着色器，它通过把 C++ AST 翻译到 Shader AST 再翻译成 HLSL 和 MSL 等语言。

- C++ AST 解析和处理的目标为 CppSLLLVM，代码在 tools/LLVMTools/shader_compiler/LLVM 下；
- 中间层 Shader AST 的目标为 CppSLAst，剔除了 C++ 的一些冗余语法，更加容易生成代码，实现在 tools/shader/AST 下；

## 运行时模块

### 扩展的 C++

项目的 C++ 被代码生成工具拓展了语言功能，有很多标准 C++ 没有的功能。

#### RTTR

项目使用的记录 C++ 类型信息和注解的库，代码生成工具在上层利用这个库并生成元信息代码。

- @docs/core_systems/rttr.md

#### 代码生成系统

基于 Clang LibTooling 和 TypeScript 的元编程框架，用于自动生成 RTTR 注册代码、序列化代码等。通过解析 C++ AST 和自定义属性，大大减少重复代码编写。

- @docs/build_system/codegen.md

### 容器

项目使用自定义容器库替代 STL，提供更好的序列化和算法支持。容器 API 与 STL 有显著差异。

- **基本原则**：优先使用引擎容器（skr::Vector, skr::Map 等）而非 STL
- **API 差异**：和 STL 有很大的差异，使用前根据实际复杂度判断，去查阅源码实现或是翻看下面的详细文档：
- **详细文档**：@docs/core_systems/container_usage.md

### 模块系统

- 灵活的代码组织和加载机制，支持静态链接、动态加载和热重载三种模式。通过依赖图管理模块间关系，并提供子系统机制支持模块内的功能扩展。
- 引擎包含的模块目录列表：@docs/modules.md
- 具体设计和原理：@docs/core_systems/module_system.md