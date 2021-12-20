# Sakura.Runtime

## 特性
### 极速编译
八成以上的代码由C构成, 其余两成为简单的C++。搭配手动安排的UnityBuild, 这将全量编译的耗时压缩到秒级。

### 干净
大部分功能暴露C接口。我们认为运行时核心的逻辑完全可以被过程式代码描述, 当然在必要时也会加入一些简单的面向对象设计, 但绝不是状态转换非常隐蔽且复杂的那种。

### 原生
充分考虑易用性并针对硬件优化的实现。你的硬件一定会非常开心，尤其是GPU。

### 易剥离
每个功能模块有清晰且尽量简单的引用关系，以及自己独立的配置头。可以很容易地剥离出某个组件为你所用，例如CGPU。

## 核心组件
- platform
- math
- cgpu: [[api]](include/cgpu/api.h) [[design]](src/gpu/cgpu/README.md)

## 嵌入源码的开源库和版本
- mimalloc v2.0.3 (MIT)
- xxhash 0.8.1 (BSD)
- VulkanMemoryAllocator 5c8b3ba, org.fork (MIT)
- D3D12MemoryAllocator 23e4d91, org.fork (MIT)
- SPIRV-Reflect 173ae4d (Apache-2.0)

## 接入的扩展API以及版本
- nvapi R460, 2021/03/16 
- amd_ags 6.0.1