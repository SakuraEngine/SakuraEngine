#pragma once
#include "helpers.hpp"
#include "UsdCore/sdfpath.hpp"
#include "pxr/usd/sdf/path.h"

namespace skd
{
    struct SUSDSdfPathImpl : public USDWrapperWithRC<SUSDSdfPath>
    {
        friend struct SUSDPrimImpl;
        SUSDSdfPathImpl(pxr::SdfPath path);
        ~SUSDSdfPathImpl();

        virtual bool IsEmpty() const SKR_NOEXCEPT override;
        virtual bool IsAbsoluteRootOrPrimPath() const SKR_NOEXCEPT override;

        virtual eastl::string GetName() const SKR_NOEXCEPT override;
        virtual eastl::string GetElementString() const SKR_NOEXCEPT override;

        virtual SharedId GetAbsoluteRootOrPrimPath() const SKR_NOEXCEPT override;
        virtual SharedId ReplaceName(const char* newName) SKR_NOEXCEPT override;
        virtual SharedId GetParentPath() const SKR_NOEXCEPT override;
        virtual SharedId AppendChild(const char* childName) SKR_NOEXCEPT override;
        virtual SharedId StripAllVariantSelections() SKR_NOEXCEPT override;

        virtual eastl::string GetString() const SKR_NOEXCEPT override;
    protected:
        pxr::SdfPath path;
    };
}