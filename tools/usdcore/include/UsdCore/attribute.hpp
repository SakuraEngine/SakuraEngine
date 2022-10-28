#pragma once
#include "EASTL/optional.h"
#include "vtvalue.hpp"
#include <EASTL/vector.h>

namespace skd
{
    struct SUSDPrim;
    struct SUSDSdfPath;

    struct SUSDAttribute : public skr::SInterface
    {
        using SharedId = skr::SObjectPtr<SUSDAttribute>;
        virtual ~SUSDAttribute() = default;

        static bool GetUnionedTimeSamples(const eastl::vector<SharedId>& attributes, eastl::vector<double>& outTimes) SKR_NOEXCEPT;

        virtual bool GetMetadata(const char* key, SUSDVtValueId outValue) const SKR_NOEXCEPT = 0;
        virtual bool HasMetadata(const char* key) const SKR_NOEXCEPT = 0;
        virtual bool SetMetadata(const char* key, const SUSDVtValueId& value) SKR_NOEXCEPT = 0;
        virtual bool ClearMetadata(const char* key) SKR_NOEXCEPT = 0;

        virtual eastl::string GetName() const SKR_NOEXCEPT = 0;
        virtual eastl::string GetBaseName() const SKR_NOEXCEPT = 0;
        virtual eastl::string GetTypeName() const SKR_NOEXCEPT = 0;

        virtual bool GetTimeSamples(eastl::vector<double>& times) const SKR_NOEXCEPT = 0;

        virtual bool HasValue() const SKR_NOEXCEPT = 0;
        virtual bool HasFallbackValue() const SKR_NOEXCEPT = 0;

        virtual bool ValueMightBeTimeVarying() const SKR_NOEXCEPT = 0;
        
        virtual bool Get(SUSDVtValueId outValue, eastl::optional<double> time = {}) const SKR_NOEXCEPT = 0;
        virtual bool Set(const SUSDVtValueId& value, eastl::optional<double> time = {}) SKR_NOEXCEPT = 0;

        virtual bool Clear() SKR_NOEXCEPT = 0;
        virtual bool ClearAtTime(double time) SKR_NOEXCEPT = 0;
        
        virtual skr::SObjectPtr<SUSDSdfPath> GetPath() const SKR_NOEXCEPT = 0;
        virtual skr::SObjectPtr<SUSDPrim> GetPrim() const SKR_NOEXCEPT = 0;
    };

    using SUSDAttributeId = SUSDAttribute::SharedId;
}