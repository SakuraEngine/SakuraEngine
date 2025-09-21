#include "CppSL/langs/MSLGenerator.hpp"

namespace skr::CppSL::MSL
{
struct MTLStageInAttr final : public Attr
{

};
struct MTLWaveValueAttr final : public Attr
{
    MTLWaveValueAttr(const String& w) 
        : what(w)
    {
        
    }
    String what = L"";
};
struct MTLKernelAttr final : public Attr
{
    MTLKernelAttr(ShaderStage s) : stage(s) {}
    ShaderStage stage = ShaderStage::None;
};
struct MTLInputOutputAssemblyAttr final : public Attr
{
    bool is_user = false;
    bool is_attribute = false;
    uint32_t color_idx = 0;
    uint32_t attrib_idx = 0;
    InterpolationMode interpolation = InterpolationMode::invalid;
    SemanticType semantic = SemanticType::Invalid;
};

inline static String GetStageName(ShaderStage stage)
{
    switch (stage)
    {
    case ShaderStage::Vertex:
        return L"vertex";
    case ShaderStage::Fragment:
        return L"fragment";
    case ShaderStage::Compute:
        return L"kernel";
    case ShaderStage::RayGen:
        return L"raygeneration";
    case ShaderStage::ClosestHit:
        return L"closesthit";
    case ShaderStage::Miss:
        return L"miss";
    case ShaderStage::AnyHit:
        return L"anyhit";
    case ShaderStage::None:
        return L"";
    default:
        assert(false && "Unknown shader stage");
        return L"unknown_stage";
    }
}

static const std::unordered_map<InterpolationMode, String> InterpolationMap = {
    { InterpolationMode::invalid, L"" },
    { InterpolationMode::linear, L"[[perspective]]" },
    { InterpolationMode::nointerpolation, L"[[flat]]" },
    { InterpolationMode::centroid, L"[[centroid_perspective]]" },
    { InterpolationMode::sample, L"[[sample_perspective]]" },
    { InterpolationMode::noperspective, L"[[center_no_perspective]]" }
};
static const String UnknownInterpolation = L"[[UnknownInterpolation]]";
inline static const String& GetInterpolationString(InterpolationMode Interpolation)
{
    auto it = InterpolationMap.find(Interpolation);
    if (it != InterpolationMap.end())
    {
        return it->second;
    }
    return UnknownInterpolation;
}

static const std::unordered_map<SemanticType, String> SystemValueMap = {
    { SemanticType::Invalid, L"" },
    { SemanticType::Position, L"[[position]]" },
    { SemanticType::ClipDistance, L"[[clip_distance]]" },
    { SemanticType::CullDistance, L"[[cull_distance]]" },
    
    { SemanticType::RenderTarget0, L"[[color(0)]]" },
    { SemanticType::RenderTarget1, L"[[color(1)]]" },
    { SemanticType::RenderTarget2, L"[[color(2)]]" },
    { SemanticType::RenderTarget3, L"[[color(3)]]" },
    { SemanticType::RenderTarget4, L"[[color(4)]]" },
    { SemanticType::RenderTarget5, L"[[color(5)]]" },
    { SemanticType::RenderTarget6, L"[[color(6)]]" },
    { SemanticType::RenderTarget7, L"[[color(7)]]" },
    
    { SemanticType::Depth, L"[[depth(any)]]" },
    { SemanticType::DepthGreaterEqual, L"[[depth(greater)]]" },
    { SemanticType::DepthLessEqual, L"[[depth(less)]]" },
    { SemanticType::StencilRef, L"[[stencil]]" },
    
    { SemanticType::VertexID, L"[[vertex_id]]" },
    { SemanticType::InstanceID, L"[[instance_id]]" },
    
    { SemanticType::GSInstanceID, L"[[render_target_array_index]]" },
    { SemanticType::TessFactor, L"[[patch_control_point]]" },
    { SemanticType::InsideTessFactor, L"[[patch_control_point]]" },
    { SemanticType::DomainLocation, L"[[position_in_patch]]" },
    { SemanticType::ControlPointID, L"[[patch_id]]" },
    
    { SemanticType::PrimitiveID, L"[[primitive_id]]" },
    { SemanticType::IsFrontFace, L"[[front_facing]]" },
    { SemanticType::SampleIndex, L"[[sample_id]]" },
    { SemanticType::SampleMask, L"[[sample_mask]]" },
    { SemanticType::Barycentrics, L"[[barycentric_coord]]" },
    
    { SemanticType::ThreadID, L"[[thread_position_in_grid]]" },
    { SemanticType::GroupID, L"[[threadgroup_position_in_grid]]" },
    { SemanticType::ThreadPositionInGroup, L"[[thread_position_in_threadgroup]]" },
    { SemanticType::ThreadIndexInGroup, L"[[thread_index_in_threadgroup]]" },
    
    { SemanticType::ViewID, L"[[render_target_array_index]]" }
};
static const String UnknownSystemValue = L"[[UnknownSystemValue]]";
inline static const String& GetSystemValueString(SemanticType Semantic)
{
    auto it = SystemValueMap.find(Semantic);
    if (it != SystemValueMap.end())
    {
        return it->second;
    }
    return UnknownSystemValue;
}

MSLGenerator::MSLGenerator()
{
    kUseNamespace = true;
}

String MSLGenerator::GetTypeName(const TypeDecl* type)
{
    if (auto asArray = dynamic_cast<const ArrayTypeDecl*>(type))
    {
        if (asArray->element_type()->is_resource() && (asArray->count() == 0))
        {
            return std::format(L"Bindless< {} >", GetQualifiedTypeName(asArray->element_type()));
        }
        return L"metal::" + asArray->name();
    }
    else if (auto cbuffer = dynamic_cast<const ConstantBufferTypeDecl*>(type))
    {
        return L"ConstantBuffer<" + GetQualifiedTypeName(cbuffer->element_type()) + L">";
    }
    else if (auto sbuffer = dynamic_cast<const StructuredBufferTypeDecl*>(type))
    {
        if (has_flag(sbuffer->flags(), BufferFlags::ReadWrite))
            return L"RWStructuredBuffer<" + GetQualifiedTypeName(&sbuffer->element_type()) + L">";
        else
            return L"StructuredBuffer<" + GetQualifiedTypeName(&sbuffer->element_type()) + L">";
    }
    else if (auto asMatrix = dynamic_cast<const MatrixTypeDecl*>(type))
    {
        return L"metal::" + asMatrix->name();
    }
    return type->name();
}

String MSLGenerator::GetFunctionName(const FunctionDecl* funcDecl)
{
    if (const StageAttr* StageEntry = FindAttr<StageAttr>(funcDecl->attrs()))
    {
        return funcDecl->name() + L"____impl____";
    }
    return funcDecl->name();
}

void MSLGenerator::VisitShaderResource(SourceBuilderNew& sb, const skr::CppSL::VarDecl* var)
{
    // null option
    // since we have already extract and dumped SRT data as builtin header
}

void MSLGenerator::VisitVariable(SourceBuilderNew& sb, const skr::CppSL::VarDecl* varDecl) 
{
    const auto isGlobal = dynamic_cast<const skr::CppSL::GlobalVarDecl*>(varDecl);
    auto _typename = GetQualifiedTypeName(&varDecl->type());
    String prefix = L"", postfix = L"";
    if (varDecl->qualifier() == EVariableQualifier::Const)
    {
        prefix = isGlobal ? L"static constant " : L"const ";
    }
    else if ((varDecl->qualifier() == EVariableQualifier::Inout) || (varDecl->qualifier() == EVariableQualifier::Out))
    {
        if (!varDecl->type().is_resource())
        {
            prefix = L"thread ";
            postfix = L"&";
        }
    }
    sb.append( std::format(L"{}{}{} {}", prefix, _typename, postfix, varDecl->name()));
    if (auto init = varDecl->initializer())
    {
        if (auto asRayQuery = dynamic_cast<const RayQueryTypeDecl*>(init->type()))
        {

        }
        else
        {
            sb.append(L" = ");
            visitStmt(sb, init);
        }
    }
}
 
// MSLGenerator generates semantics at wrapper so we need not deal with system value attributes here
void MSLGenerator::VisitParameter(SourceBuilderNew& sb, const skr::CppSL::FunctionDecl* funcDecl, const skr::CppSL::ParamVarDecl* param)
{
    auto qualifier = param->qualifier();
    String prefix = L"", postfix = L"";
    switch (qualifier)
    {
    case EVariableQualifier::Const:
        prefix = L"const ";
        break;
    case EVariableQualifier::Out:
    {
        if (!param->type().is_resource())
        {
            prefix = L"thread ";
            postfix = L"&";
        }
    }
    break;
    case EVariableQualifier::Inout:
    {
        if (!param->type().is_resource())
        {
            prefix = L"thread ";
            postfix = L"&";
        }
    }
    break;
    case EVariableQualifier::None:
        prefix = L"";
        break;
    case EVariableQualifier::GroupShared:
        prefix = L"metal_group_shared";
        break;
    }

    if (auto semantic = FindAttr<SemanticAttr>(param->attrs()))
    {
        String content = GetQualifiedTypeName(&param->type()) + L" " + param->name();
        sb.append(content);
        sb.append(GetSystemValueString(semantic->semantic()));
    }
    else
    {
        String content = prefix + GetQualifiedTypeName(&param->type()) + postfix + L" " + param->name();
        sb.append(content);
    }
    if (auto stage_in = FindAttr<MTLStageInAttr>(param->attrs()))
    {
        sb.append(L"[[stage_in]]");
    }
    if (auto wave = FindAttr<MTLWaveValueAttr>(param->attrs()))
    {
        sb.append(wave->what);
    }
}

void MSLGenerator::VisitField(SourceBuilderNew& sb, const skr::CppSL::TypeDecl* typeDecl, const skr::CppSL::FieldDecl* field)
{
    // Normal field handling
    sb.append(GetQualifiedTypeName(&field->type()) + L" " + field->name());

    if (auto IOAssembly = FindAttr<MTLInputOutputAssemblyAttr>(field->attrs()))
    {
        if (IOAssembly->is_attribute)
            sb.append(L"[[attribute(" + std::to_wstring(IOAssembly->attrib_idx) + L")]]");
        else if (IOAssembly->is_user)
            sb.append(L"[[user(" + field->name() + L")]]");

        if (IOAssembly->semantic != SemanticType::Invalid)
            sb.append(GetSystemValueString(IOAssembly->semantic));

        if (IOAssembly->interpolation != InterpolationMode::invalid)
            sb.append(GetInterpolationString(IOAssembly->interpolation));
    }

    sb.endline(L';');
}

void MSLGenerator::VisitDeclRef(SourceBuilderNew& sb, const DeclRefExpr* declRef)
{
    if (auto var = dynamic_cast<const VarDecl*>(declRef->decl()))
    {
        auto set = set_of_vars.find(var);
        if (set != set_of_vars.end())
        {
            // Check if this is a push constant
            if (auto cbuffer = dynamic_cast<const ConstantBufferTypeDecl*>(&var->type()))
            {
                sb.append(std::format(L"srt{}.{}.cgpu_buffer_data", set->second, var->name()));
            }
            else
            {
                sb.append(std::format(L"srt{}.{}", set->second, var->name()));
            }
        }
        else
        {
            sb.append(var->name());
        }
    }
}

void MSLGenerator::VisitAccessExpr(SourceBuilderNew& sb, const AccessExpr* expr)
{
    auto to_access = dynamic_cast<const Expr*>(expr->children()[0]);
    auto index = dynamic_cast<const Expr*>(expr->children()[1]);

    auto type = to_access->type();
    if (auto asTex = dynamic_cast<const TextureTypeDecl*>(type))
    {
        sb.append(L"subscript_wrapper(");
        visitStmt(sb, to_access);   
        sb.append(L", ");
        visitStmt(sb, index);
        sb.append(L")");
        return;
    }
    if (auto asBuffer = dynamic_cast<const BufferTypeDecl*>(type))
    {
        visitStmt(sb, to_access);   
        sb.append(L".cgpu_buffer_data[");
        visitStmt(sb, index);
        sb.append(L"]");
        return;
    }
    CppLikeShaderGenerator::VisitAccessExpr(sb, expr);
}

void MSLGenerator::VisitBinaryExpr(SourceBuilderNew& sb, const BinaryExpr* expr)
{
    auto ltype = expr->left()->type();
    auto rtype = expr->right()->type();
    auto op = expr->op();
    
    // Check if this is a scalar float and vector int operation that needs special handling
    bool needsSpecialHandling = false;
    
    if (ltype && rtype && ltype->is_vector() && rtype == expr->type()->ast().FloatType) {
        needsSpecialHandling = true;
    }
    else if (ltype && rtype && rtype->is_vector() && ltype == expr->type()->ast().FloatType) {
        needsSpecialHandling = true;
    }
    
    // Convert scalar float to vector float of matching size
    if (needsSpecialHandling && op == BinaryOp::MUL) 
    {
        sb.append(L"CppSLMul(");
        visitStmt(sb, expr->left());
        sb.append(L", ");
        visitStmt(sb, expr->right());
        sb.append(L")");
        return;
    }
    if (needsSpecialHandling && op == BinaryOp::DIV) 
    {
        sb.append(L"CppSLDiv(");
        visitStmt(sb, expr->left());
        sb.append(L", ");
        visitStmt(sb, expr->right());
        sb.append(L")");
        return;
    }
    
    // Fall back to parent implementation
    CppLikeShaderGenerator::VisitBinaryExpr(sb, expr);
}

void MSLGenerator::VisitConstructExpr(SourceBuilderNew& sb, const ConstructExpr* ctorExpr)
{
    std::span<Expr* const> args;

    if (args.empty())
        args = ctorExpr->args();

    if (auto AsArray = dynamic_cast<const ArrayTypeDecl*>(ctorExpr->type()))
    {
        const auto N = AsArray->size() / AsArray->element_type()->size();
        sb.append(GetTypeName(AsArray) + L"{");
        ;
        for (size_t i = 0; i < ctorExpr->args().size(); i++)
        {
            auto arg = ctorExpr->args()[i];
            if (i > 0)
            {
                sb.append(L", ");
            }
            visitStmt(sb, arg);
        }
        sb.append(L"}");
    }
    else
    {
        sb.append(GetQualifiedTypeName(ctorExpr->type()) + L"(");
        for (size_t i = 0; i < args.size(); i++)
        {
            auto arg = args[i];
            if (i > 0)
            {
                sb.append(L", ");
            }
            visitStmt(sb, arg);
        }
        sb.append(L")");
    }
}

void MSLGenerator::BeforeGenerateFunctionImplementations(SourceBuilderNew& sb, const AST& ast)
{
    std::vector<const FunctionDecl*> funcs;
    funcs.insert(funcs.end(), ast.funcs().begin(), ast.funcs().end());
    for (const auto& funcDecl : funcs)
    {
        if (const StageAttr* stageAttr = FindAttr<StageAttr>(funcDecl->attrs()))
        {
            GenerateKernelWrapper(sb, funcDecl);
        }
    }

    std::map<uint32_t, CppSL::String> SRTs;
    for (auto& [var, b] : binding_table_)
    {
        set_of_vars[var] = b.space;
        
        auto& STRING = SRTs[b.space];
        if (STRING.empty())
        {
            STRING = L"struct SRT" + std::to_wstring(b.space) + L" {\n";
        }
        if (b.is_push)
        {
            auto Type = dynamic_cast<const ConstantBufferTypeDecl*>(&var->type());
            STRING += L"PushConstant<" + GetQualifiedTypeName(Type->element_type()) + L"> " + var->name() + L";\n";
        }
        else if (b.is_bindless)
        {
            if (auto asArray = dynamic_cast<const ArrayTypeDecl*>(&var->type()))
            {
                if (auto asResource = dynamic_cast<const ResourceTypeDecl*>(asArray->element_type()))
                {
                    STRING += L"Bindless<" + GetQualifiedTypeName(asResource) + L"> " + var->name() + L";\n";
                }
            }
        }
        else
        {
            STRING += GetQualifiedTypeName(&var->type()) + L" " + var->name() + L";\n";
        }
    }
    for (auto& [space, STRING] : SRTs)
    {
        STRING += L"};";
        sb.append(STRING);
        sb.endline();

        auto SPACE = std::to_wstring(space);
        sb.append(
            std::format(L"constant {}& constant {} [[buffer({})]]", L"SRT" + SPACE, L"srt" + SPACE, SPACE)
        );
        sb.endline(L';');
    }
}

extern const wchar_t* kMSLHeader;
extern const wchar_t* kMSLWaveIntrinsics;
extern const wchar_t* kMSLBufferIntrinsics;
extern const wchar_t* kMSLTextureIntrinsics;
extern const wchar_t* kMSLRayIntrinsics;

void MSLGenerator::RecordBuiltinHeader(SourceBuilderNew& sb, const AST& ast)
{
    sb.append(kMSLHeader);
    sb.append(kMSLWaveIntrinsics);
    sb.append(kMSLBufferIntrinsics);
    sb.append(kMSLTextureIntrinsics);
    sb.append(kMSLRayIntrinsics);
    sb.endline();

    AnalyzeFunctions(ast);
}

void MSLGenerator::BeforeGenerateCallArgs(SourceBuilderNew& sb, const skr::CppSL::CallExpr* call)
{
    if (auto callee_decl = dynamic_cast<const FunctionDecl*>(call->callee()->decl()))
    {
        if (HasWaveIntrins(callee_decl))
        {
            auto hasArgs = call->args().size();
            sb.append(L"kCppSLBuiltins");
            if (hasArgs)
                sb.append(L", ");
        }
    }
}

void MSLGenerator::BeforeGenerateParamters(SourceBuilderNew& sb, const skr::CppSL::FunctionDecl* funcDecl)
{
    auto as_kernel = FindAttr<MTLKernelAttr>(funcDecl->attrs());
    if (as_kernel) return;

    if (HasWaveIntrins(funcDecl))
    {
        auto hasArgs = funcDecl->parameters().size();
        sb.append(L"thread CppSLCtx& kCppSLBuiltins");
        if (hasArgs)
            sb.append(L", ");
    }
}

void MSLGenerator::GenerateKernelWrapper(SourceBuilderNew& sb, const skr::CppSL::FunctionDecl* funcDecl)
{
    const StageAttr* stageAttr = FindAttr<StageAttr>(funcDecl->attrs());
    if (!stageAttr)
        return;
    
    const ShaderStage stage = stageAttr->stage();
    // Check if certain semantics must be standalone (can't be in structs)
    auto isStandaloneParmeter = [=](SemanticType semantic) -> bool 
    {
        if (stage == ShaderStage::Vertex)
            return semantic == SemanticType::VertexID;
        if (stage == ShaderStage::Fragment)
            return true;
        if (stage == ShaderStage::Compute)
        {
            // Compute shader thread/group attributes must be standalone
            return semantic == SemanticType::ThreadID ||
                   semantic == SemanticType::GroupID ||
                   semantic == SemanticType::ThreadPositionInGroup ||
                   semantic == SemanticType::ThreadIndexInGroup;
        }
        return false;
    };
    
    String inputStructName = funcDecl->name() + L"_in";
    String outputStructName = funcDecl->name() + L"_out";
    auto& ast = const_cast<AST&>(funcDecl->ast());
    auto inputType = ast.DeclareStructure(inputStructName, {});
    auto outputType = ast.DeclareStructure(outputStructName, {});
    std::vector<const ParamVarDecl*> wrapperParams;
    std::vector<Expr*> callArgs;
    std::vector<Stmt*> assemblyInputStmts;
    std::vector<Stmt*> assemblyOutputStmts;
    Stmt* callStmt = nullptr;
    auto inputParam = ast.DeclareParam(EVariableQualifier::None, inputType, L"in");
    auto outputVar = ast.Variable(EVariableQualifier::None, outputType, L"out");
    for (const auto& param : funcDecl->parameters())
    {        
        auto assemblyInputOutput = [&](auto elem, DeclStmt* prox, bool structure, uint32_t field_idx) 
        {
            EVariableQualifier qualifier = param->qualifier();
            if (auto semantic_attr = FindAttr<SemanticAttr>(elem->attrs()))
            {
                auto semantic = semantic_attr->semantic();
                SemanticAttr::GetSemanticQualifier(semantic, stageAttr->stage(), qualifier);
                if (bool isStandalone = isStandaloneParmeter(semantic))
                {
                    wrapperParams.emplace_back(param);
                    assemblyInputStmts.push_back(ast.Assign(prox->ref(), ast.Ref(param)));
                    return;
                }
            }
            auto f = ast.DeclareField(elem->name(), &elem->type());
            if (bool isInput = (qualifier == EVariableQualifier::None) || (qualifier == EVariableQualifier::Const) || (qualifier == EVariableQualifier::Inout))
            {
                inputType->add_field(f);
                assemblyInputStmts.push_back(ast.Assign(
                    structure ? (Expr*)ast.Field(prox->ref(), f) : (Expr*)prox->ref(),
                    ast.Field(ast.Ref(inputParam), f)
                ));
            }
            if (bool isOutput = (qualifier == EVariableQualifier::Out) || (qualifier == EVariableQualifier::Inout))
            {
                outputType->add_field(f);
                assemblyOutputStmts.push_back(ast.Assign(
                    ast.Field(ast.Ref(outputVar->decl()), f),
                    structure ? (Expr*)ast.Field(prox->ref(), f) : (Expr*)prox->ref()
                ));
            }
            auto attr = ast.DeclareAttr<MTLInputOutputAssemblyAttr>();
            if (auto stageInout = FindAttr<StageInoutAttr>(param->type().attrs()))
            {
                attr->is_attribute = (stage == ShaderStage::Vertex);
                attr->attrib_idx = field_idx;
                attr->is_user = !attr->is_attribute;
            }
            if (auto semantic = FindAttr<SemanticAttr>(elem->attrs()))
            {
                attr->semantic = semantic->semantic();
            }
            if (auto interpolation = FindAttr<InterpolationAttr>(elem->attrs()))
            {
                attr->interpolation = interpolation->mode();
            }
            f->add_attr(attr);
        };
        auto in = ast.Variable(EVariableQualifier::None, &param->type(), L"_" + param->name());
        assemblyInputStmts.emplace_back(in);
        callArgs.emplace_back(in->ref());
        if (auto structType = dynamic_cast<const StructureTypeDecl*>(&param->type()))
        {
            for (uint32_t i = 0; i < structType->fields().size(); i++)
            {
                assemblyInputOutput(structType->fields()[i], in, true, i);
            }
        }
        else
        {
            assemblyInputOutput(param, in, false, -1);
        }
    }
    if (!inputType->fields().empty())
    {
        wrapperParams.insert(wrapperParams.begin(), inputParam);
        inputParam->add_attr(ast.DeclareAttr<MTLStageInAttr>());
    }

    // GENERATE WRAPPERS FOR CTX & GROUP SHARED VALUES
    if (!ctx_type)
    {
        ctx_type = (skr::CppSL::StructureTypeDecl*)ast.DeclareStructure(L"CppSLCtx", {});
        auto ctx_var = ast.Variable(EVariableQualifier::None, ctx_type, L"kCppSLBuiltins");
        assemblyInputStmts.emplace_back(ctx_var);
        if (stage == ShaderStage::Compute)
        {
            {
                auto lane_id = ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"WaveLaneIndex");
                lane_id->add_attr(ast.DeclareAttr<MTLWaveValueAttr>(L"[[thread_index_in_simdgroup]]"));
                wrapperParams.emplace_back(lane_id);

                auto WaveLaneIndex = ast.DeclareField(L"WaveLaneIndex", ast.UIntType);
                ctx_type->add_field(WaveLaneIndex);
                assemblyInputStmts.emplace_back(ast.Assign(ast.Field(ctx_var->ref(), WaveLaneIndex), lane_id->ref()));
            }
            {
                auto lane_count = ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"WaveLaneCount");
                lane_count->add_attr(ast.DeclareAttr<MTLWaveValueAttr>(L"[[threads_per_simdgroup]]"));
                wrapperParams.emplace_back(lane_count);

                auto WaveLaneCount = ast.DeclareField(L"WaveLaneCount", ast.UIntType);
                ctx_type->add_field(WaveLaneCount);
                assemblyInputStmts.emplace_back(ast.Assign(ast.Field(ctx_var->ref(), WaveLaneCount), lane_count->ref()));
            }
        }
    }

    if (funcDecl->return_type() != ast.VoidType)
    {
        auto retVar = ast.Variable(
            EVariableQualifier::None, 
            funcDecl->return_type(), L"__zz_result",
            ast.CallFunction(funcDecl->ref(), callArgs)
        );
        auto assemblyOutput = [&](const TypeDecl* type, const String& name, bool structure)
        {
            auto f = ast.DeclareField(name, type);
            outputType->add_field(f);
            assemblyOutputStmts.push_back(ast.Assign(
                ast.Field(ast.Ref(outputVar->decl()), f),
                structure ? (Expr*)ast.Field(retVar->ref(), f) : (Expr*)retVar->ref()
            ));
            return f;
        };
        if (auto structType = dynamic_cast<const StructureTypeDecl*>(funcDecl->return_type()))
        {
            for (auto field : structType->fields())
            {
                auto prox = assemblyOutput(&field->type(), field->name(), true);
                prox->add_attrs(field->attrs());
            }
        }
        else
        {
            auto prox = assemblyOutput(funcDecl->return_type(), L"__zz_result", false);
            prox->add_attrs(funcDecl->return_type()->attrs());
        }
        callStmt = retVar;
    }
    else
    {
        callStmt = ast.CallFunction(funcDecl->ref(), callArgs);
    }
    bool hasOutput = !outputType->fields().empty() || (funcDecl->return_type() != ast.VoidType);
    if (hasOutput)
    {
        assemblyOutputStmts.insert(assemblyOutputStmts.begin(), outputVar);
        assemblyOutputStmts.emplace_back(ast.Return(ast.Ref(outputVar->decl())));
    }

    auto wrapperStmts = assemblyInputStmts;
    wrapperStmts.emplace_back(callStmt);
    wrapperStmts.insert(wrapperStmts.end(), assemblyOutputStmts.begin(), assemblyOutputStmts.end());
    auto wrapper = ast.DeclareFunction(funcDecl->name(), 
        hasOutput ? outputType : ast.VoidType,
        wrapperParams, 
        ast.Block(wrapperStmts)
    );
    wrapper->add_attr(ast.DeclareAttr<MTLKernelAttr>(stage));
    
    visit(sb, inputType);
    visit(sb, outputType);

    if (ctx_type->is_empty())
    {
        ctx_type->add_method(ast.DeclareMethod(ctx_type, L"placeholder", ast.VoidType, {}, ast.Block({})));
    }
    visit(sb, ctx_type);

    // Add SRT parameters as placeholders for debugger
    // std::set<uint32_t> used_sets;
    // for (const auto& [var, set] : set_of_vars)
    // {
    //     used_sets.insert(set);
    // }
    
    // for (uint32_t set : used_sets)
    // {
    //     if (!first) sb.append(L", ");
    //     first = false;
        
    //     auto SRTTypeName = L"SRT" + std::to_wstring(set);
    //     auto SRTVarName = L"srt" + std::to_wstring(set);
        
    //     sb.append(L"constant ");
    //     sb.append(SRTTypeName);
    //     sb.append(L"& ");
    //     sb.append(SRTVarName);
    //     sb.append(L" [[buffer(");
    //     sb.append(std::to_wstring(set));
    //     sb.append(L")]]");
    // }
}

void MSLGenerator::GenerateFunctionAttributes(SourceBuilderNew& sb, const FunctionDecl* funcDecl)
{
    if (auto Kernel = FindAttr<MTLKernelAttr>(funcDecl->attrs()))
    {
        sb.append(GetStageName(Kernel->stage));
        sb.append(L" ");
    }
}

bool MSLGenerator::SupportConstructor() const
{
    return true;
}

void MSLGenerator::AnalyzeFunctions(const AST& ast)
{
    for (const auto& funcDecl : ast.funcs())
    {
        if (const StageAttr* stageAttr = FindAttr<StageAttr>(funcDecl->attrs()))
        {
            AnalyzeFunction(funcDecl);
        }
    }
}

void MSLGenerator::AnalyzeFunction(const skr::CppSL::FunctionDecl* funcDecl)
{
    if (analyzed_functions.find(funcDecl) != analyzed_functions.end())
        return;
    
    analyzed_functions.insert(funcDecl);
    if (auto body = funcDecl->body())
    {
        WalkStmt(body, funcDecl);
    }
}

void MSLGenerator::WalkStmt(const skr::CppSL::Stmt* stmt, const skr::CppSL::FunctionDecl* currentFunc)
{
    if (auto callExpr = dynamic_cast<const skr::CppSL::CallExpr*>(stmt))
    {
        if (auto calleeDecl = dynamic_cast<const skr::CppSL::FunctionDecl*>(callExpr->callee()->decl()))
        {
            if (calleeDecl->ast().IsIntrinsic(calleeDecl, L"Wave") || 
                calleeDecl->ast().IsIntrinsic(calleeDecl, L"Quad"))
            {
                functions_with_wave_intrins.insert(currentFunc);
            }
            else
            {
                AnalyzeFunction(calleeDecl);
                if (functions_with_wave_intrins.find(calleeDecl) != functions_with_wave_intrins.end())
                {
                    functions_with_wave_intrins.insert(currentFunc);
                }
            }
        }
    }
    
    for (const auto& child : stmt->children())
    {
        WalkStmt(child, currentFunc);
    }
}

bool MSLGenerator::HasWaveIntrins(const skr::CppSL::FunctionDecl* funcDecl) const
{
    return functions_with_wave_intrins.find(funcDecl) != functions_with_wave_intrins.end();
}

const std::unordered_set<const skr::CppSL::FunctionDecl*>& MSLGenerator::GetFunctionsWithWaveIntrins() const
{
    return functions_with_wave_intrins;
}

} // end namespace skr::CppSL::MSL
