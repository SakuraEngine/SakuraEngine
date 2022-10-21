#pragma once
#include "EASTL/vector.h"
#include "helpers.hpp"
#include "UsdCore/prim.hpp"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usdGeom/xform.h"
#include "utils/defer.hpp"
#include <vcruntime_string.h>

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

struct SUSDPrimImpl : public USDWrapperWithRC<SUSDPrim>
{
    SUSDPrimImpl(pxr::UsdPrim prim);
    ~SUSDPrimImpl();

    virtual bool GetLocalTransformation(skr::span<double, 16> result, bool* resetsXformStack = nullptr) const override
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
        
    // returns false when the prim is not an xformable
    virtual void GetLocalToWorldTransformation(skr::span<double, 16> result) const override
    {
        pxr::SdfPath AbsoluteRootPath = pxr::SdfPath::AbsoluteRootPath();
        auto Time = pxr::UsdTimeCode::Default().GetValue();
	    auto USDMatrix = Util_GetLocalToWorldTransform(prim, Time, AbsoluteRootPath);
        ::memcpy(result.data(), &USDMatrix, sizeof(double) * 16);
    }

    virtual void GetLocalToWorldTransformation(skr::span<double, 16> result, double time) const override
    {
        pxr::SdfPath AbsoluteRootPath = pxr::SdfPath::AbsoluteRootPath();
	    auto USDMatrix = Util_GetLocalToWorldTransform(prim, time, AbsoluteRootPath);
        ::memcpy(result.data(), &USDMatrix, sizeof(double) * 16);
    }

    virtual eastl::vector<SharedId> GetChildren() const SKR_NOEXCEPT override
    {
        pxr::UsdPrimSiblingRange primChildren = prim.GetChildren();
        eastl::vector<SharedId> children;
		for ( const pxr::UsdPrim& child : primChildren )
		{
			children.emplace_back( skr::SPtr<SUSDPrimImpl>::Create(child) );
		}
        return children;
    }

    virtual eastl::string GetName() const SKR_NOEXCEPT override
    {
        return prim.GetName().GetString().c_str();
    }

protected:
    pxr::UsdPrim prim;
};
}