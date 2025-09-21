#pragma once
#include <SkrBase/config.h>
#include <SkrBase/types.h>
#include <SkrContainersDef/string.hpp>
#include <SkrContainersDef/vector.hpp>
#include <source_location>
#include <SkrCore/log.hpp>

namespace skr
{
struct ArchiveErrorTracker
{
    enum class EStructureKind
    {
        Array,
        Object,
    };
    struct StructureStack
    {
        EStructureKind kind;
        String key;
    };

    inline bool assume_succeeded_or_dump_error() const
    {
        if (any_error()) [[unlikely]]
        {
            dump_checkpoint_trace();
            mark_error_handled();
            return false;
        }
        else
        {
            return true;
        }
    }

    inline ~ArchiveErrorTracker()
    {
        if (!is_error_handled())
        {
            assume_succeeded_or_dump_error();
        }
    }

    // getter
    inline uint32_t warn_count() const { return _warn_count; }
    inline uint32_t error_count() const { return _error_count; }
    inline bool any_error() const { return _error_count > 0; }
    inline bool any_warn() const { return _warn_count > 0; }
    inline const String& checkpoint_trace_message() const { return _checkpoint_trace_message; }

    // error handling
    inline bool is_error_handled() const
    {
        return _is_error_handled;
    }
    inline void mark_error_handled() const
    {
        _is_error_handled = true;
    }

    // dump log
    inline void dump_checkpoint_trace() const
    {
        if (enable_checkpoint_trace)
        {
            // dump checkpoint trace
            SKR_LOG_FMT_ERROR(
                u8"Archive error trace:\n{}",
                _checkpoint_trace_message
            );
        }
    }

    // structured trace
    inline void key(StringView key)
    {
        if (enable_structure_trace) [[unlikely]]
        {
            _current_key = key;
        }
    }
    inline void begin_array()
    {
        if (enable_structure_trace) [[unlikely]]
        {
            _structure_stack.push_back({ EStructureKind::Array, _current_key });
            _current_key.clear();
        }
    }
    inline void end_array()
    {
        if (enable_structure_trace) [[unlikely]]
        {
            SKR_ASSERT(!_structure_stack.is_empty() && _structure_stack.back().kind == EStructureKind::Array);
            _structure_stack.pop_back();
        }
    }
    inline void begin_object()
    {
        if (enable_structure_trace) [[unlikely]]
        {
            _structure_stack.push_back({ EStructureKind::Object, _current_key });
            _current_key.clear();
        }
    }
    inline void end_object()
    {
        if (enable_structure_trace) [[unlikely]]
        {
            SKR_ASSERT(!_structure_stack.is_empty() && _structure_stack.back().kind == EStructureKind::Object);
            _structure_stack.pop_back();
        }
    }

    // checkpoint trace
    inline bool checkpoint(std::source_location location)
    {
        if (any_error()) [[unlikely]]
        {
            format_to(
                _checkpoint_trace_message,
                u8"  at {}({})\n",
                location.file_name(),
                location.line()
            );
        }

        return !any_error();
    }

    // log
    template <typename... Args>
    inline void warn(StringView fmt, Args&&... args)
    {
        ++_warn_count;

        if (enable_checkpoint_trace)
        {
            // build warning string
            String warn_message = format(fmt, std::forward<Args>(args)...);
            warn_message.append(u8'\n');
            _dump_structure_trace(warn_message);
        }
        else
        {
            // dump warning directly
            String warn_message = format(fmt, std::forward<Args>(args)...);
            SKR_LOG_FMT_WARN(warn_message.c_str());
        }
    }
    template <typename... Args>
    inline void error(StringView fmt, Args&&... args)
    {
        ++_error_count;

        if (enable_checkpoint_trace)
        {
            format_to(
                _checkpoint_trace_message,
                fmt,
                std::forward<Args>(args)...
            );
            _checkpoint_trace_message.append(u8'\n');
        }
    }

    // reset
    inline void reset()
    {
        assume_succeeded_or_dump_error();

        _is_error_handled = false;
        _warn_count = 0;
        _error_count = 0;
        _checkpoint_trace_message.clear();
        _current_key.clear();
        _structure_stack.clear();
    }

    // config
    bool enable_structure_trace = false;
    bool enable_checkpoint_trace = true;

private:
    // helpers
    inline void _dump_structure_trace(String result)
    {
        // write current key
        if (!_current_key.is_empty())
        {
            format_to(result, u8"  at key '{}'\n", _current_key);
        }

        // write stack
        for (auto& stack : _structure_stack)
        {
            StringView kind_str = stack.kind == EStructureKind::Array ? u8"ARRAY" : u8"OBJECT";
            format_to(result, u8"  at key '{}' [{}]\n", stack.key, kind_str);
        }
    }

private:
    // state
    mutable bool _is_error_handled = false;

    // counters
    uint32_t _warn_count = 0;
    uint32_t _error_count = 0;

    // checkpoint trace
    String _checkpoint_trace_message = {};

    // structure trace
    String _current_key = {};
    Vector<StructureStack> _structure_stack = {};
};
} // namespace skr