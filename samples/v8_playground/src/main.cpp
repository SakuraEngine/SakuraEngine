#include <V8Playground/cmd_args.hpp>

int main(int argc, char* argv[])
{
    using namespace skr;

    // parse args
    CmdParser   parser;
    MainCommand cmd;
    parser.main_cmd(
        &cmd,
        {
            .help  = u8"V8 Playground",
            .usage = u8"V8Playground [options] [...js_files]",
        }
    );
    parser.parse(argc, argv);

    return 0;
}