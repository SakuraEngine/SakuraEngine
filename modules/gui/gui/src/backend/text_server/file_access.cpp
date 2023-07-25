#include "file_access.h"
#include "ustring.h"

namespace godot
{
static skr_vfs_t* godot_vfs = nullptr;\
void FileAccess::set_vfs(skr_vfs_t* vfs)
{
    godot_vfs = vfs;
}
Ref<FileAccess> FileAccess::open(const String &p_path, int p_mode_flags, Error *r_error)
{
    FileAccess *fa = SkrNew<FileAccess>();
    ESkrFileMode mod;
    ESkrFileCreation cr = ESkrFileCreation::SKR_FILE_CREATION_OPEN_EXISTING;
    switch (p_mode_flags) {
        case READ:
            mod = ESkrFileMode::SKR_FM_READ;
            break;
        case WRITE:
            mod = ESkrFileMode::SKR_FM_WRITE;
            break;
        case READ_WRITE:
            mod = ESkrFileMode::SKR_FM_READ_WRITE;
            cr = ESkrFileCreation::SKR_FILE_CREATION_IF_NEEDED;
            break;
        case WRITE_READ:
            mod = ESkrFileMode::SKR_FM_READ_WRITE;
            break;
    }
    fa->file = skr_vfs_fopen(godot_vfs, (char8_t*)p_path.utf8().get_data(), mod, cr);
    if (fa->file == nullptr)
    {
        if (r_error)
            *r_error = ERR_CANT_OPEN;
        SkrDelete(fa);
        return Ref<FileAccess>();
    }
    return Ref<FileAccess>(fa);
}
uint64_t FileAccess::get_length() const
{
    return skr_vfs_fsize(file);
}
Vector<uint8_t> FileAccess::get_buffer(int64_t p_length) const
{
    Vector<uint8_t> ret;
    ret.resize(p_length);
    skr_vfs_fread(file, ret.data(), 0, p_length);
    return ret;
}
Vector<uint8_t> FileAccess::get_file_as_bytes(const String& p_path)
{
    auto file = skr_vfs_fopen(godot_vfs, (char8_t*)p_path.utf8().get_data(), ESkrFileMode::SKR_FM_READ_BINARY, ESkrFileCreation::SKR_FILE_CREATION_OPEN_EXISTING);
    if (file == nullptr)
        return Vector<uint8_t>();
    auto size = skr_vfs_fsize(file);
    Vector<uint8_t> ret;
    ret.resize(size);
    skr_vfs_fread(file, ret.data(), 0, size);
    skr_vfs_fclose(file);
    return ret;
}
FileAccess::~FileAccess() SKR_NOEXCEPT
{
    skr_vfs_fclose(file);
}
}