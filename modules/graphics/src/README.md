# CGPU

## RenderPass & PipelineState 
- RenderPass是多个或者一个PipelineState的组合
- 在PC这种没有Subpass的平台上，一个Pass对应一个PipelineState
- Subpass等行为会导致PipelineState在Pass内产生变化与切换
- 在更加LowLevel的平台上，可以以寄存器组为Scope直接操控Pipeline上的状态
    - 例如，在CommandBuffer的寄存器区内直接嵌入SetScissor等状态切换代码
    - 这使得操控粒度进一步达到了传统API的级别
- 所以vkSetPass+vkSetPipelineState/SetPSO是一种Flush的行为
- 而一些平台进一步地提供SubPass等细粒度控制管线状态的API

### 寄存器组
- ShRegisters：Shader状态，对应VkPipelineShaderStageCreateInfo
- CxRegisters: 调度管线状态
- UcRegisters：

### API
综上我们认为：
- 直接将Pass归类为一种Flush行为, 它有自己的RootSignature/Binder，并在开始时对GPU状态进行Flush
- 提供Pass内转换状态的扩展API，此API按照它的功用使用枚举对参数集分组
- 在进行Pass切换时，将状态分组并筛排可能会获得性能提升。虽然API（如D3D12 PSO）行为设计为Flush，但没有理由不认为驱动会diff PSO并在GPU上应用最少的状态切换

## Shader反射和管线反射
由于每个API的管线绑定部分天差地别，这部分我们必须借助运行时反射来辅助创建这些对象。

## 关于资源表和绑定
- D3D12的寄存器类似占位符，实际上DescriptorTable完全是在运行时映射的
- Vulkan的set和binding则严格对应运行时生成的表

因此

- 对于D3D12，要点是指示运行时生成与[[vk::binding]]一致的表结构。可以将space index映射到set index上去来实现

即

        [[vk::binding(1, 0)]]
        RWByteAddressBuffer buf : register(u0, space1);
        =>
        SHADER_RESOURCE(RWByteAddressBuffer, buf, 1/*set or space*/, u0, 1/*register or binding*/)


## 关于推送常量
推送常量是一种将小变量直接嵌入到管线布局的数据更新方法。
- 推常量使用非常方便，对uber shader的多分支很有用
- NVIDIA硬件下像素着色器访问根常量非常快
- AMD也在GPUOpen表示RDNA架构下改变绘制的常量可以放在根常量下
- 要减少根签名/管线布局的状态设置次数，NV和AMD都推荐按根状态对draw进行排序

暂时不对推送常量进行支持，但是方法如下：
- D3D12实际上要使用RootConstant来进行内联绑定，常量值实际上是在寄存器组bx上的，在shader中它和常规的cbuffer无二致
- VK的推送常量只需要着色器中写明attribute [[vk::push_constant]]
- 我认为着色器中应当采取vulkan的写法，在编译shader的时候对HLSL文件进行修补，自动生成寄存器bx，来弥补D3D12的失败设计
- 退而求其次的方案是使用宏：ROOT_CONSTANT(type, variable, register)

## 关于推送描述符

短期内不考虑实现此特性，写此文档时VK实现要配合VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT，并且需要很复杂的异步逻辑，且最终性能毫无疑问会更差。

推送描述符是直接将描述符内联到表槽位的更新方法。
- 比一个表占用的DWORD更多，在AMD硬件上占用2DWORD，而表只占用1DWORD
- NV表示在PS Stage访问Root CBV非常快，估计和RootConstant/PushConstant类似
- VK的Push系列API由于实际Storage位置和D3D的不同，支持vkCmdPushDescriptorSetKHR扩展的设备极少

考虑到如上几点，我们认为此特性更适合显式指定开启。

考虑对推送描述符进行单独支持。在D3D中直接设置寄存器，在VK后端则是退化到表结构。
    
        [[vk::binding(0, 0)]]
        RWByteAddressBuffer buf : register(u0);
        =>
        ROOT_DESCRIPTOR(RWByteAddressBuffer, buf, 1/*set*/, u0, 1/*binding*/)

虽然在VK后端使用表结构，但要注意并不能再使用这些set index来创建CGPUDescriptorSet，因为会产生相对D3D12的语义差。

使用cgpu_push_root_descriptor进行更新, 后端会使用Shader反射来对此错误操作进行Validate。