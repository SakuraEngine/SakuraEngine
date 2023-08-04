
#pragma once

#include "codeunit_sequence.h"
#include "codeunit_sequence_view.h"
#include "common/sequence.h"

namespace ostr
{
	class OPEN_STRING_API wide_text
	{
	public:
		explicit wide_text(const wchar_t* wide_str) noexcept;
		explicit wide_text(const codeunit_sequence_view& view) noexcept;
		wide_text& operator=(const wchar_t* wide_str) noexcept;
		wide_text& operator=(const codeunit_sequence_view& view) noexcept;

		[[nodiscard]] const wchar_t* data() const noexcept;
		void decode(codeunit_sequence& out) const noexcept;
	private:
		sequence<wchar_t, 128> sequence_{ };
	};

	template<> 
	struct argument_formatter<wide_text>
	{
		static codeunit_sequence produce(const wide_text& value, const codeunit_sequence_view& specification)
		{
			codeunit_sequence result;
			value.decode(result);
			return result;
		}
	};

	template<> 
	struct argument_formatter<const wchar_t*>
	{
		static codeunit_sequence produce(const wchar_t* value, const codeunit_sequence_view& specification)
		{
			return argument_formatter<wide_text>::produce(wide_text{ value }, specification);
		}
	};

	template<size_t N> 
	struct argument_formatter<wchar_t[N]>
	{
		static codeunit_sequence produce(const wchar_t (&value)[N], const codeunit_sequence_view& specification)
		{
			return argument_formatter<wide_text>::produce(wide_text{ value }, specification);
		}
	};
}
