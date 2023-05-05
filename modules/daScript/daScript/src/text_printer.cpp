#include "types.hpp"

namespace skr {
namespace das {

TextPrinter::~TextPrinter() SKR_NOEXCEPT
{
    
}

TextPrinter* TextPrinter::Create(const TextPrinterDescriptor& desc) SKR_NOEXCEPT
{
    return SkrNew<TextPrinterImpl>();
}

void TextPrinter::Free(TextPrinter* printer) SKR_NOEXCEPT
{
    SkrDelete(printer);
}

} // namespace das
} // namespace skr