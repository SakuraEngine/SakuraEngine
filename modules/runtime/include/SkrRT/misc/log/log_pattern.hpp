#pragma once
#include "SkrRT/platform/memory.h"
#include "SkrRT/misc/demangle.hpp"
#include "SkrRT/misc/log/log_base.hpp"
#include "SkrRT/containers/string.hpp"

#include <EASTL/array.h>

namespace skr {
namespace log {

struct FormatArg
{
    using FormatFunc = ostr::codeunit_sequence(*)(const FormatArg&, const ostr::codeunit_sequence_view&);
    FormatArg() SKR_NOEXCEPT = default;
    ~FormatArg() SKR_NOEXCEPT
    {
        if (deleter && data)
            deleter(data);
        data = nullptr;
    }   

    template<typename T>
    void initialize(const char8_t* n)
    {
        name = n;
        
        const std::string_view demangle_name_view = skr::demangle<T>();
        demangle_name = ostr::codeunit_sequence((const char8_t*)demangle_name_view.data(), 
            (const char8_t*)(demangle_name_view.data() + demangle_name_view.size()));

        formatter = +[](const FormatArg& a, const ostr::codeunit_sequence_view& spec)
        {
            if (!a.data) 
                return ostr::codeunit_sequence();

            const T& arg = *(const T*)a.data;
            return ostr::argument_formatter<T>::produce(arg, spec);
        };
        setter = +[](FormatArg* p, void* v) { *(T*)p->data = *(T*)v; };
        deleter = +[](void* p) { SkrDelete((T*)p); };
    }
    
    template<typename T>
    void set(const T& v)
    {
        static const std::string demangle_name_T = skr::demangle<std::decay_t<T>>();
        auto n0 = demangle_name_T.c_str();
        auto n1 = demangle_name.c_str();
        SKR_ASSERT((::strcmp(n0, n1) == 0) && "Type mismatch!");

        if (!data)
        {
            data = SkrNew<T>();
        }
        if (setter)
        {
            setter(this, (void*)&v);
        }
    }

    const FormatFunc& get_formatter() const SKR_NOEXCEPT
    {
        return formatter;
    }

private:
    skr::string name;
    skr::string demangle_name;
    void* data = nullptr;
    FormatFunc formatter = nullptr;
    void(*deleter)(void*) = nullptr;
    void(*setter)(FormatArg* p, void*) = nullptr;
};

struct LogPattern
{
public:
    enum class TimestampPrecision : uint8_t
    {
        None,
        MilliSeconds,
        MicroSeconds,
        NanoSeconds
    };

    enum class Attribute : uint32_t
    {
        timestamp,
        level_id,
        level_name,
        logger_name,
        thread_id,
        thread_name,
        process_id,
        process_name,
        file_name,
        file_line,
        function_name,
        message,
        Count
    };
    static constexpr uint64_t kAttributeCount = (uint64_t)LogPattern::Attribute::Count;

    [[nodiscard]] skr::string const& pattern(
        const LogEvent& event,
        skr::string_view formatted_message
    ) SKR_NOEXCEPT;

    [[nodiscard]] skr::string const& last_result() SKR_NOEXCEPT;

    LogPattern(const char8_t* format_pattern) SKR_NOEXCEPT
    {
        _initialize();
        _set_pattern(format_pattern);
    }

    LogPattern(LogPattern const& other) = delete;
    LogPattern(LogPattern&& other) noexcept = delete;
    LogPattern& operator=(LogPattern const& other) = delete;
    LogPattern& operator=(LogPattern&& other) noexcept = delete;
    virtual ~LogPattern() SKR_NOEXCEPT;

protected:
    void _initialize() SKR_NOEXCEPT;
    void _set_pattern(skr::string pattern) SKR_NOEXCEPT;

    template <Attribute I, typename T>
    void _set_arg(const char8_t* name)
    {
        const auto idx = order_index_[(size_t)I];
        if (idx != -1)
            _args[idx].initialize<T>(name);
    }

    template <Attribute I, typename T>
    FORCEINLINE void _set_arg_val(T const& arg)
    {
        const auto idx = order_index_[(size_t)I];
        if (idx != -1)
            _args[order_index_[(size_t)I]].set(arg);
        SKR_ASSERT(idx != -1);
    }

    uint64_t pid_ = 0;

    skr::string calculated_format_ = u8"";
    uint32_t _args_n = 0;
    eastl::array<FormatArg, kAttributeCount> _args;
    eastl::array<int64_t, kAttributeCount> order_index_;
    eastl::array<bool, kAttributeCount> is_set_in_pattern_;

    skr::string formatted_string_ = u8"";
};

static constexpr uint64_t kAttributeCount = LogPattern::kAttributeCount;

} // namespace log
} // namespace skr

namespace ostr
{

template<>
struct argument_formatter<skr::log::FormatArg>
{
    static codeunit_sequence produce(const skr::log::FormatArg& value, const codeunit_sequence_view& specification)
    {
        return value.get_formatter()(value, specification);
    }
};
    
} // namespace ostr