#include <SkrV8/v8_script_loader.hpp>
#include <filesystem>

namespace skr
{
String V8ScriptLoaderSystemFS::normalize_path(String raw_path)
{
    raw_path.replace(u8"\\", u8"/");
    return raw_path;
}
Optional<String> V8ScriptLoaderSystemFS::load_script(String path)
{
    auto fs_path = std::filesystem::path{ path.c_str() };

    // check exists
    if (!std::filesystem::is_regular_file(fs_path))
    {
        SKR_LOG_ERROR(u8"script file not found: %s", path.c_str());
        return {};
    }

    // check is file
    if (!std::filesystem::is_regular_file(fs_path))
    {
        SKR_LOG_ERROR(u8"script file is not a regular file: %s", path.c_str());
        return {};
    }

    // open file
    auto f = fopen(fs_path.string().c_str(), "rb");
    if (!f)
    {
        SKR_LOG_ERROR(u8"failed to open script file: %s", path.c_str());
        return {};
    }

    // get file size
    fseek(f, 0, SEEK_END);
    auto size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // read file content
    String content;
    content.resize_unsafe(size);
    if (fread(content.data_w(), 1, size, f) != size)
    {
        SKR_LOG_ERROR(u8"failed to read script file: %s", path.c_str());
        fclose(f);
        return {};
    }
    fclose(f);

    // return content
    return content;
}
} // namespace skr