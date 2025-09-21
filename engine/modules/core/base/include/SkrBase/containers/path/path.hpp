#pragma once
#include "SkrContainersDef/string.hpp"

namespace skr
{

struct Path
{
public:
    enum Style : uint8_t
    {
        None    = 0,
        Windows = 1 << 0,
        Unix    = 1 << 1,
#ifdef _WIN32
        Native = Windows,
#else
        Native = Unix,
#endif
        Universal = Windows | Unix
    };
    // constructors
    Path(uint8_t style = Universal);
    Path(const skr::String& str, uint8_t style = Universal);
    Path(skr::String&& str, uint8_t style = Universal);
    Path(const char8_t* str, uint8_t style = Universal);
    Path(skr::StringView str, uint8_t style = Universal);

    // path component extraction
    Path parent_directory() const;              // parent directory path
    Path filename() const;                      // file name with extension
    Path basename() const;                      // file name without extension (was stem)
    Path extension(bool with_dot = true) const; // file extension with or without dot

    // path manipulation
    Path& concat(const Path& p);
    Path& set_filename(const Path& filename);
    Path& set_extension(const Path& ext);
    Path& remove_extension();
    Path& replace_extension(const Path& replacement);
    Path& append(const Path& p);
    Path& join(const Path& p); // alias for append

    // path normalization
    Path normalize() const;
    Path relative_to(const Path& base) const;

    // query operations
    bool is_empty() const;
    bool is_absolute() const;
    bool is_relative() const;
    bool is_root() const;

    // component checks
    bool has_parent() const;
    bool has_filename() const;
    bool has_extension() const;

    // conversions
    const skr::String& string() const;
    const skr::String& to_string() const { return string(); }
    skr::String        native_string() const; // platform-specific separators

    // accessors
    uint8_t     style() const { return _style; }
    inline auto c_str() const { return _path.c_str(); }

    // operators
    Path& operator/=(const Path& p);
    Path& operator+=(const Path& p); // concat without separator

    Path operator/(const Path& p) const;
    Path operator+(const Path& p) const;

    bool operator==(const Path& p) const;
    bool operator!=(const Path& p) const;
    bool operator<(const Path& p) const;
    bool operator<=(const Path& p) const;
    bool operator>(const Path& p) const;
    bool operator>=(const Path& p) const;

private:
    void        normalize_separators();
    static bool is_absolute_path(skr::StringView path, uint8_t style);
    static bool is_root_component(skr::StringView component, uint8_t style);

    skr::String _path;
    uint8_t     _style;
};

template <>
struct Hash<Path>
{
    inline skr_hash operator()(const Path& x) const
    {
        return skr_hash_of(x.c_str(), x.string().size());
    }
    inline skr_hash operator()(const String& x) const
    {
        return skr_hash_of(x.c_str(), x.size());
    }
    inline skr_hash operator()(const StringView& x) const
    {
        return skr_hash_of(x.data(), x.size());
    }
    inline skr_hash operator()(const char* x) const
    {
        return skr_hash_of(x, std::strlen(x));
    }
    inline skr_hash operator()(const char8_t* x) const
    {
        return skr_hash_of(x, std::strlen(reinterpret_cast<const char*>(x)));
    }
};

} // namespace skr