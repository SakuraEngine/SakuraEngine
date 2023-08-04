
#include "wide_text.h"

#include "text.h"
#include "text_view.h"

namespace ostr
{
	namespace details
	{
		[[nodiscard]] constexpr u64 get_sequence_length(const wchar_t* str) noexcept
		{
			if(!str)
				return 0;
			u64 count = 0;
			while(str[count] != 0)
				++count;
			return count;
		}
	}
	
	wide_text::wide_text(const wchar_t* wide_str) noexcept
	{
		this->operator=(wide_str);
	}

	wide_text::wide_text(const codeunit_sequence_view& view) noexcept
	{
		this->operator=(view);
	}

	wide_text& wide_text::operator=(const wchar_t* wide_str) noexcept
	{
		this->sequence_.empty();
		this->sequence_.append(wide_str, details::get_sequence_length(wide_str) + 1);
		return *this;
	}

	wide_text& wide_text::operator=(const codeunit_sequence_view& view) noexcept
	{
		const text_view tv{ view };
		this->sequence_.empty();
		this->sequence_.reserve(tv.size() + 1);
		for(const codepoint cp : tv)
		{
#if _WIN64
			const auto utf16_pair = unicode::utf32_to_utf16(cp.get_codepoint());
			this->sequence_.push_back(utf16_pair.at(0));
			if(utf16_pair.at(1) != 0)
				this->sequence_.push_back(utf16_pair.at(1));
#elif __linux__ || __MACH__
			this->sequence_.push_back(cp.get_codepoint());
#endif
		}
		this->sequence_.push_back(L'\0');
		return *this;
	}

	const wchar_t* wide_text::data() const noexcept
	{
		return this->sequence_.data();
	}

	void wide_text::decode(codeunit_sequence& out) const noexcept
	{
#if _WIN64
		out = text::from_utf16(reinterpret_cast<const char16_t*>(this->sequence_.data())).raw();
#elif __linux__ || __MACH__
		out = text::from_utf32(reinterpret_cast<const char32_t*>(this->sequence_.data())).raw();
#endif
	}
}
