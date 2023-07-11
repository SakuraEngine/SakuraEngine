#pragma once
#include "containers/vector.hpp"
#include "platform/debug.h"
#include "misc/log.h"
#include "containers/string.hpp"
#include "containers/hashmap.hpp"
#include "misc/fast_float.h"
#include <charconv>
#include <optional>

namespace skr::cmd
{

struct parser {
    parser(int argc, char** argv)
        : m_argc(argc)
        , m_argv(argv)
        , m_required(0)
    {
    }

    struct cmd {
        skr::string shorthand, value, descr;
        bool is_required, is_boolean;
    };

    bool parse()
    {
        SKR_ASSERT(m_argc > 0);
        if (m_argc - 1 < m_required) return abort();

        int num_required = 0;
        skr::flat_hash_set<skr::string, skr::hash<skr::string>> parsed_shorthands;
        parsed_shorthands.reserve(m_argc);

        for (int i = 1; i != m_argc; ++i)
        {
            skr::string parsed((char8_t*)m_argv[i]);
            if (parsed.view() == u8"-h" || parsed == u8"--help") return abort();
            int id = 0;
            if (const auto it = m_shorthands.find(parsed); it == m_shorthands.end())
            {
                SKR_LOG_ERROR("shorthand '%s' not found", parsed.c_str());
                return abort();
            }
            else
            {
                if (const auto it = parsed_shorthands.find(parsed); it != parsed_shorthands.end())
                {
                    SKR_LOG_ERROR("shorthand '%s' already parsed", parsed.c_str());
                    return abort();
                }
                parsed_shorthands.emplace(parsed);
                id = (*it).second;
            }
            assert(static_cast<size_t>(id) < m_names.size());
            auto const& name = m_names[id];
            auto& cmd = m_cmds[name];
            if (cmd.is_required) num_required += 1;
            if (cmd.is_boolean)
            {
                parsed = u8"true";
            }
            else
            {
                ++i;
                if (i == m_argc) { return abort(); }
                parsed = (char8_t*)m_argv[i];
            }
            cmd.value = parsed;
        }

        if (num_required != m_required) return abort();

        return true;
    }

    void help() const
    {
        SKR_LOG_WARN("Usage: %s [-h,--help]", m_argv[0]);
        const auto print = [this](bool with_description) {
            for (size_t i = 0; i != m_names.size(); ++i)
            {
                auto const& cmd = m_cmds.at(m_names[i]);
                SKR_LOG_WARN(" [%s%s]", cmd.shorthand.c_str(), cmd.is_boolean ? m_names[i].c_str() : " <value>");
                if (with_description)
                {
                    SKR_LOG_WARN("%s%s", cmd.descr.c_str(), cmd.is_required ? " (required)" : "");
                }
            }
        };
        print(false);
        print(true);
        SKR_LOG_WARN(" [-h,--help]\n\tPrint this help text and silently exits.");
    }

    bool add(skr::string const& name, skr::string const& descr, skr::string const& shorthand,
    bool is_required, bool is_boolean = false)
    {
        bool ret = m_cmds
                   .emplace(name, cmd{ shorthand, is_boolean ? u8"false" : u8"", descr, is_required,
                                  is_boolean })
                   .second;
        if (ret)
        {
            if (is_required) m_required += 1;
            m_names.push_back(name);
            m_shorthands.emplace(shorthand, m_names.size() - 1);
        }
        return ret;
    }

    template <typename T>
    T get(skr::string const& name) const
    {
        auto it = m_cmds.find(name);
        if (it == m_cmds.end())
        {
            SKR_LOG_ERROR("'%s' not found", name.c_str());
            //throw std::runtime_error(std::string("error: '") + name.c_str() + "' not found");
        }
        auto const& value = (*it).second.value;
        return parse<T>(value);
    }

    template <typename T>
    std::optional<T> get_optional(skr::string const& name) const
    {
        auto it = m_cmds.find(name);
        if (it == m_cmds.end()) return std::nullopt;
        auto const& value = (*it).second.value;
        return parse<T>(value);
    }

    bool parsed(skr::string const& name) const
    {
        auto it = m_cmds.find(name);
        if (it == m_cmds.end()) return false;
        auto const& cmd = (*it).second;
        if (cmd.is_boolean)
        {
            if (cmd.value == u8"true") return true;
            if (cmd.value == u8"false") return false;
            SKR_UNREACHABLE_CODE()
        }
        return cmd.value != u8"";
    }

    template <typename T>
    T parse(skr::string const& value) const
    {
        if constexpr (std::is_same<T, skr::string>::value)
        {
            return value;
        }
        else if constexpr (std::is_same<T, unsigned int>::value || std::is_same<T, int>::value ||
                           std::is_same<T, unsigned short int>::value ||
                           std::is_same<T, short int>::value || std::is_same<T, unsigned long int>::value ||
                           std::is_same<T, long int>::value ||
                           std::is_same<T, unsigned long long int>::value ||
                           std::is_same<T, long long int>::value)
        {
            T result;
            auto ret = std::from_chars(value.c_str(), value.c_str() + value.size(), result);
            if (ret.ec != 0)
            {
                auto name = typeid(T).name();
                SKR_LOG_ERROR("failed to parse '%s' as '%s'", value.c_str(), name);
                //throw std::runtime_error(std::string("error: failed to parse '") + value.c_str() + "' as '" + name + "'");
            }
            return result;
        }
        else if constexpr (std::is_same<T, float>::value || std::is_same<T, double>::value ||
                           std::is_same<T, long double>::value)
        {
            T result;
            auto ret = fast_float::from_chars(value.c_str(), value.c_str() + value.size(), result);
            if (ret.ec != 0)
            {
                auto name = typeid(T).name();
                SKR_LOG_ERROR("failed to parse '%s' as '%s'", value.c_str(), name);
                //throw std::runtime_error(std::string("error: failed to parse '") + value.c_str() + "' as '" + name + "'");
            }
            return result;
        }
        else if constexpr (std::is_same<T, bool>::value)
        {
            if (value == u8"true" || value == u8"1" || value == u8"yes" || value == u8"on")
            {
                return true;
            }
            else if (value == u8"false" || value == u8"0" || value == u8"no" || value == u8"off")
            {
                return false;
            }
            return false;
        }
        SKR_UNREACHABLE_CODE(); // should never happen
        //throw std::runtime_error("unsupported type");
        return {};
    }

private:
    int m_argc;
    char** m_argv;
    int m_required;
    skr::flat_hash_map<skr::string, cmd, skr::hash<skr::string>> m_cmds;
    skr::flat_hash_map<skr::string, int, skr::hash<skr::string>> m_shorthands;
    skr::vector<skr::string> m_names;

    bool abort() const
    {
        help();
        return false;
    }
};

} // namespace cmd_line_parser