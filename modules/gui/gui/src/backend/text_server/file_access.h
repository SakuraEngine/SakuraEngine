#pragma once
#include "godot_containers.hpp"
#include "SkrCore/platform/vfs.h"

namespace godot 
{
class FileAccess {

public:
	enum AccessType {
		ACCESS_RESOURCES,
		ACCESS_USERDATA,
		ACCESS_FILESYSTEM,
		ACCESS_MAX
	};

	enum ModeFlags {
		READ = 1,
		WRITE = 2,
		READ_WRITE = 3,
		WRITE_READ = 7,
	};

    static void set_vfs(skr_vfs_t* vfs);
    
	static Ref<FileAccess> open(const String &p_path, int p_mode_flags, Error *r_error = nullptr); /// Create a file access (for the current platform) this is the only portable way of accessing files.
	static Vector<uint8_t> get_file_as_bytes(const String& p_path);

	virtual uint64_t get_length() const; ///< get size of the file
	Vector<uint8_t> get_buffer(int64_t p_length) const;
    
    virtual ~FileAccess() SKR_NOEXCEPT;
    skr_vfile_t* file;
};
}