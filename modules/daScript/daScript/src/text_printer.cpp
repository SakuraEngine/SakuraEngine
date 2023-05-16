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

void TextPrinterImpl::print(const char8_t* text) SKR_NOEXCEPT
{
    printer << (const char*)text;
}

} // namespace das
} // namespace skr