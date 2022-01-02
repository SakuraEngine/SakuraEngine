# Sakura.Runtime

[![build](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci.yml/badge.svg)](https://github.com/SakuraEngine/Sakura.Runtime/actions/workflows/ci.yml)



## 特性
### 极速编译
八成以上的代码由C构成, 其余两成为简单的C++。搭配手动安排的UnityBuild, 这将全量编译的耗时压缩到秒级。

### 干净
大部分功能暴露C接口。我们认为运行时核心的逻辑完全可以被过程式代码描述, 当然在必要时也会加入一些简单的面向对象设计, 但绝不是状态转换非常隐蔽且复杂的那种。

### 原生
充分考虑易用性并针对硬件优化的实现。

### 易剥离
每个功能模块有清晰且尽量简单的引用关系，以及自己独立的配置头。可以很容易地剥离出某个组件为你所用，例如CGPU。

## 示例
### [热更三角形](samples/hot-triangle)
这是一个多后端的三角形绘制demo。
- 每个后端会拉起一个窗口, 并在一个独立的线程上绘制它；
- drawcall录制的逻辑可以运行在host程序或者wasm虚拟机后端中, host程序和wasm‘脚本’共享[同一份C代码](samples/hot-triangle/triangle_module.wa.c)；
- 实现了一个简单的[filewatcher](samples/hot-triangle/hot_wasm.cpp)，自动对drawcall脚本进行变更检查，调用SDK编译wasm，并基于产出物应用热修复。
![hot-triangle](https://media.githubusercontent.com/media/SakuraEngine/Sakura.Resources/main/showcase/hot-triangle.gif)

## 核心组件
- platform
- math
- cgpu: [[api]](include/cgpu/api.h) [[design]](include/cgpu/README.md)
- swa: [[api]](include/wasm/api.h) [[design]](include/wasm/README.md)

## 嵌入源码的开源库和版本
- log.c f9ea349 (MIT)
- mimalloc v2.0.3 (MIT)
- xxhash 0.8.1 (BSD)
- VulkanMemoryAllocator 5c8b3ba, org.fork (MIT)
- D3D12MemoryAllocator 23e4d91, org.fork (MIT)
- SPIRV-Reflect 173ae4d (Apache-2.0)
- wasm3 a3abb3f, org.fork (MIT)
- WAVM nightly/2021-12-15

## 接入的扩展API以及版本
- nvapi R460, 2021/03/16 
- amd_ags 6.0.1