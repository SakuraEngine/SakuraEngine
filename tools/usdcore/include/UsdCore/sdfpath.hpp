#pragma once
#include "UsdCore/module.configure.h"
#include "platform/configure.h"
#include "containers/sptr.hpp"
#include <containers/string.hpp>

namespace skd
{
    struct SUSDSdfPath : public skr::SInterface
    {
        using SharedId = skr::SObjectPtr<SUSDSdfPath>;
        virtual ~SUSDSdfPath() = default;

        static const SUSDSdfPath* AbsoluteRootPath() SKR_NOEXCEPT;

        virtual bool IsEmpty() const SKR_NOEXCEPT = 0;
        virtual bool IsAbsoluteRootOrPrimPath() const SKR_NOEXCEPT = 0;

        virtual skr::string GetName() const SKR_NOEXCEPT = 0;
        virtual skr::string GetElementString() const SKR_NOEXCEPT = 0;

        virtual SharedId GetAbsoluteRootOrPrimPath() const SKR_NOEXCEPT = 0;
        virtual SharedId ReplaceName(const char* newName) SKR_NOEXCEPT = 0;
        virtual SharedId GetParentPath() const SKR_NOEXCEPT = 0;
        virtual SharedId AppendChild(const char* childName) SKR_NOEXCEPT = 0;
        virtual SharedId StripAllVariantSelections() SKR_NOEXCEPT = 0;

        virtual skr::string GetString() const SKR_NOEXCEPT = 0;
    };
    using SUSDSdfPathId = skr::SObjectPtr<SUSDSdfPath>;
}