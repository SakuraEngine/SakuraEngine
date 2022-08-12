#include "utils/log.h"
#include "platform/memory.h"
#include "skr_image_coder/skr_image_coder.h"
#include "libpng/png.h"

void SkrImageCoderModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("image coder module loaded!");
    auto png_ptr = png_create_read_struct(png_get_libpng_ver(nullptr), nullptr, nullptr, nullptr);
    (void)png_ptr;
}

void SkrImageCoderModule::on_unload()
{
    SKR_LOG_INFO("image coder module unloaded!");
}

IMPLEMENT_DYNAMIC_MODULE(SkrImageCoderModule, SkrImageCoder);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrImageCoder",
    "prettyname" : "SakuraImageCoder",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrRT", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrImageCoder)