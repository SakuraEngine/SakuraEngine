
#pragma once
#include "OpenString/common/sequence.h"
#include "OpenString/text_view.h"
#include "OpenString/codeunit_sequence.h"

namespace ostr
{
	class OPEN_STRING_API text
	{
	public:

		text() noexcept;
		text(const text&) noexcept;
		text(text&&) noexcept;
		text& operator=(const text&) noexcept;
		text& operator=(text&&) noexcept;
		~text();

		text(const ochar8_t* str) noexcept;
		text(const text_view& view) noexcept;
		text(codeunit_sequence sequence) noexcept;
		text(const codeunit_sequence_view& sequence) noexcept;

		static text from_utf8(const ochar8_t* string_utf8) noexcept;
		static text from_utf16(const char16_t* string_utf16) noexcept;
		static text from_utf32(const char32_t* string_utf32) noexcept;
		static text from_wide(const wchar_t* wide_string) noexcept;
	
		template<class...Args>
		static text build(const Args&... argument);
		template<typename Container>
		static text join(const Container& container, const text_view& separator) noexcept;

		// code-region-start: iterator

		using const_iterator = text_view::const_iterator;

		struct OPEN_STRING_API iterator
		{
			iterator() noexcept = default;
			iterator(text& t, u64 from, u64 size) noexcept;

			struct OPEN_STRING_API codepoint_accessor
			{
				explicit codepoint_accessor(iterator& iter) noexcept;
			
				codepoint_accessor& operator=(ochar8_t c) noexcept;
				codepoint_accessor& operator=(char32_t cp) noexcept;
				codepoint_accessor& operator=(const codepoint& cp) noexcept;
				codepoint_accessor& operator=(const text_view& tv) noexcept;
				codepoint_accessor& operator=(const text& t) noexcept;

				[[nodiscard]] codepoint get_codepoint() const noexcept;
				[[nodiscard]] operator codepoint() const noexcept;

			private:
			
				void assign(const codeunit_sequence_view& sequence_view) const noexcept;
			
				iterator& it_;
			};

			[[nodiscard]] bool is_valid() const noexcept;
			[[nodiscard]] u64 raw_size() const noexcept;
			[[nodiscard]] codepoint get_codepoint() const noexcept;
			[[nodiscard]] codepoint operator*() const noexcept;
			[[nodiscard]] codepoint_accessor operator*() noexcept;
			iterator& operator++() noexcept;
			iterator operator++(int) noexcept;
			iterator& operator--() noexcept;
			iterator operator--(int) noexcept;
			[[nodiscard]] bool operator==(const iterator& rhs) const noexcept;
			[[nodiscard]] bool operator!=(const iterator& rhs) const noexcept;

			u64 from = global_constant::INDEX_INVALID;
			u64 size = SIZE_MAX;
			text* owner = nullptr;
		};
	
		[[nodiscard]] iterator begin() noexcept;
		[[nodiscard]] const_iterator begin() const noexcept;
		[[nodiscard]] iterator end() noexcept;
		[[nodiscard]] const_iterator end() const noexcept;
		[[nodiscard]] const_iterator cbegin() const noexcept;
		[[nodiscard]] const_iterator cend() const noexcept;

		// code-region-end: iterator

		[[nodiscard]] codeunit_sequence& raw() & noexcept;
		[[nodiscard]] const codeunit_sequence& raw() const& noexcept;
		[[nodiscard]] codeunit_sequence raw() && noexcept;
	
		[[nodiscard]] text_view view() const noexcept;

		[[nodiscard]] u64 size() const noexcept;
		[[nodiscard]] bool is_empty() const noexcept;

		[[nodiscard]] bool operator==(const text_view& rhs) const noexcept;
		[[nodiscard]] bool operator==(const text& rhs) const noexcept;
		[[nodiscard]] bool operator==(const ochar8_t* rhs) const noexcept;
		[[nodiscard]] bool operator!=(const text_view& rhs) const noexcept;
		[[nodiscard]] bool operator!=(const text& rhs) const noexcept;
		[[nodiscard]] bool operator!=(const ochar8_t* rhs) const noexcept;

		text& append(const text_view& rhs) noexcept;
		text& append(const text& rhs) noexcept;
		text& append(const codepoint& cp) noexcept;
		text& append(const ochar8_t* rhs) noexcept;
		text& append(ochar8_t codeunit, u64 count = 1) noexcept;

		text& operator+=(const text_view& rhs) noexcept;
		text& operator+=(const text& rhs) noexcept;
		text& operator+=(const codepoint& cp) noexcept;
		text& operator+=(const ochar8_t* rhs) noexcept;
		text& operator+=(ochar8_t codeunit) noexcept;
	
		[[nodiscard]] text_view subview(u64 from, u64 size = global_constant::SIZE_INVALID) const noexcept;
		text& subtext(u64 from, u64 size = global_constant::SIZE_INVALID) noexcept;
	
		[[nodiscard]] u64 index_of(const text_view& pattern, u64 from = 0, u64 size = global_constant::SIZE_INVALID) const noexcept;
		[[nodiscard]] u64 last_index_of(const text_view& pattern, u64 from = 0, u64 size = global_constant::SIZE_INVALID) const noexcept;
	
		[[nodiscard]] u64 count(const text_view& pattern, u64 from = 0, u64 size = global_constant::SIZE_INVALID) const noexcept;

		[[nodiscard]] bool starts_with(const text_view& prefix) const noexcept;
		[[nodiscard]] bool ends_with(const text_view& suffix) const noexcept;

		/**
		 * Empty the text without reallocation.
		 */
		void empty() noexcept;

		// How to reserve for unknown size?
		// If you got a sequence of code units,
		// please use t.raw().reserve(size);
		text& reserve(u64) noexcept = delete;

		text& write_at(u64 index, codepoint cp) noexcept;
		[[nodiscard]] codepoint read_at(u64 index) const noexcept;
	
		/// @note: Please use operator[] for read-only access, or use write_at method for write access.
		[[nodiscard]] codepoint operator[](u64 index) const noexcept;

		text& reverse(u64 from = 0, u64 size = SIZE_MAX) noexcept;

		u32 split(const text_view& splitter, sequence<text_view>& pieces, bool cull_empty = true) const noexcept;

		text& replace(const text_view& destination, const text_view& source, u64 from = 0, u64 size = SIZE_MAX);
		text& replace(const text_view& destination, u64 from = 0, u64 size = SIZE_MAX);
	
		text& self_remove_prefix(const text_view& prefix) noexcept;
		text& self_remove_suffix(const text_view& suffix) noexcept;
		[[nodiscard]] text_view view_remove_prefix(const text_view& prefix) const noexcept;
		[[nodiscard]] text_view view_remove_suffix(const text_view& suffix) const noexcept;

		text& self_trim_start(const text_view& characters = text_view(u8" \t")) noexcept;
		text& self_trim_end(const text_view& characters = text_view(u8" \t")) noexcept;
		text& self_trim(const text_view& characters = text_view(u8" \t")) noexcept;

		[[nodiscard]] text_view view_trim_start(const text_view& characters = text_view(u8" \t")) const noexcept;
		[[nodiscard]] text_view view_trim_end(const text_view& characters = text_view(u8" \t")) const noexcept;
		[[nodiscard]] text_view view_trim(const text_view& characters = text_view(u8" \t")) const noexcept;

		[[nodiscard]] const ochar_t* c_str() const noexcept;
		[[nodiscard]] const ochar8_t* u8_str() const noexcept;

	private:

		codeunit_sequence sequence_{ };
	
	};

	namespace details
	{
		template<>
		[[nodiscard]] constexpr codeunit_sequence_view view_sequence<text_view>(const text_view& v)
		{
			return v.raw();
		}
		template<>
		[[nodiscard]] inline codeunit_sequence_view view_sequence<text>(const text& v)
		{
			return v.view().raw();
		}
	}

	template<class...Args>
	text text::build(const Args&... argument)
	{
		return { codeunit_sequence::build(argument...) };
	}

	template<typename Container>
	text text::join(const Container& container, const text_view& separator) noexcept
	{
		return { codeunit_sequence::join(container, separator.raw()) };
	}

	[[nodiscard]] OPEN_STRING_API bool operator==(const text_view& lhs, const text& rhs) noexcept;

	template<> 
	struct argument_formatter<text_view>
	{
		static codeunit_sequence produce(const text_view& value, const codeunit_sequence_view& specification)
		{
			return codeunit_sequence{ value.raw() };
		}
	};

	template<> 
	struct argument_formatter<text>
	{
		static codeunit_sequence produce(const text& value, const codeunit_sequence_view& specification)
		{
			return value.raw();
		}
	};
}
