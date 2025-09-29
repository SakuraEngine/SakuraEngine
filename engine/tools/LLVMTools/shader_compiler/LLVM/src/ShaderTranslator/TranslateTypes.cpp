#include "LLVMHelpers.hpp"
#include "ShaderTranslator.hpp"

namespace skr::CppSL
{

CppSL::TypeDecl* KernelTranslator::TranslateType(clang::QualType type)
{
    type = Decay(type);
    if (auto Existed = getType(type))
        return Existed; // already processed

    if (auto RecordDecl = type->getAsRecordDecl())
    {
        TranslateRecordDecl(RecordDecl);
    }
    else if (auto EnumType = type->getAs<clang::EnumType>())
    {
        TranslateEnumDecl(EnumType->getDecl());
    }
    else if (auto isConstantArrayType = type->isConstantArrayType())
    {
        return getType(type);
    }
    else
    {
        type->dump();
        ReportFatalError("Unsupported type: " + std::string(type->getTypeClassName()));
    }

    return getType(type);
}

CppSL::TypeDecl* KernelTranslator::TranslateEnumDecl(const clang::EnumDecl* enumDecl)
{
    using namespace clang;

    if (IsDump(enumDecl))
        enumDecl->dump();
    if (IsIgnore(enumDecl))
        return nullptr; // skip ignored types

    if (auto Existed = getType(enumDecl->getTypeForDecl()->getCanonicalTypeInternal())) return Existed; // already processed

    auto UnderlyingType = getType(enumDecl->getIntegerType());
    addType(enumDecl->getTypeForDecl()->getCanonicalTypeInternal(), UnderlyingType);

    auto EnumName = enumDecl->getName().str(); // Use short name instead of qualified
    for (auto E : enumDecl->enumerators())
    {
        const auto I = E->getInitVal().getLimitedValue();
        auto VarName = (EnumName + "__" + E->getName()).str();
        auto _constant = AST.DeclareGlobalConstant(UnderlyingType, ToText(VarName), AST.Constant(IntValue(I)));
        _enum_constants.emplace(E, _constant);
    }
    return UnderlyingType;
}

CppSL::TypeDecl* KernelTranslator::TranslateRecordDecl(const clang::RecordDecl* recordDecl)
{
    using namespace clang;

    if (IsDump(recordDecl))
        recordDecl->dump();

    for (auto subDecl : recordDecl->decls())
    {
        if (auto SubRecordDecl = llvm::dyn_cast<RecordDecl>(subDecl))
            TranslateRecordDecl(SubRecordDecl);
        else if (auto SubEnumDecl = llvm::dyn_cast<EnumDecl>(subDecl))
            TranslateEnumDecl(SubEnumDecl);
    }

    const auto* ThisType = recordDecl->getTypeForDecl();
    const auto ThisQualType = ThisType->getCanonicalTypeInternal();
    const auto* TSD = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(recordDecl);
    const auto* TSD_Partial = llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(recordDecl);
    const auto* TemplateItSelf = recordDecl->getDescribedTemplate();
    if (auto Existed = getType(ThisType->getCanonicalTypeInternal())) return Existed; // already processed
    if (recordDecl->isUnion()) return nullptr;                                        // unions are not supported
    if (IsIgnore(recordDecl)) return nullptr;                                         // skip ignored types
    if (TSD && TSD_Partial) return nullptr;                                           // skip no-def template specs

    clang::AnnotateAttr* BuiltinAttr = IsBuiltin(recordDecl);
    if (TSD)
    {
        BuiltinAttr = BuiltinAttr ? BuiltinAttr : IsBuiltin(TSD);
        BuiltinAttr = BuiltinAttr ? BuiltinAttr : IsBuiltin(TSD->getSpecializedTemplate()->getTemplatedDecl());
    }
    if (BuiltinAttr != nullptr)
    {
        auto What = GetArgumentAt<clang::StringRef>(BuiltinAttr, 1);
        if (TSD && What == "vec")
        {
            if (TSD && !TSD->isCompleteDefinition()) return nullptr; // skip no-def template specs

            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType().getCanonicalType();
            const uint64_t N = Arguments.get(1).getAsIntegral().getLimitedValue();

            if (getType(ET) == nullptr)
                ReportFatalError(recordDecl, "Error element type!");
            if (N <= 1 || N > 4)
                ReportFatalError(TSD, "Unsupported vec size: {}", std::to_string(N));

            if (getType(ET) == AST.FloatType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.Float2Type, AST.Float3Type, AST.Float4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else if (getType(ET) == AST.IntType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.Int2Type, AST.Int3Type, AST.Int4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else if (getType(ET) == AST.UIntType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.UInt2Type, AST.UInt3Type, AST.UInt4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else if (getType(ET) == AST.BoolType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.Bool2Type, AST.Bool3Type, AST.Bool4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else if (getType(ET) == AST.HalfType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.Half2Type, AST.Half3Type, AST.Half4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else
            {
                ReportFatalError(recordDecl, "Unsupported vec type: {} for vec size: {}", std::string(ET->getTypeClassName()), std::to_string(N));
            }
        }
        else if (TSD && What == "array")
        {
            if (TSD && !TSD->isCompleteDefinition()) return nullptr; // skip no-def template specs

            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType();
            const auto N = Arguments.get(1).getAsIntegral().getLimitedValue();

            if (getType(ET) == nullptr)
                TranslateType(ET->getCanonicalTypeInternal());

            auto ArrayType = AST.ArrayType(getType(ET), uint32_t(N), CppSL::ArrayFlags::None);
            addType(ThisQualType, ArrayType);
        }
        else if (TSD && What == "matrix")
        {
            if (TSD && !TSD->isCompleteDefinition()) return nullptr; // skip no-def template specs

            const auto& Arguments = TSD->getTemplateArgs();
            const auto X = Arguments.get(0).getAsIntegral().getLimitedValue();
            const auto Y = Arguments.get(1).getAsIntegral().getLimitedValue();
            const skr::CppSL::TypeDecl* Types[3][3] = { 
                { AST.Float2x2Type, AST.Float2x3Type, AST.Float2x4Type }, 
                { AST.Float3x2Type, AST.Float3x3Type, AST.Float3x4Type }, 
                { AST.Float4x2Type, AST.Float4x3Type, AST.Float4x4Type } 
            };
            addType(ThisQualType, Types[X - 2][Y - 2]);
        }
        else if (What == "half")
        {
            addType(ThisQualType, AST.HalfType);
        }
        else if (TSD && ((What == "struct_buffer") || (What == "byte_buffer") || (What == "texel_buffer")))
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType();
            const auto CacheFlags = Arguments.get(1).getAsIntegral().getLimitedValue();
            const auto BufferFlag = (CacheFlags == 1) ? CppSL::BufferFlags::Read : CppSL::BufferFlags::ReadWrite;

            if (getType(ET) == nullptr)
                TranslateType(ET->getCanonicalTypeInternal());

            if (What == "byte_buffer")
                addType(ThisQualType, AST.ByteBuffer((CppSL::BufferFlags)BufferFlag));
            else if (What == "texel_buffer")
                addType(ThisQualType, AST.TexelBuffer(getType(ET), (CppSL::BufferFlags)BufferFlag));
            else
                addType(ThisQualType, AST.StructuredBuffer(getType(ET), (CppSL::BufferFlags)BufferFlag));
        }
        else if (TSD && What == "constant_buffer")
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType();

            if (getType(ET) == nullptr)
                TranslateType(ET->getCanonicalTypeInternal());

            if (ET->isVoidType())
                ReportFatalError(recordDecl, "Constant buffer cannot be void type!");

            addType(ThisQualType, AST.ConstantBuffer(getType(ET)));
        }
        else if (TSD && What.starts_with("texture"))
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType();
            const auto CacheFlags = Arguments.get(1).getAsIntegral().getLimitedValue();
            const auto TextureFlag = (CacheFlags == 1) ? CppSL::TextureFlags::Read : CppSL::TextureFlags::ReadWrite;
            if (What == "texture1d")
                addType(ThisQualType, AST.Texture1D(getType(ET), (CppSL::TextureFlags)TextureFlag));
            else if (What == "texture2d")
                addType(ThisQualType, AST.Texture2D(getType(ET), (CppSL::TextureFlags)TextureFlag));
            else if (What == "texture3d")
                addType(ThisQualType, AST.Texture3D(getType(ET), (CppSL::TextureFlags)TextureFlag));
            else if (What == "texture_cube")
                addType(ThisQualType, AST.TextureCube(getType(ET), (CppSL::TextureFlags)TextureFlag));
            else if (What == "texture1d_array")
                addType(ThisQualType, AST.Texture1DArray(getType(ET), (CppSL::TextureFlags)TextureFlag));
            else if (What == "texture2d_array")
                addType(ThisQualType, AST.Texture2DArray(getType(ET), (CppSL::TextureFlags)TextureFlag));
            else if (What == "texture3d_array")
                addType(ThisQualType, AST.Texture3DArray(getType(ET), (CppSL::TextureFlags)TextureFlag));
        }
        else if (What == "sampler")
        {
            addType(ThisQualType, AST.Sampler());
        }
        else if (What == "accel")
        {
            addType(ThisQualType, AST.Accel());
        }
        else if (TSD && What == "ray_query")
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto Flags = Arguments.get(0).getAsIntegral().getLimitedValue();
            auto RayQueryFlags = (CppSL::RayQueryFlags)Flags;
            addType(ThisQualType, AST.RayQuery(RayQueryFlags));
        }
        else if (What == "bindless_array")
        {
            addType(ThisQualType, AST.DeclareBuiltinType(L"bindless_array", 0));
        }
    }
    else
    {
        if (!recordDecl->isCompleteDefinition()) return nullptr; // skip forward declares
        if (TSD && !TSD->isCompleteDefinition()) return nullptr; // skip no-def template specs
        if (!TSD && TemplateItSelf) return nullptr;              // skip template definitions

        auto TypeName = TSD ? std::format("{}_{}", TSD->getName().str(), next_template_spec_id++) :
                              std::format("{}_{}", recordDecl->getName().str(), next_template_spec_id++); // Use short name instead of qualified
        if (getType(ThisQualType))
            ReportFatalError(recordDecl, "Duplicate type declaration: {}", TypeName);

        // resolve field types first because some element types of array/buffer fields maybe unresolved yet.
        for (auto field : recordDecl->fields())
        {
            auto fieldType = field->getType();
            if (!getType(fieldType))
                TranslateType(fieldType);
        }

        auto NewType = AST.DeclareStructure(ToText(TypeName), {});
        if (NewType == nullptr)
            ReportFatalError(recordDecl, "Failed to create type: {}", TypeName);

        auto AsStageInout = IsStageInout(recordDecl);
        if (AsStageInout != nullptr)
        {
            NewType->add_attr(AST.DeclareAttr<CppSL::StageInoutAttr>());
        }

        for (auto field : recordDecl->fields())
        {
            if (IsDump(field))
                field->dump();

            auto fieldType = field->getType();
            if (field->getType()->isReferenceType() || field->getType()->isPointerType())
            {
                ReportFatalError(field, "Field type cannot be reference or pointer!");
            }

            auto _fieldType = getType(fieldType);
            if (!_fieldType)
            {
                TranslateType(fieldType);
                _fieldType = getType(fieldType);
            }

            if (!_fieldType)
                ReportFatalError(recordDecl, "Unknown field type: {} for field: {}", std::string(fieldType->getTypeClassName()), field->getName().str());

            auto _f = AST.DeclareField(ToText(field->getName()), _fieldType);
            if (auto AsInterpolation = IsInterpolation(field))
            {
                auto InterpolationModeString = GetArgumentAt<clang::StringRef>(AsInterpolation, 1);
                auto InterpolationMode = AST.GetInterpolationModeFromString(InterpolationModeString.str().c_str());
                if (AsStageInout != nullptr)
                    _f->add_attr(AST.DeclareAttr<CppSL::InterpolationAttr>(InterpolationMode));
                else
                    ReportFatalError(field, "Interpolation attribute can only be used in stage inout types!");
            }
            NewType->add_field(_f);
        }

        addType(ThisQualType, NewType);
    }
    return getType(ThisQualType);
}

bool KernelTranslator::addType(clang::QualType type, skr::CppSL::TypeDecl* decl)
{
    type = Decay(type);
    if (auto bt = type->getAs<clang::BuiltinType>())
    {
        auto kind = bt->getKind();
        if (_builtin_types.find(kind) != _builtin_types.end())
        {
            ReportFatalError("Duplicate builtin type declaration: {}", std::string(bt->getTypeClassName()));
            return false;
        }
        _builtin_types[kind] = decl;
    }
    else if (auto tag = type->getAsTagDecl())
    {
        if (_tag_types.find(tag) != _tag_types.end())
        {
            ReportFatalError(tag, "Duplicate tag type declaration: {}", std::string(tag->getName()));
            return false;
        }
        _tag_types[tag] = decl;
    }
    else
    {
        ReportFatalError("Unknown type declaration: " + std::string(type->getTypeClassName()));
        return false;
    }
    return true;
}

bool KernelTranslator::addType(clang::QualType type, const skr::CppSL::TypeDecl* decl)
{
    return addType(type, const_cast<skr::CppSL::TypeDecl*>(decl));
}

skr::CppSL::TypeDecl* KernelTranslator::getType(clang::QualType type) const
{
    type = Decay(type);
    if (auto bt = type->getAs<clang::BuiltinType>())
    {
        auto kind = bt->getKind();
        if (_builtin_types.find(kind) != _builtin_types.end())
            return _builtin_types.at(kind);
    }
    else if (auto tag = type->getAsTagDecl())
    {
        if (_tag_types.find(tag) != _tag_types.end())
            return _tag_types.at(tag);
    }
    else if (auto _array = pASTContext->getAsConstantArrayType(type))
    {
        auto ConstantArrayType = pASTContext->getAsConstantArrayType(type);
        if (!getType(ConstantArrayType->getElementType())) // arrays and pointers maybe incomplete
        {
            const_cast<KernelTranslator*>(this)->TranslateType(ConstantArrayType->getElementType()->getCanonicalTypeInternal());
        }
        return (CppSL::TypeDecl*)AST.ArrayType(
            getType(ConstantArrayType->getElementType()),
            ConstantArrayType->getSize().getLimitedValue(),
            ArrayFlags::None
        );
    }
    else if (auto _ptr = type->isPointerType())
    {
        auto pointee = getType(type->getPointeeType());
        if (!pointee) // arrays and pointers maybe incomplete
        {
            const_cast<KernelTranslator*>(this)->TranslateType(type->getPointeeType()->getCanonicalTypeInternal());
        }
        if (pointee->is_resource())
        {
            return (CppSL::TypeDecl*)AST.ArrayType(
                pointee,
                0,
                ArrayFlags::None
            );
        }
    }
    return nullptr;
}

} // namespace skr::CppSL