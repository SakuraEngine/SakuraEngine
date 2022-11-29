<h1 align="center">SakuraEngine</h1>

<a href="https://olivermak.es/">
  <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/logos/skr_icon.svg" width="100%" height="100%">
</a>

<h2 align="center"> 为下一代平台的功能性能需求而生 </h2>

## 特性


### 原生

- 充分考虑易用性并针对硬件优化的实现；
- 确保强缩放性且面向最先进平台功能特性的设计；
- 集成大量原生开发需要的 SDK。

### 直白

- 面向过程的实现与设计；
- **C API**

### 现代

- 基于 ECS 思想，特性丰富且高度正交的[数据驱动编程管线](https://github.com/SakuraEngine/Sakura.Runtime/tree/main/include/ecs)带来最大化的访存效率；
- 混合 Fibers 和 Thread 的任务调度系统，配合 ECS 的依赖管线，赋予运行时前所未有的多线程任务吞吐量；
- 完全面向现代 GPU 平台、几无性能开销的超薄跨平台 Graphics API；
- 清晰的 Render Graph 前端让您可以在不接触同步原语和复杂描述符的情况下完成高度异步的现代 GPU 管线编程，并充分利用 Memory Aliasing 等高级特性；
- 完全异步、针对 NVMe 驱动以及 GPU 异步拷贝引擎优化的 I/O 服务，轻松享受 Direct Storage 的极限吞吐，打破 SSD 性能桎梏。

<div align=center>

https://user-images.githubusercontent.com/39457738/192722537-6ab035a5-2789-43d0-b331-347e3669f3ae.mp4

</div>

## 模块列表

| 模块 | 核心库 | 描述 |
| --- | :---: | --- |
| `runtime` | - | 核心模块 |
| `runtime/cgpu` | - | 核心子模块，面向现代 GPU 平台的跨平台 Graphics API |
| `runtime/dual` | - | 核心子模块，真正面向数据驱动编程的 ECS 对象模型 |
| `runtime/math` | [DirectXMath] | 核心子模块, 数学库 |
| `runtime/io` | - | 核心子模块，同步/异步 I/O 服务 |
| `runtime/task` | - | 核心子模块，基于协程的计算密集任务系统 |
| `runtime/binary` | - | 核心子模块, 二进制序列化 |
| `runtime/platform` | - | 核心子模块, 系统接口 |
| `runtime/resource` | - | 核心子模块, 异步资源管理 |
| `runtime/type` | - | 核心子模块, 提供 rtti 相关功能 |
| `SkrScene` | - | 场景模块，场景资源管理 |
| `SkrRenderer` | - | 渲染器模块，渲染资源管理，驱动渲染管线 |
| `SkrImgui` | [imgui] | 用于调试的运行时UI |
| `SkrRenderGraph` | - | RHI 的上层封装 |
| `SkrLive2d` | [live2d] | Live2d 的渲染后端 |
| `SkrInputSystem` | [gaininput] | 输入的上层封装 |
| `SkrAnim` | [ozz-animation] | 动画资源管理，动画计算的基础功能 |
| `SkrToolCore` | - | 管理并驱动资源烘焙流程 |
| `SkrAnimTool` | [ozz-animation] | 导入并烘焙动画相关资源 |
| `SkrGLTFTool` | [cgltf] | 导入gltf模型并烘焙 |
| `SkrTextureCompiler` | [ISPCTextureCompressor] | 导入并烘焙贴图 |
| `SkrShaderCompiler` | - | 导入并烘焙材质相关资源 |
| `codegen` | [libtooling] [mako-template] | 提供代码生成 |
| `UsdTool` | [usd] | USD相关资源的导入与烘焙 |

[DirectXMath]: https://github.com/microsoft/DirectXMath
[imgui]: https://github.com/ocornut/imgui
[live2d]: https://www.live2d.com/en/
[gaininput]: https://github.com/jkuhlmann/gainput
[ozz-animation]: https://github.com/guillaumeblanc/ozz-animation
[cgltf]: https://github.com/jkuhlmann/cgltf
[ISPCTextureCompressor]: https://github.com/GameTechDev/ISPCTextureCompressor
[libtooling]: https://clang.llvm.org/docs/LibTooling.html
[mako-template]: https://www.makotemplates.org/
[usd]: https://github.com/PixarAnimationStudios/USD

## 组件支持矩阵

### [构建](https://github.com/SakuraEngine/Sakura.Runtime/actions)

| Platform                                                     | CI(Dev)                                                      | CI(Shipping)                                                 |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/Windows.png" height="20" /> clang-cl | [![windows-dev-build](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows.yml)[![windows-dev-build](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-vs16.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-vs16.yml) | [![windows-build-shipping](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-shipping.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-shipping.yml)[![windows-build-shipping](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-vs16-shipping.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-vs16-shipping.yml) |
| <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/Windows.png" height="20" /> cl | [![windows-dev-build](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-cl.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-cl.yml)[![windows-dev-build](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-cl-vs16.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-cl-vs16.yml) | [![windows-build-shipping](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-cl-shipping.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-cl-shipping.yml)[![windows-build-shipping](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-cl-vs16-shipping.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-cl-vs16-shipping.yml) |
| <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/MacOS.png" height="20" /> apple-clang | [![macos-dev-build](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-macos.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-macos.yml) | [![macos-build-shipping](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-macos-shipping.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-macos-shipping.yml) |

### [CGPU](include/cgpu/README.md)

| Platform | <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/DirectX12U.png" height="18" /> D3D12 | <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/DirectX11.png" height="18" />D3D11 | <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/Vulkan.png" height="18" />Vulkan |<img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/Metal.png" height="18" />Metal |
|----------|:-----:|:-----:|:------:|:-----:|
| <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/Windows.png" height="20" /> Windows |:heavy_check_mark: | :x: | :heavy_check_mark: | N/A |
| <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/MacOS.png" height="20" /> macOS | N/A | N/A | :heavy_check_mark: | :heavy_exclamation_mark: |

### [ImageCoder](https://github.com/SakuraEngine/Sakura.Runtime/tree/main/modules/image_coder)

| Platform                                                     | PNG             | JPEG                     | BMP                      | ICO                      | EXR                      | TGA                      |
| ------------------------------------------------------------ | --------------- | ------------------------ | ------------------------ | ------------------------ | ------------------------ | ------------------------ |
| <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/Windows.png" height="20" /> Windows | libpng(v1.5.2)  | libjpeg-turbo | :heavy_exclamation_mark: | :heavy_exclamation_mark: | :heavy_exclamation_mark: | :heavy_exclamation_mark: |
| <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/MacOS.png" height="20" /> macOS | libpng(v1.5.27) | libjpeg-turbo | :heavy_exclamation_mark: | :heavy_exclamation_mark: | :heavy_exclamation_mark: | :heavy_exclamation_mark: |



## 示例 (从上到下逐渐贴近底层)
### [Live2D Viewer](https://github.com/SakuraEngine/Sakura.Runtime/tree/2fee48dbc3eb6b82d0c722e723965c68e2c9a068/samples/application/live2d-viewer)

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

### [Cross-Process Qt Editor](samples/render_graph/cross-process)
这个 demo 展示了如何从 CGPU/RenderGraph Application 中导出纹理到 Qt 中进行使用。[Viewport](editors/ed-core) 使用 OGL Widget 配合 Import Memory EXT 进行实现。

<div align=center>

![QtCrossProcess](https://user-images.githubusercontent.com/39457738/192107303-d10e2dc5-8132-4ed4-b028-e0ead030f507.png)

</div>

### [Cross-Process Presentation](samples/editor/qt-viewport)
这个 demo 展示了引擎将会采用的跨进程技术雏形，即使用 LMDB 和 GRPC 的数据共享以及跨进程 CGPU 资源的视图呈现。

<div align=center>

[cross-processs](https://user-images.githubusercontent.com/39457738/191159690-638f9bf4-3120-49ca-8435-835f25121b89.mp4)

</div>

### [RenderGraph Deferred](samples/render_graph/rg-deferred)
这个 demo 展示了如何使用 RenderGraph 进行 Deferred 渲染，其中光照计算的部分有 ComputeShdaer 和 PixelShader 两种实现。实际的光照着色效果尚未在 demo 中完成，重点在于验证延迟流程的可行性。这个 demo 同样展示了如何使用自定义 Profiler 对 RenderGraph 的执行细节进行 Profile。

<div align=center>

![RenderGraphDeferred](https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/showcase/rg-deferred.png)

</div>


### [RenderGraph Triangle](samples/render_graph/rg-triangle)
这个 demo 展示了如何使用 RenderGraph 进行三角形渲染。


### [纹理](samples/cgpu-texture)
这个 demo 演示了如何在 CGPU 中使用纹理采样，demo 也演示了怎么在 CGPU 中启用 Static/Immutable Samplers。

<div align=center>

![cgpu-texture](https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/showcase/cgpu-texture.png)

</div>

### [热更三角形](samples/hot-triangle)

这是一个多后端的三角形绘制 demo。
- 每个后端会拉起一个窗口, 并在一个独立的线程上绘制它；
- drawcall 录制的逻辑可以运行在 host 程序或者 wasm 虚拟机后端中, host 程序和 wasm ‘脚本’共享[同一份C代码](samples/hot-triangle/triangle_module.wa.c)；
- 实现了一个简单的 [filewatcher](samples/hot-triangle/hot_wasm.cpp)，自动对 drawcall 脚本进行变更检查，调用 SDK 编译 wasm，并基于产出物应用热修复。

<div align=center>

![hot-triangle](https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/showcase/hot-triangle.gif)

</div>

## 核心组件
- platform
- math
- cgpu: [[api]](include/cgpu/api.h) [[design]](include/cgpu/README.md)
- swa: [[api]](include/wasm/api.h) [[design]](include/wasm/README.md)

## 嵌入源码的开源库和版本
- LMDB v0.9.29 (BSD)
- log.c f9ea349 (MIT)
- mimalloc v2.0.6 (MIT)
- xxhash 0.8.1 (BSD)
- concurrentqueue d49fa2b Simplified BSD
- VulkanMemoryAllocator 3.0.1, release
- D3D12MemoryAllocator 2.0.1 release
- SPIRV-Reflect b68b5a8 (Apache-2.0)
- wasm3 a3abb3f, org.fork (MIT)
- DirectXMath 596aa5d (MIT)
- GSL 4.0.0 Release (MIT)
- FiberTaskingLib 9d7b27d (Apache-2.0)
- sole 1.0.1 (zlib License)
- parallel-hashmap 1.3.4 (Apache-2.0)
- folly (Apache-2.0)
- simdjson v3.0.0 (Apache-2.0)
- fast_float v3.4.0
- zlib v1.2.8
- cgltf v1.13 (MIT)
- marl e007bd3 (Apache-2.0)
- cpu_features v0.7.0 (Apache-2.0)

## 作为动态库引入的开源库和版本
- Qt-Advanced-Docking-System 3cd6d76 LGPL-2.1 

## 接入的扩展API以及版本
- nvapi R510
- amd_ags 6.0.1

## 内置 SDK /软件版本
- ispc 1.18.0
- python 3.10.8

## 构建
### 前置
- xmake
- vcpkg（可选，当前 grpc 和 usd 通过 vcpkg 安装）
- python，并安装 mako `pip install mako`

### 编译
使用以下命令编译
```
> xmake l setup.lua
> xmake f -m debug --build_usdtool=n -c
> xmake project -k compile_commands
> xmake 
```
编译 usdtool 需要通过 vcpkg 安装

## 编辑环境
推荐使用 vscode + clangd 作为编辑环境，使用命令 `xmake project -k compile_commands` 来生成 clangd 需要的数据集
