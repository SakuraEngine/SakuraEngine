#pragma once
#include "EASTL/vector.h"
#include "helpers.hpp"
#include "UsdCore/prim.hpp"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usdGeom/xform.h"

namespace skd
{
struct SUSDPrimImpl : public USDWrapperWithRC<SUSDPrim>
{
    SUSDPrimImpl(pxr::UsdPrim prim);
    ~SUSDPrimImpl();

    virtual bool GetLocalTransformation(skr::span<double, 16> result, bool* resetsXformStack = nullptr) const SKR_NOEXCEPT override;
    
    virtual bool IsActive() const SKR_NOEXCEPT override;
    virtual bool SetActive(bool active) SKR_NOEXCEPT override;
    virtual bool IsValid() const SKR_NOEXCEPT override;
    virtual bool IsPseudoRoot() const SKR_NOEXCEPT override;
    virtual bool IsModel() const SKR_NOEXCEPT override;
    virtual bool IsGroup() const SKR_NOEXCEPT override;

    virtual eastl::vector<skr::string> GetAppliedSchemas() const SKR_NOEXCEPT override;

    virtual bool IsA(const char* schemaType) const SKR_NOEXCEPT override;
    virtual bool HasAPI(const char* schemaType, eastl::optional<const char*> instanceName = {}) const SKR_NOEXCEPT override;

    virtual SUSDSdfPathId GetPrimPath() const SKR_NOEXCEPT override;
    virtual skr::SObjectPtr<SUSDStage> GetStage() const SKR_NOEXCEPT override;

    // returns false when the prim is not an xformable
    virtual void GetLocalToWorldTransformation(skr::span<double, 16> result) const SKR_NOEXCEPT override;
    virtual void GetLocalToWorldTransformation(skr::span<double, 16> result, double time) const SKR_NOEXCEPT override;
    virtual void GetLocalToWorldTransformation(skr::span<double, 16> result, double time, SUSDSdfPathId sdfPath) const SKR_NOEXCEPT override;

    virtual SharedId GetParent() const SKR_NOEXCEPT override;

    virtual eastl::vector<SharedId> GetChildren() const SKR_NOEXCEPT override;
    virtual eastl::vector<SharedId> GetFilteredChildren(bool traverseInstanceProxies) const SKR_NOEXCEPT override;
    virtual eastl::vector<SharedId> GetAllPrimsOfType(const char* schemaType, skr::function_ref<bool(SharedId)> pruneChildren) const SKR_NOEXCEPT override;

    virtual bool HasPayload() const SKR_NOEXCEPT override;
    virtual bool IsLoaded() const SKR_NOEXCEPT override;
    virtual void Load() SKR_NOEXCEPT override;
    virtual void Unload() SKR_NOEXCEPT override;

    virtual bool RemoveProperty(const char* name) SKR_NOEXCEPT override;

    virtual SUSDAttributeId CreateAttribute(const char* name, const char* typeName) const SKR_NOEXCEPT override;
    virtual eastl::vector<SUSDAttributeId> GetAttributes() const SKR_NOEXCEPT override;
    virtual SUSDAttributeId GetAttribute(const char* name) const SKR_NOEXCEPT override;
    virtual bool HasAttribute(const char* name) const SKR_NOEXCEPT override;

    virtual skr::string GetName() const SKR_NOEXCEPT override;
    virtual skr::string GetTypeName() const SKR_NOEXCEPT override;
    virtual skr::string GetKind() const SKR_NOEXCEPT override;
    
    pxr::UsdPrim prim;
};
}