#pragma once

namespace clang { class InitListExpr; }

namespace skr::CppSL
{

enum class EInitListSemantic
{
    // 聚合初始化 - POD struct 或 C-style array
    // struct Vec3 { float x, y, z; };
    // Vec3 v = {1.0f, 2.0f, 3.0f};
    Aggregate,
    
    // 构造函数调用 - 有用户定义构造函数的类
    // class MyClass { MyClass(int, float); };
    // MyClass obj = {42, 3.14f};
    Constructor,
    
    // 向量初始化 - shader 内建向量类型
    // float4 color = {1, 0, 0, 1};
    Vector,
    
    // 矩阵初始化 - shader 内建矩阵类型  
    // float4x4 mat = {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}};
    Matrix,
    
    // 数组初始化
    // int arr[3] = {1, 2, 3};
    Array,
    
    // 标量包装 - 单个元素的初始化列表
    // int x = {42};
    ScalarWrapper,
    
    // 默认初始化 - 空的初始化列表
    // Type t = {};
    DefaultInit,
    
    // 未知或复杂情况
    Unknown
};

// 分析 InitListExpr 并返回其语义类型
EInitListSemantic AnalyzeInitListSemantic(const clang::InitListExpr* initList);

} // namespace skr::CppSL