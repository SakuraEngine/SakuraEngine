#pragma once 
#include <string_view>

namespace skr
{
    template <std::string_view const&... Strs>
    struct join
    {
        // Join all strings into a single std::array of chars
        static constexpr auto impl() noexcept
        {
            constexpr std::size_t len = (Strs.size() + ... + 0);
            std::array<char, len + 1> arr{};
            auto append = [i = 0, &arr](auto const& s) mutable {
                for (auto c : s) arr[i++] = c;
            };
            (append(Strs), ...);
            arr[len] = 0;
            return arr;
        }
        // Give the joined string static storage
        static constexpr auto arr = impl();
        // View as a std::string_view
        static constexpr std::string_view value {arr.data(), arr.size() - 1};
    };
    // Helper to get the value out
    template <std::string_view const&... Strs>
    static constexpr auto join_v = join<Strs...>::value;
}