#include "skr_live2d/skr_live2d.h"

void SkrLive2DModule::on_load(int argc, char** argv)
{
    // nothing to do
}

void SkrLive2DModule::on_unload()
{
    // nothing to do
}

IMPLEMENT_DYNAMIC_MODULE(SkrLive2DModule, SkrLive2D);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrLive2D",
    "prettyname" : "SakuraLive2D",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrRenderer", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrLive2D)