#include "SkrCore/log.hpp"
#include "SkrTestFramework/framework.hpp"
#include "SkrRTTR/type_signature.hpp"

TEST_CASE("test type signature")
{
    using namespace skr::rttr;
    TypeSignatureTyped<const int&> test;

    SKR_LOG_FMT_INFO(u8"raw: {}", test.view().to_string());

    test.view().normalize(
        false,
        false,
        true);

    SKR_LOG_FMT_INFO(u8"ignore const: {}", test.view().to_string());

    test.view().normalize();

    SKR_LOG_FMT_INFO(u8"ignore ref: {}", test.view().to_string());

    TypeSignatureTyped<void (*)(int&, int*&, const int&)> test_function_pointer;
    SKR_LOG_FMT_INFO(u8"function pointer: {}", test_function_pointer.view().to_string());
}