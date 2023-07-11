#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct Slot {
    uint64_t index = std::numeric_limits<uint64_t>::max();

    // constant
    inline static constexpr Slot Invalid() SKR_NOEXCEPT { return {}; }

    // ctor & dtor & assign
    inline constexpr Slot() SKR_NOEXCEPT = default;
    inline constexpr explicit Slot(uint64_t index) SKR_NOEXCEPT : index(index) {}
    inline constexpr Slot(const Slot& other) SKR_NOEXCEPT            = default;
    inline constexpr Slot(Slot&& other) SKR_NOEXCEPT                 = default;
    inline constexpr Slot& operator=(const Slot& other) SKR_NOEXCEPT = default;
    inline constexpr Slot& operator=(Slot&& other) SKR_NOEXCEPT      = default;
    inline ~Slot() SKR_NOEXCEPT                                      = default;

    // compare
    inline constexpr bool operator==(const Slot& other) const SKR_NOEXCEPT { return index == other.index; }
    inline constexpr bool operator!=(const Slot& other) const SKR_NOEXCEPT { return index != other.index; }

    // boolean
    inline constexpr bool is_valid() const SKR_NOEXCEPT { return index != std::numeric_limits<uint64_t>::max(); }
    inline constexpr operator bool() const SKR_NOEXCEPT { return is_valid(); }
};
} // namespace skr::gui