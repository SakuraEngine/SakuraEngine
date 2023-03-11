#include "containers.hpp"

namespace skr {
namespace gui {

TextStorage::TextStorage(const char* str) SKR_NOEXCEPT
{
    ctor(str);
}

TextStorage::TextStorage() SKR_NOEXCEPT
{
    ctor();
}

TextStorage::~TextStorage() SKR_NOEXCEPT
{
    dtor();
}

} }