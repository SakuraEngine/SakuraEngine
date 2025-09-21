#pragma once
#include <SkrCore/cli.hpp>
#include "WebSocketSample/web_socket_cmds.generated.h"

namespace skr
{
struct [[sattr(guid = "57dddc72-2aaf-49b1-bb72-031fe66099ae" rttr = @full)]]
WebSocketSampleCli
{
    [[srttr_attr(CmdOption{
        .short_name = u8'k',
        .help       = u8"kind of program, can be 'client' or 'server'",
    })]]
    String kind;

    [[srttr_attr(CmdOption{
        .short_name = u8'g',
        .help       = u8"group name",
        .is_required = false,
    })]]
    String group;

    [[srttr_attr(CmdExec{})]]
    void exec();
};
} // namespace skr