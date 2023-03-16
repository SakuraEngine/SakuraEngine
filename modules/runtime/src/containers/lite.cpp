#define CONTAINER_LITE_IMPL
#include "containers/lite.hpp"

namespace skr {
namespace lite {

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