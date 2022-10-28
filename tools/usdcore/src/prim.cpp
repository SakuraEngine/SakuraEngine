#include "platform/debug.h"
#include <EASTL/vector.h>
#include "containers/detail/sptr.hpp"
#include "UsdCore/attribute.hpp"
#include "UsdCore/sdfpath.hpp"

#include "detail/attribute_impl.hpp"
#include "detail/stage_impl.hpp"
#include "detail/prim_impl.hpp"
#include "detail/sdfpath_impl.hpp"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"
#include "pxr/usd/sdf/schema.h"
#include "pxr/usd/usd/primFlags.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "utils/defer.hpp"

namespace skd
{
inline static pxr::GfMatrix4d Util_GetLocalToWorldTransform(const pxr::UsdPrim& Prim, double Time, const pxr::SdfPath& AbsoluteRootPath)
{
    pxr::SdfPath PrimPath = Prim.GetPath();
    if (!Prim || PrimPath == AbsoluteRootPath)
    {
        return pxr::GfMatrix4d(1);
    }
    pxr::GfMatrix4d AccumulatedTransform(1.);
    bool bResetsXFormStack = false;
    pxr::UsdGeomXformable XFormable(Prim);
    // silently ignoring errors
    XFormable.GetLocalTransformation(&AccumulatedTransform, &bResetsXFormStack, Time);
    if (!bResetsXFormStack)
    {
        AccumulatedTransform = AccumulatedTransform * Util_GetLocalToWorldTransform(Prim.GetParent(), Time, AbsoluteRootPath);
    }
    return AccumulatedTransform;
}

SUSDPrimImpl::SUSDPrimImpl(pxr::UsdPrim prim) 
    : prim(prim) 
{

}

SUSDPrimImpl::~SUSDPrimImpl()
{

}

bool SUSDPrimImpl::IsActive() const SKR_NOEXCEPT
{
    return prim.IsActive();
}

bool SUSDPrimImpl::SetActive(bool active) SKR_NOEXCEPT
{
    return prim.SetActive(active);
}

bool SUSDPrimImpl::IsValid() const SKR_NOEXCEPT
{
    return prim.IsValid();
}

bool SUSDPrimImpl::IsPseudoRoot() const SKR_NOEXCEPT
{
    return prim.IsPseudoRoot();
}

bool SUSDPrimImpl::IsModel() const SKR_NOEXCEPT
{
    return prim.IsModel();
}

bool SUSDPrimImpl::IsGroup() const SKR_NOEXCEPT
{
    return prim.IsGroup();
}

eastl::vector<eastl::string> SUSDPrimImpl::GetAppliedSchemas() const SKR_NOEXCEPT
{
    eastl::vector<eastl::string> result;
    std::vector<pxr::TfToken> appliedSchemas = prim.GetAppliedSchemas();
    result.reserve(appliedSchemas.size());
    for (const auto& schema : appliedSchemas)
    {
        result.push_back(schema.GetString().c_str());
    }
    return result;
}

bool SUSDPrimImpl::IsA(const char* schemaType) const SKR_NOEXCEPT
{
    auto typeToken = pxr::TfToken(schemaType);
    pxr::TfType type = pxr::UsdSchemaRegistry::GetTypeFromName(typeToken);
    if (type.IsUnknown()) return false;
    return prim.IsA(type);
}

bool SUSDPrimImpl::HasAPI(const char* schemaType, eastl::optional<const char*> instanceName) const SKR_NOEXCEPT
{
    auto typeToken = pxr::TfToken(schemaType);
    pxr::TfType type = pxr::UsdSchemaRegistry::GetTypeFromName(typeToken);
    if (type.IsUnknown()) return false;
    auto usdInstanceName = instanceName.has_value() ? pxr::TfToken(instanceName.value()) : pxr::TfToken();
    return prim.HasAPI(type, usdInstanceName); 
}

SUSDSdfPathId SUSDPrimImpl::GetPrimPath() const SKR_NOEXCEPT
{
    return skr::SObjectPtr<SUSDSdfPathImpl>::Create(prim.GetPrimPath());
}

skr::SObjectPtr<SUSDStage> SUSDPrimImpl::GetStage() const SKR_NOEXCEPT
{
    return skr::SObjectPtr<SUSDStageImpl>::Create(prim.GetStage());
}

bool SUSDPrimImpl::GetLocalTransformation(skr::span<double, 16> result, bool* resetsXformStack) const SKR_NOEXCEPT
{
    pxr::GfMatrix4d USDMatrix(1);
    pxr::UsdGeomXformable XForm(prim);
    if (XForm)
    {
        // Set transform
        bool bResetXFormStack = false;
        XForm.GetLocalTransformation(&USDMatrix, &bResetXFormStack);
    }
    ::memcpy(result.data(), &USDMatrix, sizeof(double) * 16);
    return bool(XForm);
}

void SUSDPrimImpl::GetLocalToWorldTransformation(skr::span<double, 16> result) const SKR_NOEXCEPT
{
    pxr::SdfPath AbsoluteRootPath = pxr::SdfPath::AbsoluteRootPath();
    auto Time = pxr::UsdTimeCode::Default().GetValue();
    auto USDMatrix = Util_GetLocalToWorldTransform(prim, Time, AbsoluteRootPath);
    ::memcpy(result.data(), &USDMatrix, sizeof(double) * 16);
}

void SUSDPrimImpl::GetLocalToWorldTransformation(skr::span<double, 16> result, double time) const SKR_NOEXCEPT
{
    pxr::SdfPath AbsoluteRootPath = pxr::SdfPath::AbsoluteRootPath();
    auto USDMatrix = Util_GetLocalToWorldTransform(prim, time, AbsoluteRootPath);
    ::memcpy(result.data(), &USDMatrix, sizeof(double) * 16);
}

void SUSDPrimImpl::GetLocalToWorldTransformation(skr::span<double, 16> result, double time, SUSDSdfPathId sdfPath) const SKR_NOEXCEPT
{
    auto sdfPathImpl = skr::static_pointer_cast<SUSDSdfPathImpl>(sdfPath);
    auto USDMatrix = Util_GetLocalToWorldTransform(prim, time, sdfPathImpl->path);
    ::memcpy(result.data(), &USDMatrix, sizeof(double) * 16);
}

SUSDPrimId SUSDPrimImpl::GetParent() const SKR_NOEXCEPT
{
    return skr::SObjectPtr<SUSDPrimImpl>::Create(prim.GetParent());
}

eastl::vector<SUSDPrim::SharedId> SUSDPrimImpl::GetChildren() const SKR_NOEXCEPT
{
    pxr::UsdPrimSiblingRange primChildren = prim.GetChildren();
    eastl::vector<SharedId> children;
    for (const pxr::UsdPrim& child : primChildren)
    {
        children.emplace_back(skr::SObjectPtr<SUSDPrimImpl>::Create(child));
    }
    return children;
}

eastl::vector<SUSDPrim::SharedId> SUSDPrimImpl::GetFilteredChildren(bool traverseInstanceProxies) const SKR_NOEXCEPT
{
    pxr::Usd_PrimFlagsPredicate predicate = pxr::UsdPrimDefaultPredicate;
    if (traverseInstanceProxies)
    {
        predicate = pxr::UsdTraverseInstanceProxies(predicate);
    } 
    pxr::UsdPrimSiblingRange primChildren = prim.GetFilteredChildren(predicate);
    eastl::vector<SharedId> children;
    for (const pxr::UsdPrim& child : primChildren)
    {
        children.emplace_back(skr::SObjectPtr<SUSDPrimImpl>::Create(child));
    }
    return children;
}

bool SUSDPrimImpl::HasPayload() const SKR_NOEXCEPT
{
    return prim.HasPayload();
}

bool SUSDPrimImpl::IsLoaded() const SKR_NOEXCEPT
{
    return prim.IsLoaded();
}

void SUSDPrimImpl::Load() SKR_NOEXCEPT
{
    prim.Load();
}

void SUSDPrimImpl::Unload() SKR_NOEXCEPT
{
    prim.Unload();
}

bool SUSDPrimImpl::RemoveProperty(const char* name) SKR_NOEXCEPT
{
    return prim.RemoveProperty(pxr::TfToken(name));
}

SUSDAttributeId SUSDPrimImpl::CreateAttribute(const char* name, const char* typeName) const SKR_NOEXCEPT
{
    auto attribtue = prim.CreateAttribute(
        pxr::TfToken(name),
        pxr::SdfSchema::GetInstance().FindType(pxr::TfToken(typeName))
    );
    return skr::SObjectPtr<SUSDAttributeImpl>::Create(attribtue);
}

eastl::vector<SUSDAttributeId> SUSDPrimImpl::GetAttributes() const SKR_NOEXCEPT
{
    eastl::vector<SUSDAttributeId> attributes = {};
    auto usdAttributes = prim.GetAttributes();
    attributes.reserve(usdAttributes.size());
    for (const auto& usdAttribute : usdAttributes)
    {
        attributes.emplace_back(skr::SObjectPtr<SUSDAttributeImpl>::Create(usdAttribute));
        (void)usdAttribute; SKR_UNIMPLEMENTED_FUNCTION();
    }
    return attributes;
}

SUSDAttributeId SUSDPrimImpl::GetAttribute(const char *name) const SKR_NOEXCEPT
{
    return skr::SObjectPtr<SUSDAttributeImpl>::Create(prim.GetAttribute(pxr::TfToken(name)));
}

bool SUSDPrimImpl::HasAttribute(const char *name) const SKR_NOEXCEPT
{
    return prim.HasAttribute(pxr::TfToken(name));
}

eastl::string SUSDPrimImpl::GetName() const SKR_NOEXCEPT 
{
    return prim.GetName().GetString().c_str();
}

eastl::string SUSDPrimImpl::GetTypeName() const SKR_NOEXCEPT 
{
    return prim.GetTypeName().GetString().c_str();
}
}