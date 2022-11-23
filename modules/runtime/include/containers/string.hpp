#pragma once
#include "platform/configure.h"

// #define SKR_USE_STL_STRING 

#if !defined(SKR_USE_STL_STRING)
#include <EASTL/string.h>

namespace skr
{
    using eastl::string;
    using eastl::string_view;
    using eastl::wstring;
    using eastl::wstring_view;
    using eastl::to_string;
    using eastl::to_wstring;
    using eastl::basic_string;
    using eastl::basic_string_view;
    #if EASTL_USER_LITERALS_ENABLED && EASTL_INLINE_NAMESPACES_ENABLED
		EA_DISABLE_VC_WARNING(4455) // disable warning C4455: literal suffix identifiers that do not start with an underscore are reserved
        inline namespace literals
	    {
		    inline namespace string_literals
		    {
    	        using namespace eastl::literals::string_literals;
		    }
	    }
		EA_RESTORE_VC_WARNING()  // warning: 4455
	#endif

	template <>
	struct hash<string>
	{
		inline size_t operator()(const string& x) const { return eastl::hash<string>()(x); }
	};

	template <>
	struct hash<wstring>
	{
		inline size_t operator()(const wstring& x) const { return eastl::hash<wstring>()(x); }
	};

	template <>
	struct hash<string_view>
	{
		inline size_t operator()(const string_view& x) const { return eastl::hash<string_view>()(x); }
	};

	template <>
	struct hash<wstring_view>
	{
		inline size_t operator()(const wstring_view& x) const { return eastl::hash<wstring_view>()(x); }
	};
}
#else
#include <string>

namespace skr
{
    using std::string;
    using std::string_view;
    using std::wstring;
    using std::wstring_view;
    using std::to_string;
    using std::to_wstring;
    using std::basic_string;
    using std::basic_string_view;
	inline namespace literals
	{
		inline namespace string_literals
		{
			using namespace std::literals::string_literals;
		}
	}

	template <>
	struct hash<string>
	{
		inline size_t operator()(const string& x) const { return std::hash<string>()(x); }
	};

	template <>
	struct hash<wstring>
	{
		inline size_t operator()(const wstring& x) const { return std::hash<wstring>()(x); }
	};

	template <>
	struct hash<string_view>
	{
		inline size_t operator()(const string_view& x) const { return std::hash<string_view>()(x); }
	};

	template <>
	struct hash<wstring_view>
	{
		inline size_t operator()(const wstring_view& x) const { return std::hash<wstring_view>()(x); }
	};
}
#endif

#include "type/type_id.hpp"

namespace skr {
namespace type {
// {214ed643-54bd-4213-be37-e336a77fde84}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr::string, 0x214ed643, 0x54bd, 0x4213, 0xbe, 0x37, 0xe3, 0x36, 0xa7, 0x7f, 0xde, 0x84);
// {b799ba81-6009-405d-9131-e4b6101660dc}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr::string_view, 0xb799ba81, 0x6009, 0x405d, 0x91, 0x31, 0xe4, 0xb6, 0x10, 0x16, 0x60, 0xdc);
} // namespace type
} // namespace skr