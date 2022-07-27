<h1 align="center">Sakura.Runtime</h1>

<a href="https://olivermak.es/">
  <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/logos/skr_icon.svg" width="100%" height="100%">
</a>

## 特性
### 极速编译
八成以上的代码由C构成, 其余两成为简单的C++。搭配手动安排的UnityBuild, 这将全量编译的耗时压缩到秒级。

### 干净
大部分功能暴露C接口。我们认为运行时核心的逻辑完全可以被过程式代码描述, 当然在必要时也会加入一些简单的面向对象设计, 但绝不是状态转换非常隐蔽且复杂的那种。

### 原生
充分考虑易用性并针对硬件优化的实现。

### 易剥离
每个功能模块有清晰且尽量简单的引用关系，以及自己独立的配置头。可以很容易地剥离出某个组件为你所用，例如CGPU。

## 组件支持矩阵

### [CGPU](include/cgpu/README.md)

| Platform | CI(Dev) | CI(Shipping) | <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/DirectX12U.png" height="18" /> D3D12 | <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/DirectX11.png" height="18" />D3D11 | <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/Vulkan.png" height="18" />Vulkan |<img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/Metal.png" height="18" />Metal |
|----------|:--:|:--:|:-----:|:-----:|:------:|:-----:|
| <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/Windows.png" height="20" /> Windows | [![windows-dev-build](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows.yml) | [![windows-build-shipping](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-shipping.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-windows-shipping.yml) |:heavy_check_mark: | :heavy_exclamation_mark: | :heavy_check_mark: | N/A |
| <img src="https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/platform-icons/MacOS.png" height="20" /> macOS | [![macos-dev-build](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-macos.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-macos.yml) | [![macos-build-shipping](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-macos-shipping.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci-macos-shipping.yml) | N/A | N/A | :heavy_check_mark: | :heavy_exclamation_mark: |

## 示例
### [热更三角形](samples/hot-triangle)
这是一个多后端的三角形绘制demo。
- 每个后端会拉起一个窗口, 并在一个独立的线程上绘制它；
- drawcall录制的逻辑可以运行在host程序或者wasm虚拟机后端中, host程序和wasm‘脚本’共享[同一份C代码](samples/hot-triangle/triangle_module.wa.c)；
- 实现了一个简单的[filewatcher](samples/hot-triangle/hot_wasm.cpp)，自动对drawcall脚本进行变更检查，调用SDK编译wasm，并基于产出物应用热修复。

![hot-triangle](https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/showcase/hot-triangle.gif)

### [纹理](samples/cgpu-texture)
这个demo演示了如何在CGPU中使用纹理采样，demo也演示了怎么在CGPU中启用Static/Immutable Samplers。

注意ImmutableSamplers需要放在独立的绑定表中，和动态的绑定区分。

![cgpu-texture](https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/showcase/cgpu-texture.png)

### [全异步glTF渲染器](samples/cgpu-3d)
这个demo的所有I/O操作完全异步。从Disk I/O到Memory，再从Memory流送到VideoMemory，全部都是异步完成的。在有AsyncCompute支持的情况下，demo会使用CopyQueue并处理好Release/Acquire Barriers。在单一Queue的情况下，demo会使用单个的Graphics Queue，通过多个分离的TransferSubmit完成异步的上传操作。

![cgpu-glTF](https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/showcase/AsyncGLTF.gif)

## 核心组件
- platform
- math
- cgpu: [[api]](include/cgpu/api.h) [[design]](include/cgpu/README.md)
- swa: [[api]](include/wasm/api.h) [[design]](include/wasm/README.md)

## 嵌入源码的开源库和版本
- log.c f9ea349 (MIT)
- mimalloc v2.0.3 (MIT)
- xxhash 0.8.1 (BSD)
- VulkanMemoryAllocator 3.0.0, release
- D3D12MemoryAllocator 2.0.1 release
- SPIRV-Reflect 173ae4d (Apache-2.0)
- wasm3 a3abb3f, org.fork (MIT)
- DirectXMath 596aa5d (MIT)
- GSL 4.0.0 Release (MIT)
- FiberTaskingLib 9d7b27d (Apache-2.0)
- sole 1.0.1 (zlib License)
- parallel-hashmap 1.3.4 (Apache-2.0)
- folly (Apache-2.0)
- simdjson v1.0.2 (Apache-2.0)
- bitsery v5.2.2 (MIT)
- fast_float v3.4.0
- llfio 28ed462

## 接入的扩展API以及版本
- nvapi R510
- amd_ags 6.0.1

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