#include "misc/log/log_manager.hpp"
#include "SkrRT/platform/process.h"
#include "SkrRT/misc/log/logger.hpp"
#include "SkrRT/misc/log/log_pattern.hpp"
#include "SkrRT/containers/hashmap.hpp"
#include <EASTL/string.h>
#include "SkrProfile/profile.h"

namespace skr::log
{

struct named_arg_info {
    eastl::u8string name;
    int64_t id;
};

/***/
LogPattern::Attribute attribute_from_string(eastl::u8string const& attribute_name)
{
    static skr::flat_hash_map<skr::string, LogPattern::Attribute, skr::hash<skr::string>> const attr_map = {
        { u8"timestamp", LogPattern::Attribute::timestamp },
        { u8"level_id", LogPattern::Attribute::level_id },
        { u8"level_name", LogPattern::Attribute::level_name },
        { u8"logger_name", LogPattern::Attribute::logger_name },
        { u8"thread_id", LogPattern::Attribute::thread_id },
        { u8"thread_name", LogPattern::Attribute::thread_name },
        { u8"process_id", LogPattern::Attribute::process_id },
        { u8"process_name", LogPattern::Attribute::process_name },
        { u8"file_name", LogPattern::Attribute::file_name },
        { u8"file_line", LogPattern::Attribute::file_line },
        { u8"function_name", LogPattern::Attribute::function_name },
        { u8"message", LogPattern::Attribute::message }
    };

    auto const search = attr_map.find(attribute_name.c_str());

    if (search == attr_map.cend())
    {
        SKR_ASSERT(0 && "Attribute enum value does not exist for attribute_name");
        return LogPattern::Attribute::Count;
    }

    return search->second;
}

template <size_t, size_t>
constexpr void _store_named_args(eastl::array<named_arg_info, kAttributeCount>&)
{
}

/***/
template <size_t Idx, size_t NamedIdx, typename Arg, typename... Args>
constexpr void _store_named_args(eastl::array<named_arg_info, kAttributeCount>& named_args_store,
    const Arg& arg, const Args&... args)
{
    named_args_store[NamedIdx] = { eastl::u8string(arg), Idx };
    _store_named_args<Idx + 1, NamedIdx + 1>(named_args_store, args...);
}

/**
 * Convert the pattern to fmt format string and also populate the order index array
 * e.g. given :
 *   "%(timestamp) [%(thread_id)] %(file_name):%(lineno) %(level_name) %(logger_name) - "
 *
 * is changed to :
 *  {} [{}] {}:{} {:<12} {} -
 *
 *  with a order index of :
 *  i: 0 order idx[i] is: 0 - %(timestamp)
 *  i: 1 order idx[i] is: 10 - empty
 *  i: 2 order idx[i] is: 4 - %(level_name)
 *  i: 3 order idx[i] is: 5 - %(logger_name)
 *  i: 4 order idx[i] is: 1 - %(thread_id)
 *  ...
 * @tparam Args Args
 * @param pattern pattern
 * @param args args
 * @return processed pattern
 */
template <typename... Args>
[[nodiscard]] eastl::pair<skr::string, eastl::array<int64_t, kAttributeCount>> _generate_fmt_format_string(
    eastl::array<bool, kAttributeCount>& is_set_in_pattern, uint32_t& args_n, eastl::u8string pattern_string,
    Args const&... args)
{
    // Attribute enum and the args we are passing here must be in sync
    static_assert(kAttributeCount == sizeof...(Args));

    eastl::array<int64_t, kAttributeCount> order_index{};
    order_index.fill(-1);

    eastl::array<named_arg_info, kAttributeCount> named_args{};
    _store_named_args<0, 0>(named_args, args...);
    args_n = 0;

    // we will replace all %(....) with {} to construct a string to pass to fmt library
    size_t arg_identifier_pos = pattern_string.find_first_of('%');
    while (arg_identifier_pos != eastl::u8string::npos)
    {
        size_t open_paren_pos = pattern_string.find_first_of('(', arg_identifier_pos);
        if (open_paren_pos != eastl::u8string::npos && (open_paren_pos - arg_identifier_pos) == 1)
        {
            // if we found '%(' we have a matching pattern and we now need to get the closed paren
            size_t closed_paren_pos = pattern_string.find_first_of(')', open_paren_pos);

            if (closed_paren_pos == eastl::u8string::npos)
            {
                SKR_ASSERT(0 && "Invalid format pattern");
            }

            // We have everything, get the substring, this time including '%( )'
            eastl::u8string attr = pattern_string.substr(arg_identifier_pos, (closed_paren_pos + 1) - arg_identifier_pos);

            // find any user format specifiers
            size_t const pos = attr.find(':');
            eastl::u8string attr_name;

            if (pos != eastl::u8string::npos)
            {
                // we found user format specifiers that we want to keep.
                // e.g. %(fileline:<32)

                // Get only the format specifier
                // e.g. :<32
                eastl::u8string custom_format_specifier = attr.substr(pos);
                custom_format_specifier.pop_back(); // remove the ")"

                // replace with the pattern with the correct value
                eastl::u8string value;
                value += u8"{";
                value += custom_format_specifier;
                value += u8"}";

                // e.g. {:<32}
                pattern_string.replace(arg_identifier_pos, attr.length(), value);

                // Get the part that is the named argument
                // e.g. fileline
                attr_name = attr.substr(2, pos - 2);
            }
            else
            {
                // Make the replacement.
                pattern_string.replace(arg_identifier_pos, attr.length(), u8"{}");

                // Get the part that is the named argument
                // e.g. fileline
                attr.pop_back(); // remove the ")"

                attr_name = attr.substr(2, attr.size());
            }

            // reorder
            int64_t id = -1;
            for (size_t i = 0; i < kAttributeCount; ++i)
            {
                if (named_args[i].name == attr_name)
                {
                    id = named_args[i].id;
                    break;
                }
            }

            if (id < 0)
                SKR_ASSERT(0 && "Invalid format pattern");

            order_index[static_cast<size_t>(id)] = args_n++;

            // Also set the value as used in the pattern in our bitset for lazy evaluation
            LogPattern::Attribute const attr_enum_value = attribute_from_string(attr_name);
            is_set_in_pattern[(uint32_t)attr_enum_value] = true;

            // Look for the next pattern to replace
            arg_identifier_pos = pattern_string.find_first_of('%');
        }
        else
        {
            // search for the next '%'
            arg_identifier_pos = pattern_string.find_first_of('%', arg_identifier_pos + 1);
        }
    }

    skr::string pattern = pattern_string.c_str();
    return eastl::make_pair(pattern, order_index);
}

void LogPattern::_initialize() SKR_NOEXCEPT
{
    // Initialize arrays
    for (size_t i = 0; i < kAttributeCount; ++i)
    {
        order_index_[i] = -1;
        is_set_in_pattern_[i] = false;
    }
    pid_ = skr_get_current_process_id();
}

void LogPattern::_set_pattern(skr::string pattern) SKR_NOEXCEPT
{
    skr::string format_pattern = pattern;
    format_pattern += u8"\n";
    {
        // the order we pass the arguments here must match with the order of Attribute enum
        auto _ = _generate_fmt_format_string(
            is_set_in_pattern_, _args_n, format_pattern.u8_str(), 
            u8"timestamp", u8"level_id", u8"level_name", u8"logger_name",
            u8"thread_id", u8"thread_name", u8"process_id", u8"process_name", 
            u8"file_name", u8"file_line", u8"function_name", u8"message"
        );
        
        calculated_format_ = _.first;
        order_index_ = _.second;

        _set_arg<Attribute::timestamp, skr::string_view>(u8"timestamp");
        _set_arg<Attribute::level_id, uint32_t>(u8"level_id");
        _set_arg<Attribute::level_name, skr::string_view>(u8"level_name");
        _set_arg<Attribute::logger_name, skr::string_view>(u8"logger_name");
        _set_arg<Attribute::thread_id, uint64_t>(u8"thread_id");
        _set_arg<Attribute::thread_name, skr::string_view>(u8"thread_name");
        _set_arg<Attribute::process_id, uint64_t>(u8"process_id");
        _set_arg<Attribute::process_name, skr::string_view>(u8"process_name");
        _set_arg<Attribute::file_name, skr::string_view>(u8"file_name");
        _set_arg<Attribute::file_line, skr::string_view>(u8"file_line");
        _set_arg<Attribute::function_name, skr::string_view>(u8"function_name");
        _set_arg<Attribute::message, skr::string_view>(u8"message");
    }
}

LogPattern::~LogPattern() SKR_NOEXCEPT
{

}

template<typename T>
struct Convert {
    auto operator()(FormatArg const& t) -> T {
        return t;
    }
};

template <size_t... N>
skr::string format_NArgs(eastl::index_sequence<N...>, const skr::string_view& fmt, const eastl::array<FormatArg, kAttributeCount>& args)
{
    return skr::format(fmt, args[N]...);
}

const static char8_t* main_thread_name = u8"main";
const static char8_t* unknown_thread_name = u8"unknown";
const static SThreadID main_thread_id = skr_current_thread_id();
static skr::string timestring = u8"";
skr::string const& LogPattern::pattern(const LogEvent& event, skr::string_view formatted_message) SKR_NOEXCEPT
{
    formatted_string_.empty();
    if (calculated_format_.is_empty())
        return formatted_string_;
    
    const auto level_id = (uint32_t)event.level;

    if (is_set_in_pattern_[(size_t)Attribute::timestamp])
    {
        SkrZoneScopedN("LogPattern::Time");

        const auto& dt = LogManager::Get()->datetime_;
        const auto midnightNs = dt.midnightNs;
        const auto ts = LogManager::Get()->tscns_.tsc2ns(event.timestamp);
        auto t =  (ts > midnightNs) ? (ts - midnightNs) : 0;
        t /= 1'000;
        const uint64_t us = t % 1'000;
        t /= 1'000;
        const uint64_t ms = t % 1'000;
        t /= 1'000;
        const uint64_t second = t % 60;
        t /= 60;
        const uint64_t minute = (t % 60);
        t /= 60;
        uint32_t h = (uint32_t)t;
        if (h > 23) {
            h %= 24;
            LogManager::Get()->datetime_.reset_date();
        }
        timestring = skr::format(
            u8"{}/{}/{} {}:{}:{}({}:{})",
            dt.year, dt.month, dt.day,
            h, minute, second, ms, us);
        _set_arg_val<Attribute::timestamp>(timestring.view());
    }
        
    if (is_set_in_pattern_[(size_t)Attribute::level_id])
    {
        _set_arg_val<Attribute::level_id>(level_id);
    }
        
    if (is_set_in_pattern_[(size_t)Attribute::level_name])
    {
        const auto level_name = skr::string_view(LogConstants::kLogLevelNameLUT[level_id]);
        _set_arg_val<Attribute::level_name>(level_name);
    }

    if (is_set_in_pattern_[(size_t)Attribute::logger_name])
    {
        const auto logger_name = event.logger->get_name();
        _set_arg_val<Attribute::logger_name>(logger_name);
    }

    if (is_set_in_pattern_[(size_t)Attribute::thread_id])
    {
        const auto thread_id = event.thread_id;
        _set_arg_val<Attribute::thread_id>(thread_id);
    }

    if (is_set_in_pattern_[(size_t)Attribute::thread_name])
    {
        auto thread_name = skr::string_view(event.thread_name);
        if (thread_name.is_empty())
            thread_name = (event.thread_id == main_thread_id) ? main_thread_name : unknown_thread_name;
        _set_arg_val<Attribute::thread_name>(thread_name);
    }

    if (is_set_in_pattern_[(size_t)Attribute::process_id])
    {
        const auto process_id = pid_;
        _set_arg_val<Attribute::process_id>(process_id);
    }

    if (is_set_in_pattern_[(size_t)Attribute::process_name])
    {
        const auto process_name = skr::string_view(LogConstants::kLogLevelNameLUT[level_id]); // TODO
        _set_arg_val<Attribute::process_name>(process_name);
    }

    if (is_set_in_pattern_[(size_t)Attribute::file_name])
    {
        const auto file_name = skr::string_view((const char8_t*)event.src_data.file_);
        _set_arg_val<Attribute::file_name>(file_name);
    }

    if (is_set_in_pattern_[(size_t)Attribute::file_line])
    {
        const auto file_line = skr::string_view((const char8_t*)event.src_data.line_);
        _set_arg_val<Attribute::file_line>(file_line);
    }

    if (is_set_in_pattern_[(size_t)Attribute::function_name])
    {
        const auto function_name = skr::string_view((const char8_t*)event.src_data.func_);
        _set_arg_val<Attribute::function_name>(function_name);
    }

    if (is_set_in_pattern_[(size_t)Attribute::message])
    {
        _set_arg_val<Attribute::message>(formatted_message);
    }

    {
        SkrZoneScopedN("LogPattern::FormatNArgs");
        auto sequence = eastl::make_index_sequence<kAttributeCount>();
        formatted_string_ = format_NArgs(sequence, calculated_format_.view(), _args);
    }

    return formatted_string_;
}

skr::string const& LogPattern::last_result() SKR_NOEXCEPT
{
    return formatted_string_;
}

} // namespace skr::log