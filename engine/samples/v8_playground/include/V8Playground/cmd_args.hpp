#pragma once
#include <SkrContainers/string.hpp>
#include <SkrContainers/vector.hpp>
#include <SkrBase/config.h>
#include <SkrBase/meta.h>
#include <SkrCore/cli.hpp>
#include <V8Playground/app.hpp>
#include "V8Playground/cmd_args.generated.h"

namespace skr
{
struct [[sattr(
    guid = "c30115c5-4945-468f-a579-523202d4f42b"
    rttr = @full
)]] SubCommandDumpDefine
{
    [[srttr_attr(CmdOption{
        .short_name = u8'o',
        .help = u8"where to output .d.ts file",
    })]]
    skr::String outdir = {};

    [[srttr_attr(CmdExec{})]]
    void exec()
    {
        SKR_LOG_FMT_INFO(u8"dump .d.ts into dir '{}'", outdir);
        V8PlaygroundApp::env_init();

        V8PlaygroundApp app;
        app.init();
        app.load_native_types();
        if (!app.dump_types(outdir))
        {
            SKR_LOG_FMT_ERROR(u8"Failed to dump types to dir: {}", outdir);
            exit(1);
        }
        app.shutdown();

        V8PlaygroundApp::env_shutdown();

        exit(0);
    }
};

struct [[sattr(guid = "1bfb194d-abcd-42c0-9597-96f359a786aa" rttr = @full)]]
MainCommand
{
    [[srttr_attr(CmdOption{
        .short_name = u8'r',
        .name = u8"root",
        .help = u8"js project root",
        .is_required = true,
    })]]
    skr::String js_root = {};

    [[srttr_attr(CmdSub{
        .short_name = u8'd',
        .help = u8"dump .d.ts files",
        .usage = u8"V8Playground dump_def [options]",
    })]] 
    SubCommandDumpDefine dump_def = {};

    [[srttr_attr(CmdExec{})]] 
    void exec()
    {
        V8PlaygroundApp::env_init();

        V8PlaygroundApp app;

        // init
        app.init();
        app.load_native_types();
        app.setup_vfs(js_root);

        // wait for debugger
        // app.init_debugger(9865);
        // app.wait_for_debugger_connected();

        // run scripts
        app.run_script(u8"main.js");

        // shutdown
        app.shutdown();

        V8PlaygroundApp::env_shutdown();

        exit(0);
    }
};
} // namespace skr