#include "SkrCore/process.h"
#include "SkrCore/log/logger.hpp"
#include "SkrCore/log/log_pattern.hpp"
#include "SkrContainersDef/array.hpp"
#include "SkrContainersDef/hashmap.hpp"
#include "SkrProfile/profile.h"
#include "./log_manager.hpp"
// TODO: REMOVE THIS
#include <string>

namespace skr::log
{

using U8String = std::basic_string<char8_t, std::char_traits<char8_t>, skr_stl_allocator<char8_t>>;
struct named_arg_info {
    U8String name;
    int64_t  id;
};

/***/
LogPattern::Attribute attribute_from_string(U8String const& attribute_name)
{
    static skr::FlatHashMap<skr::String, LogPattern::Attribute, skr::Hash<skr::String>> const attr_map = {
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
constexpr void _store_named_args(skr::Array<named_arg_info, kAttributeCount>&)
{
}

/***/
template <size_t Idx, size_t NamedIdx, typename Arg, typename... Args>
constexpr void _store_named_args(skr::Array<named_arg_info, kAttributeCount>& named_args_store,
                                 const Arg&                                   arg, const Args&... args)
{
    named_args_store[NamedIdx] = { U8String(arg), Idx };
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
[[nodiscard]] std::pair<skr::String, skr::Array<int64_t, kAttributeCount>> _generate_fmt_format_string(
skr::Array<bool, kAttributeCount>& is_set_in_pattern, uint32_t& args_n, U8String pattern_string,
Args const&... args)
{
    // Attribute enum and the args we are passing here must be in sync
    static_assert(kAttributeCount == sizeof...(Args));

    skr::Array<int64_t, kAttributeCount> order_index{};
    order_index.fill(-1);

    skr::Array<named_arg_info, kAttributeCount> named_args{};
    _store_named_args<0, 0>(named_args, args...);
    args_n = 0;

    // we will replace all %(....) with {} to construct a string to pass to fmt library
    size_t arg_identifier_pos = pattern_string.find_first_of('%');
    while (arg_identifier_pos != U8String::npos)
    {
        size_t open_paren_pos = pattern_string.find_first_of('(', arg_identifier_pos);
        if (open_paren_pos != U8String::npos && (open_paren_pos - arg_identifier_pos) == 1)
        {
            // if we found '%(' we have a matching pattern and we now need to get the closed paren
            size_t closed_paren_pos = pattern_string.find_first_of(')', open_paren_pos);

            if (closed_paren_pos == U8String::npos)
            {
                SKR_ASSERT(0 && "Invalid format pattern");
            }

            // We have everything, get the substring, this time including '%( )'
            U8String attr = pattern_string.substr(arg_identifier_pos, (closed_paren_pos + 1) - arg_identifier_pos);

            // find any user format specifiers
            size_t const pos = attr.find(':');
            U8String     attr_name;

            if (pos != U8String::npos)
            {
                // we found user format specifiers that we want to keep.
                // e.g. %(fileline:<32)

                // Get only the format specifier
                // e.g. :<32
                U8String custom_format_specifier = attr.substr(pos);
                custom_format_specifier.pop_back(); // remove the ")"

                // replace with the pattern with the correct value
                U8String value;
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
            LogPattern::Attribute const attr_enum_value  = attribute_from_string(attr_name);
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

    skr::String pattern = pattern_string.c_str();
    return std::make_pair(pattern, order_index);
}

void LogPattern::_initialize() SKR_NOEXCEPT
{
    // Initialize arrays
    for (size_t i = 0; i < kAttributeCount; ++i)
    {
        order_index_[i]       = -1;
        is_set_in_pattern_[i] = false;
    }
    pid_ = skr_get_current_process_id();
}

void LogPattern::_set_pattern(skr::String pattern) SKR_NOEXCEPT
{
    skr::String format_pattern = pattern;
    format_pattern.append(u8"\n");
    {
        // the order we pass the arguments here must match with the order of Attribute enum
        auto _ = _generate_fmt_format_string(
        is_set_in_pattern_, _args_n, format_pattern.c_str(),
        u8"timestamp", u8"level_id", u8"level_name", u8"logger_name",
        u8"thread_id", u8"thread_name", u8"process_id", u8"process_name",
        u8"file_name", u8"file_line", u8"function_name", u8"message");

        calculated_format_ = _.first;
        order_index_       = _.second;

        _set_arg<Attribute::timestamp, skr::StringView>(u8"timestamp");
        _set_arg<Attribute::level_id, uint32_t>(u8"level_id");
        _set_arg<Attribute::level_name, skr::StringView>(u8"level_name");
        _set_arg<Attribute::logger_name, skr::StringView>(u8"logger_name");
        _set_arg<Attribute::thread_id, uint64_t>(u8"thread_id");
        _set_arg<Attribute::thread_name, skr::StringView>(u8"thread_name");
        _set_arg<Attribute::process_id, uint64_t>(u8"process_id");
        _set_arg<Attribute::process_name, skr::StringView>(u8"process_name");
        _set_arg<Attribute::file_name, skr::StringView>(u8"file_name");
        _set_arg<Attribute::file_line, skr::StringView>(u8"file_line");
        _set_arg<Attribute::function_name, skr::StringView>(u8"function_name");
        _set_arg<Attribute::message, skr::StringView>(u8"message");
    }
}

LogPattern::~LogPattern() SKR_NOEXCEPT
{
}

template <typename T>
struct Convert {
    auto operator()(FormatArg const& t) -> T
    {
        return t;
    }
};

template <size_t... N>
skr::String format_NArgs(std::index_sequence<N...>, const skr::StringView& fmt, const skr::Array<FormatArg, kAttributeCount>& args)
{
    return skr::format(fmt, args[N]...);
}

const static char8_t*  main_thread_name    = u8"main";
const static char8_t*  unknown_thread_name = u8"unnamed";
const static SThreadID main_thread_id      = skr_current_thread_id();
static thread_local skr::String timestring = u8"";
skr::String LogPattern::pattern(const LogEvent& event, skr::StringView formatted_message) SKR_NOEXCEPT
{
    skr::String formatted_string = u8"";
    if (calculated_format_.is_empty())
        return formatted_string;

    const auto level_id = (uint32_t)event.level;

    if (is_set_in_pattern_[(size_t)Attribute::timestamp])
    {
        SkrZoneScopedN("LogPattern::Time");

        const auto& dt         = LogManager::Get()->datetime_;
        const auto  midnightNs = dt.midnightNs;
        const auto  ts         = LogManager::Get()->tscns_.tsc2ns(event.timestamp);
        auto        t          = (ts > midnightNs) ? (ts - midnightNs) : 0;
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
        if (h > 23)
        {
            h %= 24;
            LogManager::Get()->datetime_.reset_date();
        }
        timestring = skr::format(
            u8"{}/{}/{} {}:{}:{}({}:{})",
            dt.year, dt.month, dt.day,
            h, minute, second, ms, us
        );
        _set_arg_val<Attribute::timestamp>(timestring.view());
    }

    if (is_set_in_pattern_[(size_t)Attribute::level_id])
    {
        _set_arg_val<Attribute::level_id>(level_id);
    }

    if (is_set_in_pattern_[(size_t)Attribute::level_name])
    {
        const auto level_name = skr::StringView(LogConstants::kLogLevelNameLUT[level_id]);
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
        auto thread_name = skr::StringView(event.thread_name);
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
        const auto process_name = skr::StringView(skr_get_current_process_name());
        _set_arg_val<Attribute::process_name>(process_name);
    }

    if (is_set_in_pattern_[(size_t)Attribute::file_name])
    {
        const auto file_name = skr::StringView((const char8_t*)event.src_data.file_);
        _set_arg_val<Attribute::file_name>(file_name);
    }

    if (is_set_in_pattern_[(size_t)Attribute::file_line])
    {
        const auto file_line = skr::StringView((const char8_t*)event.src_data.line_);
        _set_arg_val<Attribute::file_line>(file_line);
    }

    if (is_set_in_pattern_[(size_t)Attribute::function_name])
    {
        const auto function_name = skr::StringView((const char8_t*)event.src_data.func_);
        _set_arg_val<Attribute::function_name>(function_name);
    }

    if (is_set_in_pattern_[(size_t)Attribute::message])
    {
        _set_arg_val<Attribute::message>(formatted_message);
    }

    {
        SkrZoneScopedN("LogPattern::FormatNArgs");
        auto sequence     = std::make_index_sequence<kAttributeCount>();
        formatted_string = format_NArgs(sequence, calculated_format_.view(), _args);
    }

    return formatted_string;
}

} // namespace skr::log