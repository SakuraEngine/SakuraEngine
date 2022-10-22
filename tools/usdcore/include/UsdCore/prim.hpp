#pragma once
#include "UsdCore/module.configure.h"
#include "platform/configure.h"
#include "containers/sptr.hpp"
#include "containers/span.hpp"
#include <EASTL/vector.h>
#include <EASTL/string.h>

namespace skd
{
    struct SUSDPrim : public skr::SInterface
    {
        using SharedId = skr::SObjectPtr<SUSDPrim>;
        virtual ~SUSDPrim() = default; 

        // returns false when the prim is not an xformable
        virtual bool GetLocalTransformation(skr::span<double, 16> result, bool* resetsXformStack = nullptr) const = 0;
        
        // returns false when the prim is not an xformable
        virtual void GetLocalToWorldTransformation(skr::span<double, 16> result) const = 0;
        virtual void GetLocalToWorldTransformation(skr::span<double, 16> result, double time) const = 0;
        // virtual bool GetLocalToWorldTransformation(skr::span<double, 16> result, double time, SUSDSdfPathId sdfPath) const = 0;

        virtual eastl::vector<SharedId> GetChildren() const SKR_NOEXCEPT = 0;

        virtual eastl::string GetName() const SKR_NOEXCEPT = 0;
    };
    using SUSDPrimId = SUSDPrim::SharedId;

}