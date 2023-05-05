#pragma once
#include "SkrDAScript/env.hpp"

namespace skr {
namespace das {

struct TextPrinterDescriptor
{
    uint32_t _nothing_ = 0;
};

struct SKR_DASCRIPT_API TextPrinter
{
    static TextPrinter* Create(const TextPrinterDescriptor& desc) SKR_NOEXCEPT;
    static void Free(TextPrinter* printer) SKR_NOEXCEPT;

    virtual ~TextPrinter() SKR_NOEXCEPT;
};

} // namespace das
} // namespace skr