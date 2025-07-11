# SakuraEngine 容器API参考

## 容器概览

### 总数统计
- **总容器数**: 50个
- **基本序列容器**: 9个
- **关联容器**: 21个（第一组11个，第二组10个）
- **特殊容器**: 9个
- **并发容器**: 2个
- **适配器容器**: 9个

### 设计哲学
1. **漏斗式API设计**: 渐进式功能暴露，从简单到复杂
2. **内存显式管理**: 通过 `reserve()`, `shrink()`, `release()` 精确控制
3. **迭代器不稳定性**: 修改操作后需重新获取迭代器
4. **零拷贝优化**: 支持 `move()`, `steal()` 等转移语义
5. **定制内存策略**: 支持自定义 allocator 和 growth policy

### 内存策略对比
| 容器类型 | 内存连续性 | 扩容策略 | 迭代器稳定性 |
|---------|-----------|---------|-------------|
| Vector/Array | 连续 | 2倍扩容 | 不稳定 |
| SparseVector | 稀疏 | 按需分配 | 相对稳定 |
| HashSet/Map | 分散 | 重哈希 | 不稳定 |
| BTree | 分散 | 节点分裂 | 稳定 |
| List/Deque | 分散 | 按需分配 | 稳定 |

## 分类详解

### 基本序列容器（9个）

#### 1. **skr::Vector<T>**
- **用途**: 动态数组，最常用的容器
- **特性**: 内存连续，支持随机访问
- **核心API差异**:
  ```cpp
  // STL风格
  std::vector<int> v;
  v.push_back(1);
  v.size();
  v.clear();
  
  // SKR风格
  skr::Vector<int> v;
  v.add(1);              // 注意：不是 push_back
  v.size();              // 相同
  v.clear();             // 相同
  v.reserve(100);        // 预分配
  v.shrink();            // 收缩内存
  v.release();           // 释放所有内存
  ```

#### 2. **skr::Array<T>**
- **用途**: 定长数组包装器
- **特性**: 编译期大小，栈分配
- **核心API**: 类似Vector但无动态操作

#### 3. **skr::SparseVector<T>**
- **用途**: 稀疏数据存储
- **特性**: 仅存储非默认值元素
- **核心API**:
  ```cpp
  skr::SparseVector<int> sv;
  sv.add_at_index(100, 42);  // 在索引100处添加
  sv.contains_index(100);     // 检查索引是否有值
  sv.compact();              // 压缩存储
  ```

#### 4. **skr::String**
- **用途**: UTF-8字符串
- **特性**: 小字符串优化(SSO)
- **核心API**:
  ```cpp
  skr::String s("hello");
  s.append(" world");         // 不是 +=
  s.size_in_bytes();         // 字节大小
  s.size_in_chars();         // 字符数量
  s.c_str();                 // C字符串
  ```

#### 5. **skr::StringView**
- **用途**: 字符串引用
- **特性**: 不拥有内存，轻量级
- **核心API**: 只读操作，无修改方法

#### 6. **skr::RingBuffer<T>**
- **用途**: 循环缓冲区
- **特性**: 固定容量，FIFO
- **核心API**:
  ```cpp
  skr::RingBuffer<int> rb(100);
  rb.push(1);                // 添加到尾部
  rb.pop();                  // 从头部移除
  rb.is_full();              // 检查是否满
  ```

#### 7. **skr::BitVector**
- **用途**: 位向量
- **特性**: 每位一个bool，内存紧凑
- **核心API**:
  ```cpp
  skr::BitVector bv(1000);
  bv.set_bit(100);           // 设置位
  bv.test_bit(100);          // 测试位
  bv.count_set_bits();       // 统计1的数量
  ```

#### 8. **skr::BitSet<N>**
- **用途**: 固定大小位集
- **特性**: 编译期大小，栈分配
- **核心API**: 类似BitVector但大小固定

#### 9. **skr::Blob**
- **用途**: 二进制数据块
- **特性**: 原始字节存储
- **核心API**:
  ```cpp
  skr::Blob blob(1024);
  blob.data();               // 获取数据指针
  blob.resize(2048);         // 调整大小
  ```

### 关联容器第一组（11个）

#### 1-2. **skr::HashSet<T> / skr::MultiHashSet<T>**
- **用途**: 哈希集合（单值/多值）
- **特性**: O(1)查找，无序
- **核心API差异**:
  ```cpp
  // STL
  std::unordered_set<int> s;
  s.insert(1);
  s.find(1);
  s.erase(1);
  
  // SKR
  skr::HashSet<int> s;
  s.add(1);                  // 注意：不是 insert
  s.contains(1);             // 注意：不是 find
  s.remove(1);               // 注意：不是 erase
  ```

#### 3-4. **skr::HashMap<K,V> / skr::MultiHashMap<K,V>**
- **用途**: 哈希映射（单值/多值）
- **特性**: O(1)查找，键值对存储
- **核心API**:
  ```cpp
  skr::HashMap<int, String> m;
  m.add(1, "one");           // 添加键值对
  m.find(1);                 // 返回指针，可能为nullptr
  m.remove(1);               // 删除键
  m.try_add(1, "uno");       // 仅在不存在时添加
  ```

#### 5-6. **skr::SparseHashSet<T> / skr::SparseHashMultiSet<T>**
- **用途**: 稀疏哈希集合
- **特性**: 内存效率优化，适合稀疏数据
- **核心API**: 类似HashSet，增加稀疏操作

#### 7-8. **skr::SparseHashMap<K,V> / skr::SparseHashMultiMap<K,V>**
- **用途**: 稀疏哈希映射
- **特性**: 适合大范围键的稀疏映射
- **核心API**: 类似HashMap，优化稀疏场景

#### 9-10. **skr::Set<T> / skr::MultiSet<T>**
- **用途**: 有序集合（基于BTree）
- **特性**: O(log n)操作，有序遍历
- **核心API**:
  ```cpp
  skr::Set<int> s;
  s.add(1);
  s.lower_bound(5);          // 第一个>=5的元素
  s.upper_bound(5);          // 第一个>5的元素
  ```

#### 11. **skr::Map<K,V>**
- **用途**: 有序映射（基于BTree）
- **特性**: 键有序，平衡树结构
- **核心API**: 类似HashMap但保持有序

### 关联容器第二组（10个）

#### 1. **skr::MultiMap<K,V>**
- **用途**: 有序多值映射
- **特性**: 允许重复键
- **核心API**: 支持equal_range查询

#### 2-5. **skr::FlatSet/FlatMultiSet/FlatMap/FlatMultiMap**
- **用途**: 扁平化关联容器
- **特性**: 底层使用排序vector，缓存友好
- **核心API**:
  ```cpp
  skr::FlatSet<int> fs;
  fs.add(1);                 // 自动保持排序
  fs.reserve(100);           // 预分配
  fs.shrink();               // 收缩内存
  ```

#### 6. **skr::BTree<K,V>**
- **用途**: B树实现
- **特性**: 高扇出度，缓存友好
- **核心API**: 低级接口，通常通过Set/Map使用

#### 7. **skr::IntervalTree<T>**
- **用途**: 区间树
- **特性**: 区间查询优化
- **核心API**:
  ```cpp
  skr::IntervalTree<int> it;
  it.insert(10, 20, data);   // 插入[10,20]区间
  it.query(15, 25);          // 查询重叠区间
  ```

#### 8. **skr::RadixTree<T>**
- **用途**: 基数树/前缀树
- **特性**: 字符串前缀匹配
- **核心API**:
  ```cpp
  skr::RadixTree<int> rt;
  rt.insert("hello", 1);
  rt.find_prefix("hel");     // 前缀查找
  ```

#### 9. **skr::LRUCache<K,V>**
- **用途**: LRU缓存
- **特性**: 自动淘汰最少使用项
- **核心API**:
  ```cpp
  skr::LRUCache<int, String> cache(100);
  cache.insert(1, "one");
  cache.get(1);              // 更新访问时间
  cache.evict();             // 手动淘汰
  ```

#### 10. **skr::BloomFilter<T>**
- **用途**: 布隆过滤器
- **特性**: 概率性数据结构
- **核心API**:
  ```cpp
  skr::BloomFilter<String> bf(10000, 0.01);
  bf.add("hello");
  bf.possibly_contains("hello");  // 可能包含
  ```

### 特殊容器（9个）

#### 1. **skr::Variant<Types...>**
- **用途**: 类型安全的联合体
- **特性**: 编译期类型检查
- **核心API**:
  ```cpp
  skr::Variant<int, float, String> v;
  v = 42;
  v.is<int>();               // 类型检查
  v.get<int>();              // 获取值
  v.visit([](auto& val){});  // 访问者模式
  ```

#### 2. **skr::Optional<T>**
- **用途**: 可选值包装
- **特性**: 明确表达"无值"语义
- **核心API**:
  ```cpp
  skr::Optional<int> opt;
  opt.has_value();           // 检查是否有值
  opt.value();               // 获取值(可能抛异常)
  opt.value_or(0);           // 带默认值获取
  ```

#### 3. **skr::Expected<T,E>**
- **用途**: 结果或错误
- **特性**: 函数式错误处理
- **核心API**:
  ```cpp
  skr::Expected<int, String> result;
  if (result.has_value()) {
    use(result.value());
  } else {
    handle(result.error());
  }
  ```

#### 4. **skr::Span<T>**
- **用途**: 连续内存视图
- **特性**: 不拥有内存
- **核心API**:
  ```cpp
  int arr[10];
  skr::Span<int> span(arr, 10);
  span.size();
  span[5];                   // 下标访问
  ```

#### 5. **skr::WeakPtr<T>**
- **用途**: 弱引用智能指针
- **特性**: 不影响引用计数
- **核心API**:
  ```cpp
  skr::SharedPtr<T> sp;
  skr::WeakPtr<T> wp = sp;
  if (auto locked = wp.lock()) {
    // 使用locked
  }
  ```

#### 6. **skr::UniquePtr<T>**
- **用途**: 独占智能指针
- **特性**: RAII，移动语义
- **核心API**:
  ```cpp
  auto up = skr::make_unique<T>();
  up.release();              // 释放所有权
  up.reset(new T);           // 重置
  ```

#### 7. **skr::SharedPtr<T>**
- **用途**: 共享智能指针
- **特性**: 引用计数
- **核心API**:
  ```cpp
  auto sp = skr::make_shared<T>();
  sp.use_count();            // 引用计数
  sp.reset();                // 释放引用
  ```

#### 8. **skr::IntrusivePtr<T>**
- **用途**: 侵入式智能指针
- **特性**: 对象内置引用计数
- **核心API**: 要求T实现add_ref/release

#### 9. **skr::Any**
- **用途**: 类型擦除容器
- **特性**: 存储任意类型
- **核心API**:
  ```cpp
  skr::Any a = 42;
  a.type();                  // 获取类型信息
  a.cast<int>();             // 类型转换
  ```

### 并发容器（2个）

#### 1. **skr::ConcurrentQueue<T>**
- **用途**: 线程安全队列
- **特性**: 无锁或细粒度锁
- **核心API**:
  ```cpp
  skr::ConcurrentQueue<int> q;
  q.enqueue(42);             // 线程安全入队
  int val;
  q.try_dequeue(val);        // 尝试出队
  ```

#### 2. **skr::AtomicQueue<T>**
- **用途**: 原子操作队列
- **特性**: 完全无锁
- **核心API**:
  ```cpp
  skr::AtomicQueue<int> aq(1024);
  aq.push(42);               // 原子推入
  int val;
  aq.pop(val);               // 原子弹出
  ```

### 适配器容器（9个）

#### 1-2. **skr::stl::vector<T> / skr::stl::list<T>**
- **用途**: STL容器的SKR适配
- **特性**: 使用SKR allocator
- **核心API**: 完全兼容STL接口

#### 3-4. **skr::stl::deque<T> / skr::stl::queue<T>**
- **用途**: 双端队列/队列适配
- **特性**: SKR内存管理
- **核心API**: STL兼容

#### 5. **skr::stl::string**
- **用途**: STL string适配
- **特性**: 与skr::String互操作
- **核心API**: STL兼容

#### 6. **skr::stl::function<Sig>**
- **用途**: 函数包装器
- **特性**: 类型擦除的可调用对象
- **核心API**: STL兼容

#### 7. **skr::stl::optional<T>**
- **用途**: STL optional适配
- **特性**: C++17 optional语义
- **核心API**: STL兼容

#### 8. **skr::FunctionRef<Sig>**
- **用途**: 轻量级函数引用
- **特性**: 不分配内存
- **核心API**:
  ```cpp
  void foo(skr::FunctionRef<void()> fn) {
    fn();                    // 直接调用
  }
  ```

#### 9. **skr::Tuple<Types...>**
- **用途**: 元组容器
- **特性**: 编译期索引
- **核心API**:
  ```cpp
  skr::Tuple<int, float> t(1, 2.0f);
  skr::get<0>(t);            // 获取第一个元素
  ```

## 最佳实践

### 容器选择决策树

```
需要什么数据结构？
├─ 序列容器
│  ├─ 需要动态大小？
│  │  ├─ 是 → Vector（默认选择）
│  │  └─ 否 → Array（固定大小）
│  ├─ 稀疏数据？→ SparseVector
│  ├─ 字符串？→ String/StringView
│  ├─ 循环缓冲？→ RingBuffer
│  └─ 位操作？→ BitVector/BitSet
├─ 关联容器
│  ├─ 需要有序？
│  │  ├─ 是 → Set/Map（BTree实现）
│  │  └─ 否 → HashSet/HashMap（更快）
│  ├─ 稀疏键？→ SparseHash系列
│  ├─ 缓存友好？→ Flat系列
│  └─ 特殊查询？
│      ├─ 区间 → IntervalTree
│      ├─ 前缀 → RadixTree
│      └─ LRU → LRUCache
├─ 并发访问？
│  ├─ 队列 → ConcurrentQueue
│  └─ 无锁 → AtomicQueue
└─ 特殊需求？
   ├─ 多类型 → Variant
   ├─ 可选值 → Optional
   ├─ 错误处理 → Expected
   └─ 智能指针 → SharedPtr/UniquePtr
```

### 常见错误与修复

#### 1. **API名称错误**
```cpp
// 错误：使用STL风格API
vector.push_back(value);
set.insert(value);
map.erase(key);

// 正确：使用SKR风格API
vector.add(value);
set.add(value);
map.remove(key);
```

#### 2. **迭代器失效**
```cpp
// 错误：修改后继续使用迭代器
auto it = vec.begin();
vec.add(42);
++it;  // 迭代器已失效！

// 正确：修改后重新获取
vec.add(42);
for (auto it = vec.begin(); it != vec.end(); ++it) {
    // 使用迭代器
}
```

#### 3. **内存管理遗漏**
```cpp
// 错误：忘记预分配
for (int i = 0; i < 10000; ++i) {
    vec.add(i);  // 多次重分配
}

// 正确：预先reserve
vec.reserve(10000);
for (int i = 0; i < 10000; ++i) {
    vec.add(i);
}
```

#### 4. **查找API混淆**
```cpp
// 错误：HashSet使用find
if (set.find(value) != set.end()) { }

// 正确：使用contains
if (set.contains(value)) { }

// 对于HashMap，find返回指针
if (auto* val = map.find(key)) {
    use(*val);
}
```

#### 5. **智能指针误用**
```cpp
// 错误：循环引用
struct Node {
    skr::SharedPtr<Node> next;
    skr::SharedPtr<Node> prev;  // 循环引用！
};

// 正确：使用WeakPtr打破循环
struct Node {
    skr::SharedPtr<Node> next;
    skr::WeakPtr<Node> prev;    // 弱引用
};
```

### 性能优化建议

1. **预分配容量**
   - Vector/HashMap等使用`reserve()`避免重分配
   - 已知大小时优先使用Array/BitSet

2. **选择合适的容器**
   - 频繁查找：HashMap > Map > Vector
   - 有序遍历：Map/Set > HashMap/HashSet
   - 缓存友好：FlatMap > Map

3. **避免不必要的拷贝**
   - 使用`move()`转移所有权
   - 大对象使用`emplace()`原地构造
   - 传参使用`const&`或`&&`

4. **内存回收**
   - 使用`shrink()`释放多余容量
   - 临时容器及时`release()`
   - 注意容器的生命周期

5. **并发优化**
   - 只读操作不需要并发容器
   - 高竞争场景使用ConcurrentQueue
   - 低竞争场景考虑分片+局部锁

## API速查表

### 核心API差异对比

| 操作 | STL | SKR | 说明 |
|-----|-----|-----|-----|
| 添加元素 | push_back/insert | add | 统一使用add |
| 删除元素 | erase | remove | 统一使用remove |
| 查找元素 | find | find/contains | Set用contains |
| 清空 | clear | clear | 相同 |
| 大小 | size | size | 相同 |
| 判空 | empty | is_empty | 更明确的命名 |
| 迭代器 | begin/end | begin/end | 相同但注意失效 |

### 内存管理API

| 方法 | 功能 | 适用容器 |
|------|------|---------|
| reserve(n) | 预分配容量 | Vector, HashMap等 |
| shrink() | 收缩到实际大小 | Vector, String等 |
| release() | 释放所有内存 | 大部分容器 |
| capacity() | 查询容量 | Vector, String等 |
| compact() | 压缩存储 | SparseVector等 |

### 迭代器模式

```cpp
// 范围for循环（推荐）
for (auto& item : container) {
    process(item);
}

// 迭代器循环
for (auto it = container.begin(); it != container.end(); ++it) {
    process(*it);
}

// 索引循环（仅序列容器）
for (size_t i = 0; i < container.size(); ++i) {
    process(container[i]);
}
```

## 注意事项

1. **生命周期管理**：容器不会自动管理指针元素的生命周期
2. **异常安全性**：大部分操作提供基本异常保证
3. **线程安全性**：除并发容器外，其他容器非线程安全
4. **ABI稳定性**：容器布局可能在版本间变化
5. **调试支持**：Debug模式有额外的边界检查

## 总结

SakuraEngine的容器系统通过精心设计的API和内存管理策略，在保持高性能的同时提供了更好的控制力。理解这些容器的特性和正确使用方式，是高效使用引擎的关键。在迁移或重构代码时，请特别注意API差异和内存管理要求。