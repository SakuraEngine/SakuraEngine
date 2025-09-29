<h1 align="center">SakuraEngine</h1>

<a href="https://olivermak.es/">
  <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/logos/skr_icon.svg" width="100%" height="100%">
</a>

<h2 align="center"> 为下一代平台的功能性能需求而生 </h2>

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/SakuraEngine/SakuraEngine)
[![ci](https://github.com/SakuraEngine/SakuraEngine/actions/workflows/ci.yml/badge.svg)](https://github.com/SakuraEngine/SakuraEngine/actions/workflows/ci.yml)


## 特性

### 原生

- 充分考虑易用性并针对硬件优化的实现；
- 确保强缩放性且面向最先进平台功能特性的设计；
- 集成大量原生开发需要的 SDK。

### 直白

- 面向过程的实现与设计；
- **C API**

### 先进

- 基于 C# 平台自研的构建系统, 在为代码生成工具提供强大并发驱动力的同时支持最新的 C++ 工具链, 并满足大型项目中重度的自动化需求;
- 基于 LLVM 工具链自研的代码生成工具链, 为拓展 C++ 语言功能与编程模型提供强大动力;
- 基于 LLVM 工具链自研的 CppSL 着色器编程语言, 提供几乎标准的 C++ 语法来编写着色器, 与 CPU 代码完美协同, 并共享代码生成工具链!
- 基于 ECS 思想，特性丰富且高度正交的数据驱动编程管线带来最大化的访存效率；
- 混合 Fibers 和 Thread 的任务调度系统，配合 ECS 的依赖管线，赋予运行时前所未有的多线程任务吞吐量；
- 完全面向现代 GPU 平台、几无性能开销的超薄跨平台 Graphics API；
- 清晰的 Render Graph 前端让您可以在不接触同步原语和复杂描述符的情况下完成高度异步的现代 GPU 管线编程，并充分利用 Memory Aliasing 等高级特性；
- 完全异步、针对 NVMe 驱动以及 GPU 异步拷贝引擎优化的 I/O 服务，轻松享受 Direct Storage 的极限吞吐，打破 SSD 性能桎梏。

<div align=center>

https://user-images.githubusercontent.com/39457738/192722537-6ab035a5-2789-43d0-b331-347e3669f3ae.mp4

</div>


## 构建

### 前置

- dotnet 9.0+

### 编译

使用以下命令编译:

```
> dotnet run SB build --mode=release --category=tools #这会构建反射代码生成器和着色器构建工具并自动拷贝到 SDK 目录下
> dotnet run SB build #默认以 debug 模式构建所有的模块 (除了 tools)
```

使用以下工具命令生成开发所需内容:

```
> dotnet run SB clean --database=targets #清理非包目标的依赖缓存, 触发后续重新编译, 可以在 help 和 runbuild.cs 查看支持的命令列表
> dotnet run SB compile_commands --category=all #生成全局的 compile_commands(C++/CppSL)
> dotnet run SB vscode #生成 launch 和 task
> dotnet run SB vs #生成 VisualStudio 工程 (在 .sb 文件夹下)
> dotnet run SB graph #生成模块依赖图 (graphviz 格式)
```

### 构建命令简化
可以使用别名来简化构建命令的输入

macos/linux 使用 `alias` 命令:
1. 打开 `~/.bashrc` 或 `~/.zshrc` 文件
2. 添加行 `alias sb='dotnet run SB --'`

windows (powershell) 使用 function:
1. 运行命令创建 `$PROFILE` 文件
```
if (-not (Test-Path $PROFILE)) {
    New-Item -Path $PROFILE -Type File -Force
}
```
1. 打开 `$PROFILE` 文件，可以使用 `code $PROFILE` 命令
2. 添加行 `function sb { dotnet run SB -- $args }`

### 自定义编译默认配置
1. 新建 local_config.cs，这是个固定名字，并且是 gitignore 的
2. 添加如下代码，自行更改以适应配置
```csharp
using SB;
using SB.Core;

class LocalConfig
{
    [AfterStage(EBuildStage.PrepareCommandline)] // 在最早的阶段执行
    private static void _startUp()
    {
        D5FTP.Enable = false; // 禁用 D5Ftp 而是使用 Github 源，在 D5Ftp 不稳定时有用
        Engine.DefaultMode = "debug"; // 默认 mode
        Engine.DefaultToolchain = "clang-cl"; // 默认工具链
    }
}
```

## 编辑环境

推荐使用 vscode + clangd 作为编辑环境，使用命令 `dotnet run SB compile_commands` 来生成 clangd 需要的数据集

## 示例 (从上到下逐渐贴近底层)

### [Live2D Viewer](samples/application/live2d-viewer)

集成 Cubism Native SDK 且使用 Render Graph 进行 Live2D 模型高效绘制的程序示例。

- Live2D 渲染器的实现摒弃了传统的变体流程，在 Live2D 模型绘制的过程中实现了 0 管线切换；
- Live2D 渲染器的可动模型顶点信息会使用 CPU Visible VRAM，充分利用 PCIe 带宽进行最高效的顶点上传，并抹消 Copy Engine 在 GPU Timeline 上的时间消耗；
- Live2D 的全部读取和贴图上传由 I/O 服务驱动，服务后台实现会使用最合适的平台 I/O API 最大化 NVMe 队列深度，提升实际带宽；
- 在支持 Direct Storage 的 Windows 平台，还会充分利用自定义解压队列进行 png 的解码。

<div align=center>

![Live2DViewer](https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/showcase/Live2DViewer.png)

</div>

Live2D 模型复合了多种源数据类型，所有数据类型异步地加载和解析。整个模型的加载过程复合了硬盘读取、内存流送到显存、文件解压流送到显存以及直接上传文件到显存。Demo 保证了所有类型的 I/O 操作保持带宽最高效，在此期间发起请求的主线程没有任何停顿与开销。未处理的 Live2D 模型包含了数十个小尺寸 JSON 文件、数个中尺寸模型顶点文件、2张需要解码的 4K PNG 贴图，构成了下图的 I/O 流水线 profile 图表。

<div align=center>

![Live2DViewerIO](https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/showcase/Live2DAsyncIO.png)

</div>

Shipping Build 的最终呈现帧数可以轻松地突破数千帧，这是 Cubism 官方示例基准的十数倍。
