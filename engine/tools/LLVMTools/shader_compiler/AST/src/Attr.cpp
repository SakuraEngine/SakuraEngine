#include "CppSL/Attr.hpp"

namespace skr::CppSL {

AlignAttr::AlignAttr(uint32_t alignment) 
    : _alignment(alignment) 
{

}

SemanticAttr::SemanticAttr(SemanticType semantic) 
    : _semantic(semantic) 
{

}

bool SemanticAttr::GetSemanticQualifier(SemanticType semantic, ShaderStage stage, EVariableQualifier& out)
{
    switch (stage)
    {
        case ShaderStage::Vertex:
        {
            switch (semantic)
            {
                case SemanticType::Position: out = EVariableQualifier::Out; return true;
                case SemanticType::VertexID: out = EVariableQualifier::None; return true;
                case SemanticType::InstanceID: out = EVariableQualifier::None; return true;
                case SemanticType::ViewID: out = EVariableQualifier::Inout; return true;
                case SemanticType::CullDistance: out = EVariableQualifier::Out; return true;
                case SemanticType::ClipDistance: out = EVariableQualifier::Out; return true;
                default: break;
            }
            break;
        }
        case ShaderStage::Hull:
        {
            switch (semantic)
            {
                case SemanticType::PrimitiveID: out = EVariableQualifier::Inout; return true;
                case SemanticType::ControlPointID: out = EVariableQualifier::None; return true;
                case SemanticType::TessFactor: out = EVariableQualifier::Out; return true;
                case SemanticType::InsideTessFactor: out = EVariableQualifier::Out; return true;
                case SemanticType::ViewID: out = EVariableQualifier::Inout; return true;
                default: break;
            }
        }
        case ShaderStage::Domain:
        {
            switch (semantic)
            {
                case SemanticType::Position: out = EVariableQualifier::Out; return true;
                case SemanticType::TessFactor: out = EVariableQualifier::None; return true;
                case SemanticType::InsideTessFactor: out = EVariableQualifier::None; return true;
                case SemanticType::DomainLocation: out = EVariableQualifier::None; return true;
                case SemanticType::ViewID: out = EVariableQualifier::Inout; return true;
                case SemanticType::CullDistance: out = EVariableQualifier::Out; return true;
                case SemanticType::ClipDistance: out = EVariableQualifier::Out; return true;
                case SemanticType::PrimitiveID: out = EVariableQualifier::None; return true;
                default: break;
            }
        }
        case ShaderStage::Geometry:
        {
            switch (semantic)
            {
                case SemanticType::Position: out = EVariableQualifier::Inout; return true;
                case SemanticType::PrimitiveID: out = EVariableQualifier::Inout; return true;
                case SemanticType::GSInstanceID: out = EVariableQualifier::None; return true;
                case SemanticType::ViewID: out = EVariableQualifier::Inout; return true;
                case SemanticType::CullDistance: out = EVariableQualifier::Inout; return true;
                case SemanticType::ClipDistance: out = EVariableQualifier::Inout; return true;
                default: break;
            }
        }
        case ShaderStage::Fragment:
        {
            switch (semantic)
            {
                case SemanticType::Position: out = EVariableQualifier::None; return true;
                case SemanticType::RenderTarget0:
                case SemanticType::RenderTarget1:
                case SemanticType::RenderTarget2:
                case SemanticType::RenderTarget3:
                case SemanticType::RenderTarget4:
                case SemanticType::RenderTarget5:
                case SemanticType::RenderTarget6:
                case SemanticType::RenderTarget7:
                case SemanticType::Depth:
                case SemanticType::DepthGreaterEqual:
                case SemanticType::DepthLessEqual:
                case SemanticType::StencilRef:
                    out = EVariableQualifier::Out; return true;
                case SemanticType::IsFrontFace:
                case SemanticType::SampleIndex:
                case SemanticType::PrimitiveID:
                case SemanticType::Barycentrics:
                case SemanticType::ViewID:
                    out = EVariableQualifier::None; return true;
                case SemanticType::SampleMask: 
                    out = EVariableQualifier::Inout; return true;
                default: break;
            }
        }
        case ShaderStage::Compute:
        {
            switch (semantic)
            {
                case SemanticType::ThreadID: 
                case SemanticType::GroupID:
                case SemanticType::ThreadPositionInGroup: 
                case SemanticType::ThreadIndexInGroup: 
                    out = EVariableQualifier::None; 
                    return true;
                default: break;
            }
        }
        case ShaderStage::ClosestHit:
        case ShaderStage::Miss:
        case ShaderStage::AnyHit:
        {
            switch (semantic)
            {
                case SemanticType::RayPayload: 
                    out = EVariableQualifier::Inout; 
                    return true;
                default: break;
            }
        }
        default:
            break;
    }
    return false;
}

InterpolationAttr::InterpolationAttr(InterpolationMode mode)
    : _mode(mode) 
{

}

BuiltinAttr::BuiltinAttr(const String& name) 
    : _name(name) 
{

}

KernelSizeAttr::KernelSizeAttr(uint32_t x, uint32_t y, uint32_t z)
    : _x(x), _y(y), _z(z) 
{

}

ResourceBindAttr::ResourceBindAttr()
    : _group(~0), _binding(~0) 
{

}

ResourceBindAttr::ResourceBindAttr(uint32_t group, uint32_t binding)
    : _group(group), _binding(binding) 
{

}

PushConstantAttr::PushConstantAttr()
{

}

NormFloatAccessAttr::NormFloatAccessAttr(bool is_signed)
    : _is_signed(is_signed)
{

}

GloballyCoherentAttr::GloballyCoherentAttr()
{

}

StageInoutAttr::StageInoutAttr()
{

}

StageAttr::StageAttr(ShaderStage stage) 
    : _stage(stage)
{

}

BranchAttr::BranchAttr()
{
    
}

FlattenAttr::FlattenAttr()
{

}

LoopAttr::LoopAttr()
{

}

UnrollAttr::UnrollAttr(uint32_t N)
    : _N(N)
{

}

} // namespace skr::CppSL