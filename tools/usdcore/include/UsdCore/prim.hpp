#pragma once
#include "UsdCore/attribute.hpp"
#include "UsdCore/sdfpath.hpp"
#include "containers/span.hpp"
#include "utils/function_ref.hpp"
#include <EASTL/optional.h>

namespace skd
{
    struct SUSDStage;
    struct SUSDPrim : public skr::SInterface
    {
        using SharedId = skr::SObjectPtr<SUSDPrim>;
        virtual ~SUSDPrim() = default; 

        // returns false when the prim is not an xformable
        virtual bool GetLocalTransformation(skr::span<double, 16> result, bool* resetsXformStack = nullptr) const = 0;
        
        virtual bool IsActive() const SKR_NOEXCEPT = 0;
        virtual bool SetActive(bool active) SKR_NOEXCEPT = 0;
        virtual bool IsValid() const SKR_NOEXCEPT = 0;
        virtual bool IsPseudoRoot() const SKR_NOEXCEPT = 0;
        virtual bool IsModel() const SKR_NOEXCEPT = 0;
        virtual bool IsGroup() const SKR_NOEXCEPT = 0;

        virtual eastl::vector<skr::string> GetAppliedSchemas() const = 0;

        virtual bool IsA(const char* schemaType) const SKR_NOEXCEPT = 0;
        virtual bool HasAPI(const char* schemaType, eastl::optional<const char*> instanceName = {}) const SKR_NOEXCEPT = 0;

        virtual SUSDSdfPathId GetPrimPath() const SKR_NOEXCEPT = 0;
        virtual skr::SObjectPtr<SUSDStage> GetStage() const SKR_NOEXCEPT = 0;

        // returns false when the prim is not an xformable
        virtual void GetLocalToWorldTransformation(skr::span<double, 16> result) const SKR_NOEXCEPT = 0;
        virtual void GetLocalToWorldTransformation(skr::span<double, 16> result, double time) const SKR_NOEXCEPT = 0;
        virtual void GetLocalToWorldTransformation(skr::span<double, 16> result, double time, SUSDSdfPathId sdfPath) const = 0;

        virtual SharedId GetParent() const SKR_NOEXCEPT = 0;

        virtual eastl::vector<SharedId> GetChildren() const SKR_NOEXCEPT = 0;
        virtual eastl::vector<SharedId> GetFilteredChildren(bool traverseInstanceProxies) const SKR_NOEXCEPT = 0;
        virtual eastl::vector<SharedId> GetAllPrimsOfType(const char* schemaType, skr::function_ref<bool(SharedId)> pruneChildren) const SKR_NOEXCEPT = 0;

        virtual bool HasPayload() const SKR_NOEXCEPT = 0;
        virtual bool IsLoaded() const SKR_NOEXCEPT = 0;
        virtual void Load() SKR_NOEXCEPT = 0;
        virtual void Unload() SKR_NOEXCEPT = 0;

        virtual bool RemoveProperty(const char* name) SKR_NOEXCEPT = 0;

        virtual SUSDAttributeId CreateAttribute(const char* name, const char* typeName) const SKR_NOEXCEPT = 0;
        virtual eastl::vector<SUSDAttributeId> GetAttributes() const SKR_NOEXCEPT = 0;
        virtual SUSDAttributeId GetAttribute(const char* name) const SKR_NOEXCEPT = 0;
        virtual bool HasAttribute(const char* name) const SKR_NOEXCEPT = 0;

        virtual skr::string GetName() const SKR_NOEXCEPT = 0;
        virtual skr::string GetTypeName() const SKR_NOEXCEPT = 0;
        virtual skr::string GetKind() const SKR_NOEXCEPT = 0;
    };
    using SUSDPrimId = SUSDPrim::SharedId;

}