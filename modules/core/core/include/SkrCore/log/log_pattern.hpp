#pragma once
#include "SkrCore/memory/memory.h"
#include "SkrBase/misc/demangle.hpp"
#include "SkrCore/log/log_base.hpp"
#include "SkrContainersDef/string.hpp"
#include "SkrContainersDef/array.hpp"

namespace skr
{
namespace log
{

struct FormatArg {
    using FormatFunc         = void (*)(String&, const FormatArg& value, StringView spec);
    FormatArg() SKR_NOEXCEPT = default;
    ~FormatArg() SKR_NOEXCEPT
    {
        if (deleter && data)
            deleter(data);
        data = nullptr;
    }

    template <typename T>
    void initialize(const char8_t* n)
    {
        name = n;

        const std::string_view demangle_name_view = skr::demangle<T>();
        demangle_name                             = String::From(demangle_name_view.data(), demangle_name_view.size());
        formatter = +[](String& out, const FormatArg& a, StringView spec) {
            if (a.data)
            {
                const T& arg = *(const T*)a.data;
                ::skr::container::Formatter<T>::format(out, arg, spec);
            }
        };
        setter  = +[](FormatArg* p, void* v) { *(T*)p->data = *(T*)v; };
        deleter = +[](void* p) { SkrDelete((T*)p); };
    }

    template <typename T>
    void set(const T& v)
    {
        static const std::string demangle_name_T = skr::demangle<std::decay_t<T>>();
        auto                     n0              = demangle_name_T.c_str();
        auto                     n1              = demangle_name.c_str_raw();
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
    skr::String name;
    skr::String demangle_name;
    void*       data                    = nullptr;
    FormatFunc  formatter               = nullptr;
    void (*deleter)(void*)              = nullptr;
    void (*setter)(FormatArg* p, void*) = nullptr;
};

struct LogPattern {
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

    [[nodiscard]] skr::String pattern(const LogEvent& event, skr::StringView formatted_message) SKR_NOEXCEPT;

    LogPattern(const char8_t* format_pattern) SKR_NOEXCEPT
    {
        _initialize();
        _set_pattern(format_pattern);
    }

    LogPattern(LogPattern const& other)                = delete;
    LogPattern(LogPattern&& other) noexcept            = delete;
    LogPattern& operator=(LogPattern const& other)     = delete;
    LogPattern& operator=(LogPattern&& other) noexcept = delete;
    virtual ~LogPattern() SKR_NOEXCEPT;

protected:
    void _initialize() SKR_NOEXCEPT;
    void _set_pattern(skr::String pattern) SKR_NOEXCEPT;

    template <Attribute I, typename T>
    void _set_arg(const char8_t* name)
    {
        const auto idx = order_index_[(size_t)I];
        if (idx != -1)
            _args[idx].initialize<T>(name);
    }

    template <Attribute I, typename T>
    SKR_FORCEINLINE void _set_arg_val(T const& arg)
    {
        const auto idx = order_index_[(size_t)I];
        if (idx != -1)
            _args[order_index_[(size_t)I]].set(arg);
        SKR_ASSERT(idx != -1);
    }

    uint64_t pid_ = 0;

    skr::String                            calculated_format_ = u8"";
    uint32_t                               _args_n            = 0;
    skr::Array<FormatArg, kAttributeCount> _args;
    skr::Array<int64_t, kAttributeCount>   order_index_;
    skr::Array<bool, kAttributeCount>      is_set_in_pattern_;
};

static constexpr uint64_t kAttributeCount = LogPattern::kAttributeCount;

} // namespace log
} // namespace skr

namespace skr::container
{

template <>
struct Formatter<skr::log::FormatArg> {
    inline static void format(String& out, const skr::log::FormatArg& value, StringView spec)
    {
        value.get_formatter()(out, value, spec);
    }
};

} // namespace ostr