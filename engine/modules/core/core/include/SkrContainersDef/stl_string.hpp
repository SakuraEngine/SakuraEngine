#pragma once
#include <SkrBase/config.h>
#include "SkrCore/memory/memory.h"
#include <string>
#include <string_view>

namespace skr
{

using stl_string [[sfinal_alias]] = std::basic_string<char, std::char_traits<char>, skr_stl_allocator<char>>;
using stl_wstring [[sfinal_alias]] = std::basic_string<wchar_t, std::char_traits<wchar_t>, skr_stl_allocator<wchar_t>>;
using stl_u8string [[sfinal_alias]] = std::basic_string<char8_t, std::char_traits<char8_t>, skr_stl_allocator<char8_t>>;
using stl_u16string [[sfinal_alias]] = std::basic_string<char16_t, std::char_traits<char16_t>, skr_stl_allocator<char16_t>>;
using stl_u32string [[sfinal_alias]] = std::basic_string<char32_t, std::char_traits<char32_t>, skr_stl_allocator<char32_t>>;

using stl_string_view [[sfinal_alias]] = std::basic_string_view<char, std::char_traits<char>>;
using stl_wstring_view [[sfinal_alias]] = std::basic_string_view<wchar_t, std::char_traits<wchar_t>>;
using stl_u8string_view [[sfinal_alias]] = std::basic_string_view<char8_t, std::char_traits<char8_t>>;
using stl_u16string_view [[sfinal_alias]] = std::basic_string_view<char16_t, std::char_traits<char16_t>>;
using stl_u32string_view [[sfinal_alias]] = std::basic_string_view<char32_t, std::char_traits<char32_t>>;

} // namespace skr