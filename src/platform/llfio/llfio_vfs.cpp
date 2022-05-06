#include "platform/vfs.h"
#include "utils/log.h"
#include "llfio.hpp"
#include <ghc/filesystem.hpp>

bool skr_llfio_fopen(skr_vfs_t* fs, const char8_t* path,
ESkrFileMode mode, const char8_t* password, skr_vfile_t* out_file)
{
    namespace llfio = LLFIO_V2_NAMESPACE;
    ghc::filesystem::path p(fs->mount_dir ? fs->mount_dir : "");
    p /= path;
    // Open the file for read
    llfio::file_handle fh =
    llfio::file( //
    {},          // path_handle to base directory
    p.c_str()    // path_view to path fragment relative to base directory
                 // default mode is read only
                 // default creation is open existing
                 // default caching is all
                 // default flags is none
    )
    .value(); // If failed, throw a filesystem_error exception

    // Make a vector sized the current maximum extent of the file
    std::vector<llfio::byte> buffer(fh.maximum_extent().value());

    // Synchronous scatter read from file
    llfio::file_handle::size_type bytesread =
    llfio::read(
    fh,                                  // handle to read from
    0,                                   // offset
    { { buffer.data(), buffer.size() } } // Single scatter buffer of the vector
                                         // default deadline is infinite
    )
    .value(); // If failed, throw a filesystem_error exception

    // In case of racy truncation of file by third party to new length, adjust buffer to
    // bytes actually read
    buffer.resize(bytesread);

    return true;
}

void skr_vfs_get_native_procs(struct skr_vfs_proctable_t* procs)
{
    procs->fopen = &skr_llfio_fopen;
}