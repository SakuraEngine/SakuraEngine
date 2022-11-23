#include "detail/attribute_impl.hpp"
#include "detail/sdfpath_impl.hpp"
#include "detail/prim_impl.hpp"

skd::SUSDAttributeImpl::SUSDAttributeImpl(pxr::UsdAttribute attribute)
    : attribute(attribute)
{

}

skd::SUSDAttributeImpl::~SUSDAttributeImpl()
{

}

bool skd::SUSDAttributeImpl::GetMetadata(const char* key, SUSDVtValueId outValue) const SKR_NOEXCEPT
{
    // TODO: TE impl?
    SKR_UNIMPLEMENTED_FUNCTION();
    return false;
}

bool skd::SUSDAttributeImpl::HasMetadata(const char* key) const SKR_NOEXCEPT
{
    return attribute.HasMetadata(pxr::TfToken(key));
}

bool skd::SUSDAttributeImpl::SetMetadata(const char* key, const SUSDVtValueId& value) SKR_NOEXCEPT
{
    // TODO: TE impl?
    SKR_UNIMPLEMENTED_FUNCTION();
    return false;
}

bool skd::SUSDAttributeImpl::ClearMetadata(const char* key) SKR_NOEXCEPT
{
    return attribute.ClearMetadata(pxr::TfToken(key));
}

skr::string skd::SUSDAttributeImpl::GetName() const SKR_NOEXCEPT
{
    return attribute.GetName().GetString().c_str();
}

skr::string skd::SUSDAttributeImpl::GetBaseName() const SKR_NOEXCEPT
{
    return attribute.GetBaseName().GetString().c_str();
}

skr::string skd::SUSDAttributeImpl::GetTypeName() const SKR_NOEXCEPT
{
    return attribute.GetTypeName().GetAsToken().GetString().c_str();
}

bool skd::SUSDAttributeImpl::GetTimeSamples(eastl::vector<double>& times) const SKR_NOEXCEPT
{
    std::vector<double> timesArray;
    if (auto result = attribute.GetTimeSamples(&timesArray);!result)
    {
        return false;
    }
    times.resize(timesArray.size());
    for (size_t i = 0; i < timesArray.size(); ++i)
    {
        times[i] = timesArray[i];
    }
    return true;
}

bool skd::SUSDAttributeImpl::HasValue() const SKR_NOEXCEPT
{
    return attribute.HasValue();
}

bool skd::SUSDAttributeImpl::HasFallbackValue() const SKR_NOEXCEPT
{
    return attribute.HasFallbackValue();
}

bool skd::SUSDAttributeImpl::ValueMightBeTimeVarying() const SKR_NOEXCEPT
{
    return attribute.ValueMightBeTimeVarying();
}

bool skd::SUSDAttributeImpl::Get(SUSDVtValueId outValue, eastl::optional<double> time) const SKR_NOEXCEPT
{
    // TODO: TE impl?
    SKR_UNIMPLEMENTED_FUNCTION();
    return false;
}

bool skd::SUSDAttributeImpl::Set(const SUSDVtValueId& value, eastl::optional<double> time) SKR_NOEXCEPT
{
    // TODO: TE impl?
    SKR_UNIMPLEMENTED_FUNCTION();
    return false;
}

bool skd::SUSDAttributeImpl::Clear() SKR_NOEXCEPT
{
    return attribute.Clear();
}

bool skd::SUSDAttributeImpl::ClearAtTime(double time) SKR_NOEXCEPT
{
    return attribute.ClearAtTime(time);
}

skr::SObjectPtr<skd::SUSDSdfPath> skd::SUSDAttributeImpl::GetPath() const SKR_NOEXCEPT
{
    return skr::SObjectPtr<skd::SUSDSdfPathImpl>::Create(attribute.GetPath());
}

skr::SObjectPtr<skd::SUSDPrim> skd::SUSDAttributeImpl::GetPrim() const SKR_NOEXCEPT
{
    return skr::SObjectPtr<skd::SUSDPrimImpl>::Create(attribute.GetPrim());
}