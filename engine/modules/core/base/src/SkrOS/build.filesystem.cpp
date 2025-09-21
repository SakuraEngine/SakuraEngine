#include "SkrOS/filesystem.hpp"

namespace skr::fs
{

// 全局错误状态
Error g_last_error = Error::None;

Error get_last_error()
{
    return g_last_error;
}

void set_last_error(Error error)
{
    g_last_error = error;
}

// 时间戳转换函数已在头文件中定义为内联函数

// DirectoryEntry::get_info 实现
FileInfo DirectoryEntry::get_info() const
{
    return File::get_info(path);
}

// File 实例方法的通用实现
File::~File()
{
    close();
}

File::File(File&& other) noexcept
    : handle_(other.handle_), last_error_(other.last_error_)
{
    other.handle_ = nullptr;
    other.last_error_ = Error::None;
}

File& File::operator=(File&& other) noexcept
{
    if (this != &other)
    {
        close();
        handle_ = other.handle_;
        last_error_ = other.last_error_;
        other.handle_ = nullptr;
        other.last_error_ = Error::None;
    }
    return *this;
}

Error File::get_error() const
{
    return last_error_;
}

// 便利函数实现
bool File::read_all_bytes(const Path& path, skr::Vector<uint8_t>& out_data)
{
    File file;
    if (!file.open(path, OpenMode::ReadOnly))
        return false;
    
    int64_t file_size = file.get_size();
    if (file_size < 0)
        return false;
    
    if (file_size > 0)
    {
        out_data.resize_unsafe(static_cast<size_t>(file_size));
        size_t bytes_read = file.read(out_data.data(), static_cast<size_t>(file_size));
        if (bytes_read != static_cast<size_t>(file_size))
            return false;
    }
    else
    {
        out_data.clear();
    }
    
    return true;
}

bool File::read_all_text(const Path& path, skr::String& out_text)
{
    skr::Vector<uint8_t> data;
    if (!read_all_bytes(path, data))
        return false;
    
    out_text = skr::String(reinterpret_cast<const char8_t*>(data.data()), data.size());
    return true;
}

bool File::write_all_bytes(const Path& path, skr::Span<const uint8_t> data)
{
    File file;
    if (!file.open(path, OpenMode::Write | OpenMode::Create | OpenMode::Truncate))
        return false;
    
    size_t bytes_written = file.write(data.data(), data.size());
    return bytes_written == data.size();
}

bool File::write_all_text(const Path& path, skr::StringView text)
{
    skr::Span<const uint8_t> data(reinterpret_cast<const uint8_t*>(text.data()), text.size());
    return write_all_bytes(path, data);
}

// PathUtils 实现
Path PathUtils::absolute(const Path& p)
{
    if (p.is_absolute())
        return p;
    
    return Directory::current() / p;
}

Path PathUtils::canonical(const Path& p)
{
    // 将路径规范化：解析符号链接，移除 . 和 ..
    Path abs_path = absolute(p);
    
    // 使用 normalized 来处理 . 和 ..
    Path normalized = abs_path.normalize();
    
    // TODO: 在未来可以添加符号链接解析
    // 目前只返回规范化的绝对路径
    return normalized;
}

Path PathUtils::relative(const Path& p, const Path& base)
{
    return absolute(p).relative_to(absolute(base));
}

} // namespace skr::fs