#pragma once
#include <v8.h>
#include "SkrGuid/guid.hpp"

namespace skr
{
// 关于 V8 与 CPP 的内存管理交叉:
//  1. 由导出至 V8 的构造函数创建，生命周期跟随 v8 或由 EmbeddedRC 管理
//  2. 由 CPP 创建，导出至 V8，生命周期跟随 CPP 或由 EmbeddedRC 管理
//  3. 数组或者值类型 Field 对象导出至 V8，生命周期跟随其容器（对象），由于 V8 对象会保有其指针，其引用需要传递给容器（对象）
//  4. 某些类管理的子对象，管理类销毁会连带子对象销毁，需要进行标记以传递引用，或者通过 EmbeddedRC 通知销毁
//
// 提出解决方案，要点如下:
//  a. EmbeddedRC 可以直接利用，无需额外投入心智负担
//  b. 未进行内存管理的 CPP 对象以及值类型 Field，数组元素等，不提供默认的内存管理（回收）操作，直接任由其 SegmentFault，
//     Debug 方式则是定期扫描残存的 V8 对象（考虑手动触发 GC），并输出警告
//  c. 针对第 1 点，可以增加一个 OwnerShip 标记，CPP 如果期望主动管理（回收），需要获取 OwnerShip
//     反之，如果期望跟随 v8 对象，可以将 OwnerShip 让渡给 v8
//
// 关于值类型处理，与是否需要暴露指针类型
//  确定一定要支持的：
//      1. 值类型 Field 引用会一直保持其原本对象的引用，如果对象失效，Field 引用也应当失效
//      2. 值类型容器，Get 出来的 Object 内存的其实是容器对象引用和一个索引，这回导致 Add/Remove 后行为出现变化，
//         这与 CPP 内的行为一致，是为了避免 SegmentFault 的妥协做法，在语言交界层需要注意这个问题
//      3. JS 层需要一个区分 object 类型的手段，为 FieldRef 或 ContainerRef 提供 Extract 方法解除引用提取为对象
//  不一定要做的：
//      1. 支持以指针类型直接暴露，一般来说胶水层代码用不到
//      2. 值类型容器和指针类型容器区分导出，否则 Vector<T> 和 Vector<T*> 都导出为 Vector<T> 在 JS 层写代码时候难以引起足够注意
//
// Bind Traits:
//  主要的运用场景为便利导出至 V8 的书写，尤其是 Class 导出的 Codegen，通过模板特化，最大限度的保障代码的整洁、复用、灵活
//  1. 内存管理交叉问题需要一个以 Isolate 为单位的包装，在这个包装内用一张 map 存储 native 与 v8 对象的映射关系，同时
//     还需要存储已经制造出来的 type<->FunctionTemplate 映射，因此必须依赖 RTTR（GUID/TypeDescValue），如果不依赖就需要
//     由 Bind Traits 提供一个用于 Bind 的 GUID
//  2. 与 v8 的转换主要依赖 FunctionTemplate，因此 to_v8 和 from_v8 都需要依赖 Isolate/Context 抽象，
//     更何况转换前需要验证 v8 对象是否存在，所以 Isolate/Context 抽象的指针为必要参数
//  3. 除了 v8 主动创建对象（直接写在 FunctionTemplate 内了），其它的转换都需要通过 Bind Traits，而且实现都大同小异，主要
//     区别就在于 OwnerShip，所以 to_v8 接口可以固定
//  4. to_v8 中如果传入的 OwnerShip 与既存的冲突，应当予以警告，视 DebugLevel 而定，可能需要直接 Error/Fault
//  5. Object 的作用域一般是 Context，Isolate 内需要依据 Context 对 Object 的引用进行二级管理，保持引用
//  6. 针对数组元素的引用返回是否使用特殊的 cache 机制比较好（防止扩容导致地址变更），还是说直接使用一个特殊的类型，
//     存储 index，现场解索引，其实，无论如何，保存 cpp 数组元素引用都是不安全的，因此也不应该在 js 中如此使用
//  7. 对值类型成员的访问有两种选择：
//      a. 每次 get 出来都 copy 一份，这样就不会有生命周期的问题，但是会有性能问题，以及赋值语义的问题
//      b. 每次 get 出来的是一个代理对象，但是这会导致父对象被销毁后，字段的内存访问是不安全的，可以提供一个
//         native.extract() 来独立对象
//      c. 在 b 的基础上，如果父对象被回收，子对象会被标记，在后续访问时，会抛出 JS 异常，而不是 SegmentFault 直接
//         Crash（只有在监听的到父对象销毁时候有用）
//  8. Native OwnerShip 的对象，如果在 Native 销毁时没有主动通知，永远都会引发 SegmentFault，如果进行了通知，将会
//     保证将 SegmentFault 转化为 JS 异常，这样就可以在 JS 层面处理异常，而不是直接 Crash
//  9. 第 8 点在容器情形下很难进行完美的通知，因此容器 Field 对象最好使用一个容器引用+索引的方式，其实说白了，容器引用
//     本身就不应该被保存，可以 JS 层面让任何修改操作都无效化既有的引用，如果是 cpp 层面的修改，那就无能为力了
//  10. 函数返回值有时候视为 JS 的 OwnerShip 比较好，比如一些 load 系的 API，可以通过 meta flag 进行标记
//  11. 延申自 10，由 JS 创建的对象所产出的对象，其 OwnerShip 也应当被标记为 JS，生成用的 API 需要通过 meta flag 来进行标记
//  12. 说了这么多，还是裸指针太垃圾，用 EmbeddedRC 可以解决绝大多数问题
//
// 命名空间：
//  1. xmake.lua 内指定一个根空间，其中导出的所有代码都强制在这个空间下
//  2. xmake.lua 内指定一个默认 pattern，用于生成空间名，比如将 ::skr::(.*) 替代为 $1，来移除代码自带的根空间
//  3. 代码内指定一个 meta flag 来替换原有的空间
//  4. 底层命名空间视为 module 名，其下的命名空间在 import 时使用解构赋值
//
// 继承模型
// // 1. 与 JS 保持一致，使用原型链继承成员函数（由于 CPP 都是通过访问器访问的，所以不存在 Field）
// // 2. V8 提供了 Template 级别的 Inherit，但是只支持单继承链
// // 3. interface 的实现待定，由于不是 JS 的语言特性，可以自由发挥
//  1. 为了不对 cpp 侧的编码造成侵入，js 不识别任何 cpp 的继承关系，由于其脚本的性质，没有继承关系不影响它对 API 的调用
//  2. 继承的 field 和 method 导出时直接展平到导出的类型中去
//  3. 需要知晓 cpp 侧继承情况的时候，用引擎提供的 cpp_instance_of 来进行判断
//  4. 导出至 v8 时需要拿到类的 new 时类型，因此必须依赖 IObject，对于非 IObject 对象，视为 struct，不保证导出行为的继承关系正确
//
// 手写导出/FunctionTemplateBuilder
//  1. 需要一套手写导出工具来支持快速手动导出类（并统一实现）
//  2. 这套工具会生产一个必要信息组，同时携带可能的 Debug 信息，在生产 Template 时会依据它进行生产
//  3. 这些信息可以被一起放在 V8 对象的附带信息中，以便能更好的追踪问题
//  4. 参数类型问题，const T&、T& 等，通过模板来进行转换，因为 v8 对象中是直接存储对象指针，如果是 primitive 则需要处理，但是总体可以用模板控制
//  5. 标记为 out 的参数需要进行分化处理，如果是 struct 类型还好，但如果是 primitive 和 string 类型，就需要外部配合创建变量了
//
// 跨语言继承
//  目的: 跨语言创建的对象可以覆写目标函数，完成 native 无感的逻辑注入
//  问题: cpp 的虚表是个黑盒，没法合法侵入
//  假设:
//    1. 需要继承的对象一定由脚本创建
//    2. 继承发生的瞬间，native 实现已经没有覆写可能
//  方案:
//    1. 每个对象导出脚本时会创建一个代理类，这个类实现了所有继承结构中可被脚本覆写的虚函数，同时冻结了当前的覆写状态
//    2. 需要脚本覆写的函数，导出时特殊处理，根据对象来源进行，如果来源是脚本则调用冻结的函数避免发生虚调用，否则调用 native 实现
//  限制:
//    1. 脚本创建的对象一定要通过代理类创建，否则无法覆写，这对手动导出的对象增加了一定难度
//    2. 为避免多重继承，每个导出类都要实现其继承链上所有的导出虚函数
//    3. meta 阶段需要知晓哪些函数是可以被覆写的
//    4. 冻结必须是 devirtualize 的，因此成员函数指针并无法达成这一点，必须手动调用，这就意味着必须硬编码实现
//
// 导出多维 ArrayDim，多重指针等 field 的问题：
//    1. RTTR 系统提供了 generic type 来辅助导出
//    2. 导出层能处理一重指针和三维数组，以减少创建 generic type 的次数，对于超过这一上限的，需要创建 generic type，此时 js 侧的导出会有问题
//    3. JS 对于指针的导出策略需要商榷

struct V8Isolate;
struct V8Context;

template <typename T>
struct V8Bind {
    enum class EOwnerShip
    {
        V8,         // release by v8, if native use it, will trigger SegmentFault
        Native,     // release by native, if v8 use it, will trigger SegmentFault
        Field,      // memory in parent object by field (or container member), if parent object released, will trigger SegmentFault or js exception
        EmbeddedRC, // managed by EmbeddedRC, v8 object just increase reference count
    };
    enum class EOwnerShipOp
    {
        Init,               // if already exist, ignore
        InitOrWarn,         // if already exist, warn and ignore
        InitOrError,        // if already exist, error and ignore
        ForceOverride,      // if already exist, force override
        ForceOverrideWarn,  // if already exist, force override and warn
        ForceOverrideError, // if already exist, force override and error
    };
    enum class EType
    {
        Primitive, // primitive type: integer, floating, boolean, string
        Object,    // impl IObject，will solve inheritance automatically
        Struct,    // other record types, require guid, will not solve inheritance
    };

    static constexpr EType export_type = EType::Primitive;

    // primitive convert
    static v8::Local<v8::Value> primitive_to_v8(V8Context* ctx, const T& value);
    static T                    primitive_from_v8(V8Context* ctx, v8::Local<v8::Value> value);

    // object/struct bind info
    static v8::Local<v8::FunctionTemplate> make_template(V8Isolate* isolate);

    // object convert
    static v8::Local<v8::Value> obj_find_v8(V8Context* ctx, T* p_obj);
    static v8::Local<v8::Value> obj_to_v8(V8Context* ctx, T* p_obj, EOwnerShip ownership, EOwnerShipOp ownership_op = EOwnerShipOp::InitOrWarn);
    static T*                   obj_from_v8(V8Context* ctx, v8::Local<v8::Value> value);

    // struct convert
    static v8::Local<v8::Value> struct_find_v8(V8Context* ctx, T* p_struct);
    static v8::Local<v8::Value> struct_to_v8(V8Context* ctx, T* p_struct, EOwnerShip ownership, EOwnerShipOp ownership_op = EOwnerShipOp::InitOrWarn);
    static T*                   struct_from_v8(V8Context* ctx, v8::Local<v8::Value> value);
};
} // namespace skr