#pragma once
#include "helpers.hpp"
#include "UsdCore/attribute.hpp"
#include "pxr/usd/usd/attribute.h"

namespace skd
{
struct SUSDAttributeImpl : public USDWrapperWithRC<SUSDAttribute>
{
    SUSDAttributeImpl(pxr::UsdAttribute attribute);
    ~SUSDAttributeImpl();

    virtual bool GetMetadata(const char* key, SUSDVtValueId outValue) const SKR_NOEXCEPT override;
    virtual bool HasMetadata(const char* key) const SKR_NOEXCEPT override;
    virtual bool SetMetadata(const char* key, const SUSDVtValueId& value) SKR_NOEXCEPT override;
    virtual bool ClearMetadata(const char* key) SKR_NOEXCEPT override;

    virtual skr::string GetName() const SKR_NOEXCEPT override;
    virtual skr::string GetBaseName() const SKR_NOEXCEPT override;
    virtual skr::string GetTypeName() const SKR_NOEXCEPT override;

    virtual bool GetTimeSamples(eastl::vector<double>& times) const SKR_NOEXCEPT override;

    virtual bool HasValue() const SKR_NOEXCEPT override;
    virtual bool HasFallbackValue() const SKR_NOEXCEPT override;

    virtual bool ValueMightBeTimeVarying() const SKR_NOEXCEPT override;
    
    virtual bool Get(SUSDVtValueId outValue, eastl::optional<double> time = {}) const SKR_NOEXCEPT override;
    virtual bool Set(const SUSDVtValueId& value, eastl::optional<double> time = {}) SKR_NOEXCEPT override;

    virtual bool Clear() SKR_NOEXCEPT override;
    virtual bool ClearAtTime(double time) SKR_NOEXCEPT override;
    
    virtual skr::SObjectPtr<SUSDSdfPath> GetPath() const SKR_NOEXCEPT override;
    virtual skr::SObjectPtr<SUSDPrim> GetPrim() const SKR_NOEXCEPT override;

protected:
    pxr::UsdAttribute attribute;
};
}