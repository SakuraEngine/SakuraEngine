#include "InitListAnalyzer.hpp"
#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>
#include <clang/AST/RecordLayout.h>

namespace skr::CppSL
{
EInitListSemantic AnalyzeInitListSemantic(const clang::InitListExpr* initList)
{
    if (!initList)
        return EInitListSemantic::Unknown;
    
    // 空初始化列表
    if (initList->getNumInits() == 0)
        return EInitListSemantic::DefaultInit;
    
    // 单元素初始化列表
    if (initList->getNumInits() == 1 && !initList->getType()->isAggregateType())
        return EInitListSemantic::ScalarWrapper;
    
    auto type = initList->getType();
    
    // 数组类型
    if (type->isArrayType())
        return EInitListSemantic::Array;
    
    // 检查是否为向量类型 (通过类型名称判断)
    if (auto recordType = type->getAs<clang::RecordType>())
    {
        auto recordDecl = recordType->getDecl();
        auto name = recordDecl->getName();
        
        // 常见的向量类型名称模式
        if (name.starts_with("float") || name.starts_with("int") || 
            name.starts_with("uint") || name.starts_with("bool") ||
            name.starts_with("double") || name.starts_with("half"))
        {
            // 检查是否为向量 (如 float4) 或矩阵 (如 float4x4)
            if (name.contains("x"))
                return EInitListSemantic::Matrix;
            else if (name.back() >= '2' && name.back() <= '4')
                return EInitListSemantic::Vector;
        }
        
        // 检查是否有用户定义的构造函数
        if (auto cxxRecord = llvm::dyn_cast<clang::CXXRecordDecl>(recordDecl))
        {
            // 有非默认构造函数
            if (cxxRecord->hasUserDeclaredConstructor())
            {
                // 检查是否有匹配参数数量的构造函数
                for (auto ctor : cxxRecord->ctors())
                {
                    if (!ctor->isDefaultConstructor() && 
                        ctor->getNumParams() == initList->getNumInits())
                    {
                        return EInitListSemantic::Constructor;
                    }
                }
            }
            
            // POD 或聚合类型
            if (cxxRecord->isPOD() || cxxRecord->isAggregate())
                return EInitListSemantic::Aggregate;
        }
    }
    
    // 基础类型的初始化列表
    if (type->isScalarType() && initList->getNumInits() == 1)
        return EInitListSemantic::ScalarWrapper;
    
    return EInitListSemantic::Unknown;
}

} // 