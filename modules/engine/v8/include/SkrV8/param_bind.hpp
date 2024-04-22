#pragma once
#include <SkrBase/containers/misc/placeholder.hpp>

namespace skr::v8_bind
{
struct ParamBindConfig {
    using SetupDefaultFunc = void (*)(void* ptr);

    bool             is_out        = false;
    SetupDefaultFunc setup_default = nullptr;
};

template <typename T>
struct ParamBind {
    static constexpr bool is_const      = std::is_const_v<T>;
    static constexpr bool is_ref        = std::is_reference_v<T>;
    static constexpr bool is_rvalue_ref = std::is_rvalue_reference_v<T>;
    static constexpr bool is_pointer    = std::is_pointer_v<T>;
    using DecayType                     = std::decay_t<T>;

    // pre_invoke(invoke_info)
    // make_param(invoke_info)
    // post_invoke(invoke_info)

private:
    container::Placeholder<DecayType> _placeholder;
};

} // namespace skr::v8_bind