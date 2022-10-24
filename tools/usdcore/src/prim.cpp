#include "detail/prim_impl.hpp"
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

bool SUSDPrimImpl::GetLocalTransformation(skr::span<double, 16> result, bool* resetsXformStack) const
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

void SUSDPrimImpl::GetLocalToWorldTransformation(skr::span<double, 16> result) const
{
    pxr::SdfPath AbsoluteRootPath = pxr::SdfPath::AbsoluteRootPath();
    auto Time = pxr::UsdTimeCode::Default().GetValue();
    auto USDMatrix = Util_GetLocalToWorldTransform(prim, Time, AbsoluteRootPath);
    ::memcpy(result.data(), &USDMatrix, sizeof(double) * 16);
}

void SUSDPrimImpl::GetLocalToWorldTransformation(skr::span<double, 16> result, double time) const
{
    pxr::SdfPath AbsoluteRootPath = pxr::SdfPath::AbsoluteRootPath();
    auto USDMatrix = Util_GetLocalToWorldTransform(prim, time, AbsoluteRootPath);
    ::memcpy(result.data(), &USDMatrix, sizeof(double) * 16);
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

eastl::string SUSDPrimImpl::GetName() const SKR_NOEXCEPT 
{
    return prim.GetName().GetString().c_str();
}
}