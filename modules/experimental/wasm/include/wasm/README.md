# SWA
多个wasm VM/Engine的统一前端抽象。

# 概念
SWA的wasm流程中概念和其他的编译-链接型语言保持一致，并可以引入和操作系统动态库类似的动态桩子机制。

由于在wasm实际的链接发生在运行时，可以暂时悬空弱桩，延迟到实际执行之前的任意阶段。
- 运行时：Runtime，一个托管运行环境实例；
- 模块：Module/Bundle，一个程序集；
- 强桩：注册函数为API，即Runtime::Link，随后此Runtime会向所有的Module公开此桩子；
- 弱桩：动态地执行Program::Link；
- 程序：Program直接从WASM代码或者从Module创建得来，可以实际执行；

此处模块全部都是共享库，不提供内联和静态库相关的功能。

模块/程序内的概念：
- Memory：本质上是模块内的一片托管内存；
- Function：模块内的一个函数，可以向外暴露桩，被host或是其他的模块invoke；

更上层可能会有的概念：
- 数据库：ECS的数据库等；
- 对象：静态/动态类型的、全局可用的对象；

一些需要区分的内容：
- Marshalling：也就是区分HostPtr(64bit)和LocalPtr(32/64bit)；
    - 使用具有公共头的对象头互相传输可以严格区分对象究竟是在Host堆还是沙箱堆上；
    - 传递HeapObject和offset(也就是语言前端的rawptr)的组合可以定位到精准的地址，但是这需要在传参时进行包装；