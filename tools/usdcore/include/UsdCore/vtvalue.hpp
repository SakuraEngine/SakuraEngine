#pragma once
#include "UsdCore/module.configure.h"
#include "platform/configure.h"
#include "containers/sptr.hpp"
#include "containers/string.hpp"

namespace skd
{
    // TODO: type erase 了草
    struct SUSDVtValue : public skr::SInterface
    {
        virtual ~SUSDVtValue() = default;
        
        virtual skr::string GetTypeName() const SKR_NOEXCEPT = 0;
        virtual bool IsArrayValued() const SKR_NOEXCEPT = 0;
        virtual bool IsEmpty() const SKR_NOEXCEPT = 0;
    protected:
        void* _this;
    };
    using SUSDVtValueId = skr::SObjectPtr<SUSDVtValue>;
}