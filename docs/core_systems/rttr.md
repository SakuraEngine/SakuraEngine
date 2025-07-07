# SakuraEngine RTTR (运行时类型反射) 系统

## 概述

RTTR (Runtime Type Reflection) 是 SakuraEngine 的运行时类型反射系统，提供了在运行时查询和操作类型信息的能力。该系统是引擎许多高级功能的基础，包括自动序列化、脚本绑定、编辑器集成等。

## 核心概念

### 类型系统架构

RTTR 将所有类型分为三类：

1. **Primitive (基础类型)**: int、float、bool 等内置类型
2. **Record (记录类型)**: 结构体、类等复合类型  
3. **Enum (枚举类型)**: 枚举类型

每个反射类型都由唯一的 GUID 标识，并包含以下基本信息：

- 类型名称和命名空间
- 内存布局（大小、对齐）
- 类型分类和标志
- 关联的元数据

### 关键组件

```cpp
// 核心类型表示
class RTTRType {
    skr_guid_t guid;        // 类型唯一标识
    const char* name;       // 类型名称
    const char* namespace;  // 命名空间
    size_t size;           // 类型大小
    size_t alignment;      // 对齐要求
    // ... 更多成员
};

// 类型注册表
class RTTRTypeRegistry {
    // 注册和查询类型
    static RTTRType* get_type_from_guid(skr_guid_t guid);
    static void register_type_loader(skr_guid_t guid, TypeLoader loader);
};
```

## 使用方式

### 1. 标记需要反射的类型

使用宏标记需要反射的类型：

```cpp
// 标记结构体
sreflect_struct("guid": "12345678-1234-5678-1234-567812345678")
struct MyClass : public IObject {
    sreflect_property("display": "位置")
    float3 position;
    
    sreflect_property("display": "名称", "readonly": true)  
    SkrString name;
    
    sreflect_method()
    void update(float delta_time);
    
    // 不带标记的成员不会被反射
    int internal_state;
};

// 标记枚举
sreflect_enum("guid": "87654321-4321-8765-4321-876543210987")
enum class MyEnum : uint32_t {
    sreflect_enum_item("display": "选项A")
    OptionA = 0,
    
    sreflect_enum_item("display": "选项B")
    OptionB = 1,
};
```

### 2. 代码生成

SB 构建系统会自动扫描标记的类型并生成反射代码：

```bash
# 构建时自动生成
dotnet run SB build
```

生成的代码包括：
- 类型注册函数
- RTTRTraits 特化
- 友元声明（访问私有成员）

### 3. 运行时反射

#### 获取类型信息

```cpp
// 通过类型获取
auto type = rttr_get_type<MyClass>();

// 通过 GUID 获取
auto type = RTTRTypeRegistry::get_type_from_guid(guid);

// 通过对象获取
IObject* obj = ...;
auto type = obj->get_rttr_type();
```

#### 创建对象

```cpp
// 查找默认构造函数
auto ctor = type->find_ctor();
if (ctor) {
    void* instance = ctor.invoke();
    
    // 对于 IObject 派生类
    IObject* obj = ctor.invoke_object();
}

// 查找带参数的构造函数
auto ctor = type->find_ctor_t<void(float, float, float)>();
if (ctor) {
    void* instance = ctor.invoke(1.0f, 2.0f, 3.0f);
}
```

#### 访问字段

```cpp
// 获取所有字段
type->each_field([](const RTTRFieldData* field, const RTTRType* owner) {
    SKR_LOG_INFO(u8"Field: %s, Type: %s", 
        field->name, field->type->name);
});

// 查找特定字段
auto field = type->find_field("position");
if (field) {
    // 获取字段值
    float3* pos = (float3*)field->get_address(obj);
    
    // 类型安全的访问
    auto typed_field = type->find_field_t<float3>("position");
    if (typed_field) {
        float3& pos = typed_field.get(obj);
    }
}
```

#### 调用方法

```cpp
// 查找方法
auto method = type->find_method("update");
if (method) {
    // 调用方法
    method.invoke(obj, 0.016f);
}

// 类型安全的调用
auto typed_method = type->find_method_t<void(float)>("update");
if (typed_method) {
    typed_method.invoke(obj, 0.016f);
}
```

#### 查询继承关系

```cpp
// 检查是否派生自某个类
if (type->is_derived_from<IObject>()) {
    // 安全转换
    IObject* base = static_cast<IObject*>(obj);
}

// 遍历所有基类
type->each_base([](const RTTRBaseData* base) {
    SKR_LOG_INFO(u8"Base: %s", base->type->name);
});
```

### 4. 与序列化集成

RTTR 与序列化系统深度集成，支持自动序列化：

```cpp
// JSON 序列化
template<typename T>
struct JsonSerde {
    static bool read(JsonReader* r, T& v) {
        auto type = rttr_get_type<T>();
        
        // 自动序列化所有标记的字段
        type->each_field([&](const RTTRFieldData* field, const RTTRType* owner) {
            if (field->has_flag(RTTRFieldFlag::Serializable)) {
                // 读取字段...
                field->generic_read(r, field->get_address(&v));
            }
        });
        
        return true;
    }
};

// 使用示例
MyClass obj;
json_read(json_data, obj);  // 自动序列化
```

### 5. 脚本绑定

RTTR 支持将 C++ 类型导出到脚本：

```cpp
// 标记可脚本化的类
sreflect_struct("guid": "...", "scriptable": true)
struct ScriptableClass : public IScriptableObject {
    sreflect_property("script_visible": true)
    float health;
    
    sreflect_method("script_visible": true)
    void take_damage(float amount);
};

// 在脚本中使用
// JavaScript/Lua 可以直接访问标记的成员
```

## 高级特性

### 泛型容器支持

RTTR 支持标准容器的自动反射：

```cpp
sreflect_struct()
struct GameData {
    // 这些容器会自动获得反射支持
    skr::Vector<int> scores;
    skr::HashMap<SkrString, float> stats;
    skr::Optional<float3> target_position;
};
```

### 自定义元数据

可以为类型和成员附加自定义元数据：

```cpp
sreflect_struct("category": "Gameplay", "icon": "heart.png")
struct HealthComponent {
    sreflect_property("min": 0, "max": 100, "step": 1)
    float health;
    
    sreflect_property("tooltip": "最大生命值上限")
    float max_health;
};

// 运行时查询元数据
auto category = type->get_metadata("category");
auto icon = type->get_metadata("icon");
```

### 性能优化建议

1. **缓存类型查询结果**
   ```cpp
   // 避免重复查询
   static auto cached_type = rttr_get_type<MyClass>();
   ```

2. **使用类型安全的接口**
   ```cpp
   // 优先使用模板版本，避免运行时类型检查
   auto field = type->find_field_t<float>("value");
   ```

3. **批量操作**
   ```cpp
   // 使用 each_field 而不是多次 find_field
   type->each_field([](auto field, auto owner) {
       // 处理所有字段
   });
   ```

## 最佳实践

### DO - 推荐做法

1. **使用 GUID 确保类型唯一性**
   - 每个反射类型都应有唯一的 GUID
   - 使用工具生成 GUID 避免冲突

2. **合理使用反射标记**
   - 只标记需要反射的成员
   - 使用 flag 控制成员的用途（序列化、脚本等）

3. **利用代码生成**
   - 让构建系统自动生成反射代码
   - 避免手动编写类型注册代码

### DON'T - 避免做法

1. **避免在热点代码中使用反射**
   - 反射有性能开销
   - 关键路径应使用直接调用

2. **不要反射所有类型**
   - 只反射需要的类型
   - 过多反射会增加二进制大小

3. **避免运行时修改类型信息**
   - RTTR 设计为只读系统
   - 类型信息应在启动时确定

## 故障排查

### 常见问题

1. **找不到类型信息**
   - 检查是否正确标记了 `sreflect` 宏
   - 确认代码生成是否成功运行
   - 验证 GUID 是否正确

2. **私有成员无法访问**
   - 确认生成的代码包含友元声明
   - 检查是否包含了生成的头文件

3. **序列化失败**
   - 验证字段是否有 Serializable 标记
   - 检查字段类型是否支持序列化

### 调试技巧

```cpp
// 打印类型信息
void debug_type_info(const RTTRType* type) {
    SKR_LOG_INFO(u8"Type: %s::%s", type->namespace, type->name);
    SKR_LOG_INFO(u8"GUID: %s", type->guid.to_string().c_str());
    SKR_LOG_INFO(u8"Size: %zu, Align: %zu", type->size, type->alignment);
    
    // 打印所有字段
    type->each_field([](auto field, auto owner) {
        SKR_LOG_INFO(u8"  Field: %s : %s", 
            field->name, field->type->name);
    });
}
```

## 相关系统

- **序列化系统**: 使用 RTTR 实现自动序列化
- **脚本系统**: 通过 RTTR 导出 C++ 类型到脚本
- **编辑器**: 使用 RTTR 生成属性面板
- **ECS 系统**: 组件通过 RTTR 注册

## 总结

RTTR 是 SakuraEngine 的核心基础设施，提供了强大而灵活的运行时反射能力。通过合理使用 RTTR，可以大大简化序列化、脚本绑定、编辑器集成等功能的实现，同时保持良好的性能和类型安全性。