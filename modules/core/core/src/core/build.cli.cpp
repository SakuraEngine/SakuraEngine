#include <SkrContainers/vector.hpp>
#include <SkrContainers/set.hpp>
#include <SkrContainers/string.hpp>
#include <SkrCore/cli.hpp>
#include <SkrCore/log.hpp>
#include <SkrContainers/optional.hpp>

// cmd option
namespace skr
{
bool CmdOptionData::init(attr::CmdOption config, TypeSignatureView type_sig, void* memory)
{
    TypeSignatureView raw_type_sig = type_sig;

    // setup type & optional
    if (type_sig.is_type())
    {
        // check decayed pointer
        if (type_sig.is_decayed_pointer())
        {
            SKR_LOG_FMT_ERROR(u8"option '{}' must be a value field!", config.name);
            return false;
        }

        // get type id
        GUID type_id;
        type_sig.jump_modifier();
        type_sig.read_type_id(type_id);

        // get type
        _kind = _solve_kind(type_id);
        if (_kind == EKind::None)
        {
            SKR_LOG_FMT_ERROR(u8"unknown type for option '{}'!", config.name);
            return false;
        }
    }
    else if (type_sig.is_generic_type())
    {
        // check decayed pointer
        if (type_sig.is_decayed_pointer())
        {
            SKR_LOG_FMT_ERROR(u8"option '{}' must be a value field!", config.name);
            return false;
        }

        // get generic id
        GUID     generic_id;
        uint32_t generic_param_count;
        type_sig.jump_modifier();
        type_sig = type_sig.read_generic_type_id(generic_id, generic_param_count);

        if (generic_id == kOptionalGenericId)
        { // process optional
            _is_optional = true;

            // get type id
            if (type_sig.is_decayed_pointer())
            {
                SKR_LOG_FMT_ERROR(u8"option '{}' T of Optional<T> must be a value field!", config.name);
                return false;
            }

            if (type_sig.is_type())
            { // read inner type id
                GUID type_id;
                type_sig.jump_modifier();
                type_sig.read_type_id(type_id);
                _kind = _solve_kind(type_id);
                if (_kind == EKind::None)
                {
                    SKR_LOG_FMT_ERROR(u8"unknown T of Optional<T> for option '{}'!", config.name);
                    return false;
                }
            }
            else
            {
                SKR_LOG_FMT_ERROR(u8"fuck '{}'!", raw_type_sig.to_string());
                SKR_LOG_FMT_ERROR(u8"unknown T of Optional<T> for option '{}'!", config.name);
                return false;
            }
        }
        else if (generic_id == kVectorGenericId)
        {
            // get type id
            if (type_sig.is_type() && !type_sig.is_decayed_pointer())
            {
                // get typeid
                GUID type_id;
                type_sig.jump_modifier();
                type_sig.read_type_id(type_id);

                // check type
                if (type_id == type_id_of<String>())
                {
                    _kind = EKind::StringVector;
                }
            }
            else
            {
                SKR_LOG_FMT_ERROR(u8"unknown T of Vector<T> for option '{}'!", config.name);
                return false;
            }
        }
        else
        {
            SKR_LOG_FMT_ERROR(u8"unknown generic type for option '{}'!", config.name);
            return false;
        }
    }
    else
    {
        SKR_LOG_FMT_ERROR(u8"unknown type for option '{}'!", config.name);
        return false;
    }

    // setup memory & config
    _memory = memory;
    _config = std::move(config);

    // check reset param kind
    if (_config.name == u8"..." && _kind != EKind::StringVector)
    {
        SKR_LOG_ERROR(u8"option '...' must be a string vector!");
        return false;
    }

    return true;
}

// setter
void CmdOptionData::set_value(bool v)
{
    SKR_ASSERT(_kind == EKind::Bool);
    _set<bool>(v);
}
void CmdOptionData::set_value(int64_t v)
{
    SKR_ASSERT(_kind == EKind::Int8 || _kind == EKind::Int16 || _kind == EKind::Int32 || _kind == EKind::Int64);
    switch (_kind)
    {
    case EKind::Int8:
        _set<int8_t>(static_cast<int8_t>(v));
        break;
    case EKind::Int16:
        _set<int16_t>(static_cast<int16_t>(v));
        break;
    case EKind::Int32:
        _set<int32_t>(static_cast<int32_t>(v));
        break;
    case EKind::Int64:
        _set<int64_t>(static_cast<int64_t>(v));
        break;
    default:
        SKR_UNREACHABLE_CODE()
        break;
    }
}
void CmdOptionData::set_value(uint64_t v)
{
    SKR_ASSERT(_kind == EKind::UInt8 || _kind == EKind::UInt16 || _kind == EKind::UInt32 || _kind == EKind::UInt64);
    switch (_kind)
    {
    case EKind::UInt8:
        _set<uint8_t>(static_cast<uint8_t>(v));
        break;
    case EKind::UInt16:
        _set<uint16_t>(static_cast<uint16_t>(v));
        break;
    case EKind::UInt32:
        _set<uint32_t>(static_cast<uint32_t>(v));
        break;
    case EKind::UInt64:
        _set<uint64_t>(static_cast<uint64_t>(v));
        break;
    default:
        SKR_UNREACHABLE_CODE()
        break;
    }
}
void CmdOptionData::set_value(double v)
{
    SKR_ASSERT(_kind == EKind::Float || _kind == EKind::Double);
    switch (_kind)
    {
    case EKind::Float:
        _set<float>(static_cast<float>(v));
        break;
    case EKind::Double:
        _set<double>(static_cast<double>(v));
        break;
    default:
        SKR_UNREACHABLE_CODE()
        break;
    }
}
void CmdOptionData::set_value(String v)
{
    SKR_ASSERT(_kind == EKind::String);
    if (_kind == EKind::String)
    {
        _set<String>(std::move(v));
    }
    else
    {
        SKR_UNREACHABLE_CODE()
    }
}
void CmdOptionData::add_value(String v)
{
    SKR_ASSERT(_kind == EKind::StringVector);
    if (_kind == EKind::StringVector)
    {
        _as<Vector<String>>().add(std::move(v));
    }
    else
    {
        SKR_UNREACHABLE_CODE()
    }
}

// helper
CmdOptionData::EKind CmdOptionData::_solve_kind(const GUID& type_id)
{
    switch (type_id.get_hash())
    {
    case type_id_of<bool>().get_hash():
        return EKind::Bool;
    case type_id_of<int8_t>().get_hash():
        return EKind::Int8;
    case type_id_of<int16_t>().get_hash():
        return EKind::Int16;
    case type_id_of<int32_t>().get_hash():
        return EKind::Int32;
    case type_id_of<int64_t>().get_hash():
        return EKind::Int64;
    case type_id_of<uint8_t>().get_hash():
        return EKind::UInt8;
    case type_id_of<uint16_t>().get_hash():
        return EKind::UInt16;
    case type_id_of<uint32_t>().get_hash():
        return EKind::UInt32;
    case type_id_of<uint64_t>().get_hash():
        return EKind::UInt64;
    case type_id_of<float>().get_hash():
        return EKind::Float;
    case type_id_of<double>().get_hash():
        return EKind::Double;
    case type_id_of<String>().get_hash():
        return EKind::String;
    };
    return EKind::None;
}
} // namespace skr

// cmd node
namespace skr
{
// dtor
CmdNode::~CmdNode()
{
    for (auto& sub_cmd : sub_cmds)
    {
        SkrDelete(sub_cmd);
    }
}

// solve cmd
bool CmdNode::solve()
{
    bool failed = false;

    // solve options & sub commands
    type->each_field([&](const RTTRFieldData* field, const RTTRType* owner_type) {
        // find option
        auto found_option_attr_ref = field->attrs.find_if([&](const Any& v) { return v.type_is<attr::CmdOption>(); });
        if (found_option_attr_ref)
        {
            // solve config
            attr::CmdOption option_attr = found_option_attr_ref.ref().as<attr::CmdOption>();
            option_attr.name            = (!option_attr.name.is_empty()) ? option_attr.name : field->name;

            // get field address
            void* owner_obj     = type->cast_to_base(owner_type->type_id(), object);
            void* field_address = field->get_address(owner_obj);

            // add option
            auto& new_option = options.add_default().ref();
            failed |= !new_option.init(std::move(option_attr), field->type, field_address);
            return;
        }

        // find sub command
        auto found_sub_cmd_attr_ref = field->attrs.find_if([&](const Any& v) { return v.type_is<attr::CmdSub>(); });
        if (found_sub_cmd_attr_ref)
        {
            // solve config
            attr::CmdSub sub_cmd_attr = found_sub_cmd_attr_ref.ref().as<attr::CmdSub>();
            sub_cmd_attr.name         = (!sub_cmd_attr.name.is_empty()) ? sub_cmd_attr.name : field->name;
            auto field_type_sig       = field->type.view();

            // check signature
            if (!field_type_sig.is_type())
            {
                SKR_LOG_ERROR(u8"sub command must be a type!");
                return;
            }
            if (field_type_sig.is_decayed_pointer())
            {
                SKR_LOG_ERROR(u8"sub command must be a value field!");
                return;
            }

            // get type
            GUID type_id;
            field_type_sig.jump_modifier();
            field_type_sig.read_type_id(type_id);
            const RTTRType* sub_cmd_type = get_type_from_guid(type_id);
            if (!sub_cmd_type)
            {
                SKR_LOG_ERROR(u8"sub command type not found!");
                return;
            }

            // get field address
            void* owner_obj     = type->cast_to_base(owner_type->type_id(), object);
            void* field_address = field->get_address(owner_obj);

            // create sub command
            auto sub_cmd = SkrNew<CmdNode>();

            // setup basic info
            sub_cmd->config = std::move(sub_cmd_attr);
            sub_cmd->type   = sub_cmd_type;
            sub_cmd->object = field_address;

            // solve sub cmd
            failed |= !sub_cmd->solve();

            // register sub command
            sub_cmds.push_back(sub_cmd);
            return;
        }

        // clang-format off
    }, { .include_bases = true });
    // clang-format on

    // find exec method
    type->each_method([&](const RTTRMethodData* method, const RTTRType*) {
        // find exec
        auto found_exec_attr_ref = method->attrs.find_if([&](const Any& v) { return v.type_is<attr::CmdExec>(); });
        if (found_exec_attr_ref)
        {
            // check signature
            TypeSignatureTyped<void()> sig;
            if (!method->signature_equal(sig.view(), ETypeSignatureCompareFlag::Strict))
            {
                SKR_LOG_ERROR(u8"exec method must be a void() method!");
                return;
            }

            // setup exec method
            exec_method = ExportMethodInvoker<void()>(method);
        }
        // clang-format off
    }, { .include_bases = false });
    // clang-format on

    failed |= check_options();
    failed |= check_sub_commands();
    return !failed;
}
bool CmdNode::check_options()
{
    Map<String, CmdOptionData*>    seen_options;
    Map<skr_char8, CmdOptionData*> seen_short_options;
    bool                           no_error = true;

    for (auto& option : options)
    {
        // check name
        if (auto found = seen_options.find(option.config().name))
        {
            SKR_LOG_FMT_ERROR(u8"duplicate option name: {}", option.config().name);
            no_error = false;
        }
        seen_options.add(option.config().name, &option);

        // check short name
        if (option.config().short_name != 0)
        {
            if (auto found = seen_short_options.find(option.config().short_name))
            {
                SKR_LOG_FMT_ERROR(u8"duplicate option short name: {}", option.config().short_name);
                no_error = false;
            }
            seen_short_options.add(option.config().short_name, &option);
        }
    }

    return no_error;
}
bool CmdNode::check_sub_commands()
{
    Map<String, CmdNode*>    sub_cmd_map;
    Map<skr_char8, CmdNode*> sub_cmd_short_map;

    bool no_error = true;

    for (auto& sub_cmd : sub_cmds)
    {
        // check name
        if (auto found = sub_cmd_map.find(sub_cmd->config.name))
        {
            SKR_LOG_FMT_ERROR(u8"duplicate sub command name: {}", sub_cmd->config.name);
            no_error = false;
        }
        sub_cmd_map.add(sub_cmd->config.name, sub_cmd);

        // check short name
        if (auto found = sub_cmd_short_map.find(sub_cmd->config.short_name))
        {
            SKR_LOG_FMT_ERROR(u8"duplicate sub command short name: {}", sub_cmd->config.short_name);
            no_error = false;
        }
        sub_cmd_short_map.add(sub_cmd->config.short_name, sub_cmd);
    }

    return no_error;
}
void CmdNode::print_help()
{
    CliOutputBuilder builder;

    // print self help
    builder.line(config.help);

    // print self usage
    builder
        .style_bold()
        .write(u8"usage: ")
        .style_clear()
        .style_front_blue()
        .write(config.usage)
        .style_clear()
        .next_line();

    if (!options.is_empty())
    {
        builder
            .style_bold()
            .line(u8"options:")
            .style_clear();
        uint64_t max_option_name_length = 0;
        for (auto& option : options)
        {
            uint64_t len = 0;
            len += 4;                                        // '-X, '
            len += 2 + option.config().name.length_buffer(); // '--XXXX'
            max_option_name_length = std::max(max_option_name_length, len);
        }
        for (auto& option : options)
        {
            String option_str;

            if (option.config().short_name != 0)
            {
                format_to(option_str, u8"-{}, --{}", option.config().short_name, option.config().name);
            }
            else
            {
                format_to(option_str, u8"    --{}", option.config().name);
            }
            option_str.pad_right(max_option_name_length);

            builder
                .write(u8"  ")
                // write type
                .style_front_yellow()
                .write(u8"{} ", option.dump_type_name())
                .style_clear()
                // write option
                .style_bold()
                .style_front_green()
                .write(u8"{} : ", option_str)
                .style_clear()
                // write help
                .style_clear()
                .write_indent(u8"{}{}", (option.is_required() ? "[REQUIRED] " : ""), option.config().help)
                .style_clear()
                .next_line();
        }
    }

    // print subcommands
    if (!sub_cmds.is_empty())
    {
        builder
            .style_bold()
            .write(u8"sub commands:")
            .style_clear()
            .next_line();
        uint64_t max_sub_cmd_name_length = 0;
        for (auto& sub_cmd : sub_cmds)
        {
            uint64_t len = 0;
            len += 3;                                    // 'x, '
            len += sub_cmd->config.name.length_buffer(); // 'XXXX'
            max_sub_cmd_name_length = std::max(max_sub_cmd_name_length, len);
        }
        for (auto& sub_cmd : sub_cmds)
        {
            String sub_cmd_str;
            if (sub_cmd->config.short_name != 0)
            {
                format_to(sub_cmd_str, u8"{}, {}", sub_cmd->config.short_name, sub_cmd->config.name);
            }
            else
            {
                format_to(sub_cmd_str, u8"   {}", sub_cmd->config.name);
            }
            sub_cmd_str.pad_left(max_sub_cmd_name_length);

            builder
                .style_bold()
                .style_front_cyan()
                .write(u8"  {} : ", sub_cmd_str)
                .style_clear()
                .write(sub_cmd->config.help)
                .next_line();
        }
    }

    // output
    builder.dump();
}
} // namespace skr

// cmd parser
namespace skr
{
// ctor & dtor
CmdParser::CmdParser()
{
}
CmdParser::~CmdParser()
{
}

// register command
void CmdParser::main_cmd(const RTTRType* type, void* object, attr::CmdSub config)
{
    if (_root_cmd.object != nullptr)
    {
        SKR_LOG_ERROR(u8"main command has registered!");
        return;
    }

    // fill main command info
    _root_cmd.config = config;
    _root_cmd.type   = type;
    _root_cmd.object = object;
    _root_cmd.solve();
}
void CmdParser::sub_cmd(const RTTRType* type, void* object, attr::CmdSub config)
{
    // fill sub command info
    auto* sub_cmd   = SkrNew<CmdNode>();
    sub_cmd->config = config;
    sub_cmd->type   = type;
    sub_cmd->object = object;
    sub_cmd->solve();
    _root_cmd.sub_cmds.push_back(sub_cmd);
    _root_cmd.check_sub_commands();
}

// parse
void CmdParser::parse(int argc, char* argv[])
{
    // parse args
    String           program_name = String::From(argv[0]);
    Vector<CmdToken> args         = {};
    args.reserve(argc);
    {
        CliOutputBuilder builder;
        bool             any_error = false;
        for (int i = 1; i < argc; ++i)
        {
            auto& arg = args.add_default().ref();
            any_error |= !arg.parse(StringView{ reinterpret_cast<const skr_char8*>(argv[i]) }, builder);
        }
        if (any_error)
        {
            builder.dump();
            _root_cmd.print_help();
            return;
        }
    }

    uint32_t current_idx = 0;

    // solve sub command
    CmdNode* current_cmd = &_root_cmd;
    while (current_idx < args.size() && args[current_idx].type == CmdToken::Type::Name)
    {
        const auto& token = args[current_idx].token;
        auto        found = current_cmd->sub_cmds.find_if([&](const CmdNode* cmd) {
            String short_name;
            short_name.add(cmd->config.short_name);
            return short_name == token || cmd->config.name == token;
        });
        if (found)
        {
            current_cmd = found.ref();
            ++current_idx;
        }
        else
        { // support command like 'run.exe test.js'
            break;
            // CliOutputBuilder builder;
            // builder
            //     .style_bold()
            //     .style_front_red()
            //     .write(u8"unknown sub command: ")
            //     .style_clear()
            //     .style_front_cyan()
            //     .write(token)
            //     .style_clear()
            //     .next_line();
            // builder.dump();
            // _root_cmd.print_help();
            // return;
        }
    }

    // help command
    {
        auto found = args.find_if([&](const CmdToken& token) {
            return (token.type == CmdToken::Type::ShortOption && token.token == u8"h") ||
                   (token.type == CmdToken::Type::Option && token.token == u8"help");
        });

        if (found)
        {
            // dump ignored args
            if (args.size() - current_idx > 1)
            {
                CliOutputBuilder builder;
                builder.style_bold().style_front_yellow().write(u8"these args will be ignored:").style_clear().next_line();
                for (uint64_t i = current_idx; i < args.size(); ++i)
                {
                    if (i == found.index()) continue;
                    builder.line(u8"  {}", args[i].raw());
                }
                builder.dump();
            }

            // dump help
            current_cmd->print_help();
            return;
        }
    }

    // collect options info
    Set<CmdOptionData*>            required_options;
    Map<String, CmdOptionData*>    options_map;
    Map<skr_char8, CmdOptionData*> short_options_map;
    for (auto& option : current_cmd->options)
    {
        if (option.is_required())
        {
            required_options.add(&option);
        }
        options_map.add(option.config().name, &option);
        if (option.config().short_name != 0)
        {
            short_options_map.add(option.config().short_name, &option);
        }
    }

    // parse options
    CliOutputBuilder builder;
    bool             any_error = false;
    while (current_idx < args.size())
    {
        const auto& arg = args[current_idx];

        if (arg.is_any_option())
        { // process option
            auto params = _find_option_param_pack(args, current_idx);

            // find option
            if (arg.is_option())
            {
                auto* found_option = options_map.find(arg.token).value_or(nullptr);
                if (!found_option)
                { // unknown option will take all params
                    _error_unknown_option(builder, arg);
                    any_error = true;
                    current_idx += 1 + params.size();
                    continue;
                }
                any_error |= !_process_option(
                    found_option,
                    current_idx,
                    arg,
                    params,
                    builder,
                    required_options
                );
            }
            else if (arg.is_short_option())
            {
                if (arg.token.length_buffer() == 1)
                {
                    auto* found_option = short_options_map.find(arg.token.at_buffer(0)).value_or(nullptr);
                    if (!found_option)
                    { // unknown option will take all params
                        _error_unknown_option(builder, arg);
                        any_error = true;
                        current_idx += 1 + params.size();
                        continue;
                    }

                    any_error |= !_process_option(
                        found_option,
                        current_idx,
                        arg,
                        params,
                        builder,
                        required_options
                    );
                }
                else
                { // process compound short options
                    for (uint32_t i = 0; i < arg.token.length_buffer(); ++i)
                    {
                        auto  short_name   = arg.token.at_buffer(i);
                        auto* found_option = short_options_map.find(short_name).value_or(nullptr);
                        if (!found_option)
                        {
                            _error_unknown_option(builder, arg);
                            any_error = true;
                            continue;
                        }
                        if (!found_option->is_bool())
                        {
                            builder
                                // error body
                                .style_bold()
                                .style_front_red()
                                .write(u8"option '")
                                .style_clear()
                                // option name
                                .style_front_green()
                                .write(u8"-{}", short_name)
                                .style_clear()
                                // error tail
                                .style_bold()
                                .style_front_red()
                                .write(u8"' cannot used in compound short options!")
                                .style_clear()
                                .next_line();
                            any_error = true;
                            continue;
                        }

                        uint32_t dummy_idx = current_idx;
                        any_error |= !_process_option(
                            found_option,
                            dummy_idx,
                            CmdToken{ CmdToken::Type::ShortOption, short_name },
                            params,
                            builder,
                            required_options
                        );
                    }

                    current_idx += 1; // skip short option
                }
            }
        }
        else
        { // trigger end of options, parse rest options
            // get option
            auto* found_option = options_map.find(u8"...").value_or(nullptr);
            if (!found_option)
            {
                builder
                    .style_bold()
                    .style_front_red()
                    .write(u8"rest params is not supported!")
                    .style_clear()
                    .next_line();
                any_error = true;
                break;
            }

            // add params
            auto params = _find_rest_params_pack(args, current_idx);
            for (const auto& param : params)
            {
                found_option->add_value(param.token);
            }

            required_options.remove(found_option);
            current_idx += params.size();
            break;
        }
    }

    // check required options
    if (!required_options.is_empty())
    {
        builder
            .style_bold()
            .style_front_red()
            .write(u8"missing required options:")
            .style_clear()
            .next_line();
        uint32_t max_option_name_length = 0;
        for (auto& option : required_options)
        {
            uint32_t len = 0;
            len += 4;                                         // '-X, '
            len += 2 + option->config().name.length_buffer(); // '--XXXX'
            max_option_name_length = std::max(max_option_name_length, len);
        }
        for (auto& option : required_options)
        {
            String option_str;
            if (option->config().short_name != 0)
            {
                format_to(option_str, u8"-{}, --{}", option->config().short_name, option->config().name);
            }
            else
            {
                format_to(option_str, u8"    --{}", option->config().name);
            }
            option_str.pad_right(max_option_name_length);

            builder
                .write(u8"  ")
                // write type
                .style_front_yellow()
                .write(u8"{} ", option->dump_type_name())
                .style_clear()
                // write options
                .style_bold()
                .style_front_green()
                .write(u8"{} : ", option_str, option->dump_type_name())
                .style_clear()
                // write help
                .write_indent(option->config().help)
                .next_line();
        }
        builder.dump();
        current_cmd->print_help();
        return;
    }

    // dump ignored options
    if (current_idx != args.size())
    {
        builder
            .style_bold()
            .style_front_yellow()
            .write(u8"these args will be ignored:")
            .style_clear()
            .next_line();
        for (uint64_t i = current_idx; i < args.size(); ++i)
        {
            builder.line(u8"  {}", args[i].raw());
        }
        builder.dump();
    }
    else
    {
        if (any_error)
        {
            builder.dump();
        }
    }

    // block execute before exec
    if (any_error)
    {
        current_cmd->print_help();
        return;
    }

    // invoke command
    if (current_cmd->exec_method.is_valid())
    {
        current_cmd->exec_method.invoke(current_cmd->object);
    }
}
// helper
span<const CmdToken> CmdParser::_find_option_param_pack(const Vector<CmdToken>& args, uint64_t option_idx)
{
    // trigger end
    if (option_idx == args.size() - 1)
    {
        return {};
    }

    // next token is option
    {
        const auto& next_token = args[option_idx + 1];
        if (next_token.type == CmdToken::Type::Option || next_token.type == CmdToken::Type::ShortOption)
        {
            return {};
        }
    }

    // find option pack
    uint64_t search_idx = option_idx + 1;
    while (search_idx < args.size())
    {
        const auto& token = args[search_idx];
        if (token.type == CmdToken::Type::Option || token.type == CmdToken::Type::ShortOption)
        {
            break;
        }
        ++search_idx;
    }
    return args.span().subspan(option_idx + 1, search_idx - option_idx - 1);
}
span<const CmdToken> CmdParser::_find_rest_params_pack(const Vector<CmdToken>& args, uint64_t name_idx)
{
    uint64_t search_idx = name_idx;
    while (search_idx < args.size())
    {
        const auto& token = args[search_idx];
        if (token.type == CmdToken::Type::Option || token.type == CmdToken::Type::ShortOption)
        {
            break;
        }
        ++search_idx;
    }
    return args.span().subspan(name_idx, search_idx - name_idx);
}
void CmdParser::_error_require_params(CliOutputBuilder& builder, const CmdToken& arg)
{
    builder
        // error head
        .style_bold()
        .style_front_red()
        .write(u8"option '")
        .style_clear()
        // option name
        .style_front_green()
        .write(arg.raw())
        .style_clear()
        // error tail
        .style_bold()
        .style_front_red()
        .write(u8"' requires at least one parameter!")
        .style_clear()
        .next_line();
}
void CmdParser::_error_parse_params(CliOutputBuilder& builder, const CmdToken& arg, StringView param, StringView type)
{
    builder
        // error body
        .style_bold()
        .style_front_red()
        .write(u8"failed to parse parameter '")
        .style_clear()
        // parameter
        .write(param)
        // error body
        .style_bold()
        .style_front_red()
        .write(u8"' for option '")
        .style_clear()
        // option name
        .style_front_green()
        .write(arg.raw())
        .style_clear()
        // error body
        .style_bold()
        .style_front_red()
        .write(u8"' as '")
        .style_clear()
        // type
        .style_front_yellow()
        .write(type)
        .style_clear()
        // error body
        .style_bold()
        .style_front_red()
        .write(u8"'!")
        .style_clear()
        .next_line();
}
void CmdParser::_error_unknown_option(CliOutputBuilder& builder, const CmdToken& arg)
{
    builder
        .style_bold()
        .style_front_red()
        .write(u8"unknown option: ")
        .style_clear()
        .style_front_green()
        .write(arg.raw())
        .style_clear()
        .next_line();
}
bool CmdParser::_process_option(
    CmdOptionData*       found_option,
    uint32_t&            current_idx,
    const CmdToken&      arg,
    span<const CmdToken> params,
    CliOutputBuilder&    builder,
    Set<CmdOptionData*>& required_options
)
{
    // check option type
    if (found_option->is_bool())
    { // boolean option will not consume any params
        found_option->set_value(true);
        required_options.remove(found_option);
        ++current_idx;
        return true;
    }
    else if (found_option->is_string_vector())
    { // string vector option will consume all params
        // check required
        if (params.is_empty() && found_option->is_required())
        {
            _error_require_params(builder, arg);
            ++current_idx;
            return false;
        }

        // set value
        for (auto& param : params)
        {
            found_option->add_value(param.token);
        }
        required_options.remove(found_option);
        current_idx += 1 + params.size();
        return true;
    }
    else
    { // normal value case
        // check require
        if (params.is_empty() && found_option->is_required())
        {
            _error_require_params(builder, arg);
            ++current_idx;
            return false;
        }

        // get param
        auto param = params[0];

        // try set value
        if (found_option->is_int())
        {
            auto parsed_result = param.token.parse_int();
            if (!parsed_result.is_success())
            {
                _error_parse_params(builder, arg, param.token, u8"int");
                ++current_idx;
                return false;
            }
            found_option->set_value(parsed_result.value);
        }
        else if (found_option->is_uint())
        {
            auto parsed_result = param.token.parse_uint();
            if (!parsed_result.is_success())
            {
                _error_parse_params(builder, arg, param.token, u8"uint");
                ++current_idx;
                return false;
            }
            found_option->set_value(parsed_result.value);
        }
        else if (found_option->is_float())
        {
            auto parsed_result = param.token.parse_float();
            if (!parsed_result.is_success())
            {
                _error_parse_params(builder, arg, param.token, u8"float");
                ++current_idx;
                return false;
            }
            found_option->set_value(parsed_result.value);
        }
        else if (found_option->is_string())
        {
            found_option->set_value(param.token);
        }

        // pass and jump
        required_options.remove(found_option);
        current_idx += 2;
        return true;
    }
}
} // namespace skr