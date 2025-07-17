#pragma once
#include <SkrContainers/string.hpp>
#include <SkrContainers/vector.hpp>
#include <SkrBase/config.h>
#include <SkrBase/meta.h>
#include <SkrCore/cli.hpp>
#include <V8Playground/app.hpp>
#ifndef __meta__
    #include "V8Playground/cmd_args.generated.h"
#endif

namespace skr
{
sreflect_struct(guid = "c30115c5-4945-468f-a579-523202d4f42b" rttr = @full)
SubCommandDumpDefine {
    srttr_attr(CmdOption{
        .short_name = u8'o',
        .help       = u8"where to output .d.ts file",
    })
    skr::String outdir = {};

    srttr_attr(CmdExec{}) void exec()
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

sreflect_struct(guid = "1bfb194d-abcd-42c0-9597-96f359a786aa" rttr = @full)
MainCommand {
    srttr_attr(CmdOption{
        .name        = u8"...",
        .help        = u8"js files to execute",
        .is_required = false,
    })
    skr::Vector<skr::String> exec_files = {};

    srttr_attr(CmdSub{
        .short_name = u8'd',
        .help       = u8"dump .d.ts files",
        .usage      = u8"V8Playground dump_def [options]",
    })
    SubCommandDumpDefine dump_def = {};

    srttr_attr(CmdExec{}) void exec()
    {
        V8PlaygroundApp::env_init();

        V8PlaygroundApp app;
        
        // init
        app.init();
        app.load_native_types();

        // wait for debugger
        app.init_debugger(9865);
        app.wait_for_debugger_connected();
        
        // run scripts
        bool any_failed = false;
        for (auto& file : exec_files)
        {
            any_failed |= !app.run_script(file);
        }
        if (any_failed)
        {
            exit(1);
        }
        
        // shutdown
        app.shutdown_debugger();
        app.shutdown();

        V8PlaygroundApp::env_shutdown();

        exit(0);
    }
};
} // namespace skr