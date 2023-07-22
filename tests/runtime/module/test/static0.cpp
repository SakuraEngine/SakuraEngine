#include "static0.hpp"

void SStaticModule0::on_load(int argc, char8_t** argv)
{
    SKR_LOG_INFO(u8"static module 0 loaded!");
}
void SStaticModule0::on_unload()
{
    SKR_LOG_INFO(u8"static module 0 unloaded!");
}
const char8_t* SStaticModule0::get_meta_data(void)
{
    return u8R"(
{
    "api" : "0.1.0",
    "name" : "static0",
    "prettyname" : "static0",
    "version" : "0.0.1",
    "linking" : "static",
    "dependencies" : [{"name":"dynamic0", "version":"0.0.1"}],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)";
}