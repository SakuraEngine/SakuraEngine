# SakuraEngine 代码生成系统

## 概述

SakuraEngine 的代码生成系统是一个基于 Clang LibTooling 和 TypeScript 的元编程框架，用于自动生成 RTTR 注册代码、序列化代码等样板代码。系统通过解析 C++ AST 和自定义属性，大大减少了手动编写重复代码的工作量。

## 系统架构

### 工作流程

```
┌─────────────────┐     ┌──────────────┐     ┌────────────────┐
│   C++ 源文件    │ ──> │  Meta 工具   │ ──> │   .meta 文件   │
│ (带 sreflect)   │     │ (Clang AST)  │     │  (JSON 格式)   │
└─────────────────┘     └──────────────┘     └────────────────┘
                                                      │
                                                      ▼
┌─────────────────┐     ┌──────────────┐     ┌────────────────┐
│ 生成的 C++ 代码 │ <── │  TypeScript  │ <── │  代码生成器    │
│ (.generated.h)  │     │   渲染器     │     │  (*.meta.ts)   │
└─────────────────┘     └──────────────┘     └────────────────┘
```

### 核心组件

1. **Meta 工具** - C++ AST 解析器，提取类型信息和属性
2. **代码生成器** - TypeScript 编写的生成逻辑
3. **SB 集成** - 通过 TaskEmitter 驱动整个流程
4. **元语言编译器** - 处理属性中的元语言表达式

## 基本使用

### 标记需要生成的类型

使用 `sreflect` 宏标记类型：

```cpp
// 标记结构体
sreflect_struct(
    guid = "12345678-1234-5678-1234-567812345678";
    rttr = @enable;                    // 启用 RTTR 生成
    serialize = @enable;               // 启用序列化生成
    rttr.flags = ["ScriptVisible"];    // 额外标记
)
struct PlayerData {
    // 标记字段
    sattr(serialize.no = false)
    float health;
    
    sattr(rttr.flags = ["ReadOnly"])
    uint32_t level;
    
    // 标记方法
    sattr(rttr.script_mixin = true)
    void LevelUp();
    
    // 不带标记的成员不会被处理
    int internal_state;
};

// 标记枚举
sreflect_enum(
    guid = "87654321-4321-8765-4321-876543210987";
    rttr = @enable;
)
enum class GameState : uint8_t {
    sattr(display = "主菜单")
    MainMenu,
    
    sattr(display = "游戏中")
    Playing,
    
    sattr(display = "暂停")
    Paused
};
```

### 配置构建系统

在 `build.cs` 中启用代码生成：

```csharp
// 启用 Meta 解析
Target.AddCodegenMeta(meta => meta
    .SetMetaDirectory(".sb/.meta")
    .AddHeaders("include/**.h", "include/**.hpp"));

// 启用代码渲染
Target.AddCodegenRender(render => render
    .AddScript("rttr.meta.ts")      // RTTR 生成器
    .AddScript("serialize.meta.ts")  // 序列化生成器
    .SetOutputDirectory("src/.generated"));
```

### 生成的代码

系统会自动生成以下类型的代码：

#### RTTR 注册代码

```cpp
// PlayerData.rttr.generated.cpp
namespace {
    static void zz_register_record_PlayerData(RTTRType* type) {
        type->set_name(u8"PlayerData");
        type->set_namespace(u8"game");
        type->set_guid(guid_PlayerData);
        
        type->build_record([&](RTTRRecordData* data) {
            RTTRRecordBuilder<PlayerData> builder(data);
            
            // 注册字段
            builder.field<&PlayerData::health>(u8"health")
                   .flag(ERTTRFieldFlag::Serializable);
                   
            builder.field<&PlayerData::level>(u8"level")
                   .flag(ERTTRFieldFlag::ReadOnly)
                   .flag(ERTTRFieldFlag::ScriptVisible);
            
            // 注册方法
            builder.method<void(), &PlayerData::LevelUp>(u8"LevelUp")
                   .flag(ERTTRMethodFlag::ScriptMixin);
        });
    }
    
    // 自动注册
    RTTRAutoRegister<PlayerData> auto_register_PlayerData(
        &zz_register_record_PlayerData);
}
```

#### 序列化代码

```cpp
// PlayerData.serde.generated.hpp
template<>
struct JsonSerde<PlayerData> {
    static bool read(JsonReader* r, PlayerData& v) {
        r->StartObject();
        
        r->Key(u8"health");
        JsonSerde<float>::read(r, v.health);
        
        r->Key(u8"level");
        JsonSerde<uint32_t>::read(r, v.level);
        
        r->EndObject();
        return true;
    }
    
    static bool write(JsonWriter* w, const PlayerData& v) {
        w->StartObject();
        
        w->Key(u8"health");
        JsonSerde<float>::write(w, v.health);
        
        w->Key(u8"level");
        JsonSerde<uint32_t>::write(w, v.level);
        
        w->EndObject();
        return true;
    }
};
```

## 元语言系统

### 预设配置

使用 `@` 前缀应用预设：

```cpp
// 常用预设
rttr = @enable;        // 启用基本 RTTR
rttr = @full;          // 完整 RTTR（包括所有成员）
serialize = @enable;   // 启用序列化
ecs.comp = @enable;    // 标记为 ECS 组件
```

### 属性语法

```cpp
// 赋值
rttr.flags = ["Flag1", "Flag2"];

// 追加
rttr.flags += ["Flag3"];

// 条件配置
rttr.reflect_fields = true;
rttr.reflect_ctors = true;

// 自定义属性
rttr.attrs = `MyNamespace::MyAttribute{42, "hello"}`;
```

### 复杂表达式

```cpp
sreflect_struct(
    // 使用表达式
    rttr.flags = condition ? ["Flag1"] : ["Flag2"];
    
    // 多行配置
    rttr = {
        reflect_fields = true;
        reflect_methods = true;
        flags = ["Serializable", "ScriptVisible"];
    };
)
MyComplexType { };
```

## 创建自定义生成器

### 1. 定义生成器类

创建 `my_generator.meta.ts`：

```typescript
import { Generator, GenerateManager } from "@framework/generator";
import { CppDatabase, CppRecord } from "@framework/database";

class MyCustomGenerator extends Generator {
    // 注入配置
    override inject_configs(): void {
        this.main_module_db.each_record((record) => {
            // 为每个记录注入自定义配置
            record.ml_configs.my_config = {
                enabled: record.attrs.has("my_attr"),
                options: {}
            };
        });
    }
    
    // 预处理
    override pre_gen(): void {
        // 创建输出文件
        this.add_header("my_generated.h");
        this.add_source("my_generated.cpp");
    }
    
    // 主生成逻辑
    override gen(): void {
        // 遍历所有需要处理的类型
        this.main_module_db.each_record((record) => {
            if (record.ml_configs.my_config?.enabled) {
                this._generate_for_record(record);
            }
        });
    }
    
    // 生成单个类型的代码
    private _generate_for_record(record: CppRecord): void {
        const header = this.get_header("my_generated.h");
        const source = this.get_source("my_generated.cpp");
        
        // 生成头文件内容
        header.append(`
            // Generated code for ${record.name}
            void RegisterMyFeature_${record.name}();
        `);
        
        // 生成源文件内容
        source.append(`
            void RegisterMyFeature_${record.name}() {
                // Implementation
                static MyRegistry registry;
                registry.Register<${record.full_name}>("${record.name}");
            }
        `);
    }
    
    // 后处理
    override post_gen(): void {
        // 生成注册表
        const source = this.get_source("my_generated.cpp");
        source.append(`
            void RegisterAllMyFeatures() {
                ${this._collected_functions.join("\n")}
            }
        `);
    }
}

// 导出加载函数
export function load_generator(gm: GenerateManager) {
    gm.add_generator("my_custom", new MyCustomGenerator());
}
```

### 2. 使用代码模板

创建可复用的代码模板：

```typescript
// templates.ts
export class CodeTemplates {
    static function_declaration(name: string, params: string[]): string {
        return `void ${name}(${params.join(", ")});`;
    }
    
    static class_wrapper(class_name: string, content: string): string {
        return `
            class ${class_name}Wrapper {
            public:
                ${content}
            };
        `;
    }
}

// 在生成器中使用
import { CodeTemplates } from "./templates";

class MyGenerator extends Generator {
    override gen(): void {
        const decl = CodeTemplates.function_declaration(
            "MyFunction", 
            ["int a", "float b"]
        );
        header.append(decl);
    }
}
```

### 3. 处理复杂类型

```typescript
// 处理模板类型
private _process_template_type(record: CppRecord): void {
    if (record.is_template) {
        // 获取模板参数
        const template_params = record.template_parameters;
        
        // 为每个实例化生成代码
        record.template_instances.forEach(instance => {
            this._generate_template_instance(instance);
        });
    }
}

// 处理继承关系
private _process_inheritance(record: CppRecord): void {
    const bases = record.bases;
    
    // 生成虚函数覆盖
    bases.forEach(base => {
        base.virtual_methods.forEach(method => {
            if (record.overrides(method)) {
                this._generate_override(record, method);
            }
        });
    });
}
```

## 高级特性

### 批量生成优化

```typescript
// 配置批量大小避免单个文件过大
export class RttrGenerator extends Generator {
    override gen(): void {
        const batch_size = this.config.batch_size || 50;
        const records = this.collect_records();
        
        // 分批生成
        for (let i = 0; i < records.length; i += batch_size) {
            const batch = records.slice(i, i + batch_size);
            const batch_index = Math.floor(i / batch_size);
            
            this.add_source(`rttr_${batch_index}.generated.cpp`);
            this._generate_batch(batch, batch_index);
        }
    }
}
```

### 条件生成

```typescript
// 根据平台生成不同代码
override gen(): void {
    const platform = this.config.platform;
    
    this.main_module_db.each_record((record) => {
        // 平台特定代码
        if (platform === "windows" && record.has_attr("windows_only")) {
            this._generate_windows_specific(record);
        } else if (platform === "linux" && record.has_attr("linux_only")) {
            this._generate_linux_specific(record);
        }
        
        // 通用代码
        this._generate_common(record);
    });
}
```

### 错误处理和诊断

```typescript
// 提供清晰的错误信息
override gen(): void {
    try {
        this._do_generate();
    } catch (e) {
        // 包含源文件位置信息
        const location = this.current_record?.source_location;
        console.error(`Code generation failed at ${location}:`);
        console.error(`  ${e.message}`);
        
        // 提供修复建议
        if (e.code === "MISSING_GUID") {
            console.error("  Hint: Add guid attribute to the type");
        }
        
        throw e;
    }
}
```

## 调试技巧

### 查看生成的元数据

```bash
# Meta 工具生成的 JSON 文件
cat .sb/.meta/MyModule/MyType.meta

# 查看解析的属性
{
  "name": "PlayerData",
  "attrs": {
    "guid": "12345678-...",
    "rttr": { "enabled": true, "flags": ["ScriptVisible"] }
  }
}
```

### 调试 TypeScript 生成器

```typescript
// 启用调试日志
class MyGenerator extends Generator {
    private debug = true;
    
    override gen(): void {
        if (this.debug) {
            console.log(`Processing ${this.main_module_db.name}`);
            console.log(`Found ${this.main_module_db.records.length} records`);
        }
    }
}
```

### 验证生成的代码

```cpp
// 使用静态断言验证
static_assert(
    rttr_is_registered<PlayerData>::value,
    "PlayerData should be registered with RTTR"
);
```

## 性能优化

### 1. 增量生成

只在源文件改变时重新生成：

```csharp
// SB 自动处理增量构建
Target.AddCodegenMeta(meta => meta
    .EnableIncremental()  // 启用增量模式
    .SetCacheDirectory(".sb/.meta_cache"));
```

### 2. 并行处理

```typescript
// 使用 Promise 并行处理
override async gen(): Promise<void> {
    const promises = this.main_module_db.modules.map(module => 
        this._process_module_async(module)
    );
    
    await Promise.all(promises);
}
```

### 3. 内存优化

```typescript
// 流式处理大文件
override gen(): void {
    // 使用流式写入而非内存缓冲
    const stream = this.create_output_stream("large_output.cpp");
    
    this.main_module_db.each_record_batch(100, (batch) => {
        const content = this._generate_batch(batch);
        stream.write(content);
    });
    
    stream.close();
}
```

## 最佳实践

### DO - 推荐做法

1. **保持生成器单一职责**
   ```typescript
   // 好：每个生成器负责一种功能
   class SerializeGenerator { }
   class RttrGenerator { }
   
   // 避免：一个生成器做所有事情
   class MegaGenerator { }
   ```

2. **使用类型安全的模板**
   ```typescript
   // 使用 TypeScript 类型确保正确性
   function generateMethod<T extends CppMethod>(method: T): string
   ```

3. **提供清晰的错误信息**
   ```typescript
   if (!record.guid) {
       throw new Error(
           `Type '${record.name}' at ${record.location} ` +
           `requires a guid attribute for code generation`
       );
   }
   ```

### DON'T - 避免做法

1. **避免硬编码**
   ```typescript
   // 不好：硬编码路径
   const output = "/home/user/project/generated.cpp";
   
   // 好：使用配置
   const output = path.join(this.config.output_dir, "generated.cpp");
   ```

2. **不要忽略错误处理**
   ```typescript
   // 总是处理可能的错误情况
   try {
       this._generate();
   } catch (e) {
       this.report_error(e);
   }
   ```

## 总结

SakuraEngine 的代码生成系统通过结合 Clang 的强大解析能力和 TypeScript 的灵活性，提供了一个高效、可扩展的元编程解决方案。系统不仅减少了样板代码，还通过类型安全和增量构建等特性保证了开发效率和代码质量。