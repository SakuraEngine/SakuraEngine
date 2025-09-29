#include "IgnoreGeneratedFileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

namespace meta {

// Custom File class that returns empty content
class EmptyFile : public llvm::vfs::File {
public:
  explicit EmptyFile(llvm::StringRef Name) : FileName(Name.str()) {}

  llvm::ErrorOr<llvm::vfs::Status> status() override {
    return llvm::vfs::Status(FileName, llvm::sys::fs::UniqueID(0, 0),
                             std::chrono::system_clock::now(), 0, 0, 0,
                             llvm::sys::fs::file_type::regular_file,
                             llvm::sys::fs::all_all);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(const llvm::Twine &Name, int64_t FileSize, bool RequiresNullTerminator,
            bool IsVolatile) override {
    // Return empty buffer for .generated.h files
    return llvm::MemoryBuffer::getMemBuffer("", Name.str());
  }

  std::error_code close() override { return std::error_code(); }

private:
  std::string FileName;
};

// IgnoreGeneratedFileSystem implementation
bool IgnoreGeneratedFileSystem::shouldIgnoreFile(llvm::StringRef Path) const {
  // Check for .generated.h in various formats
  return Path.ends_with(".generated.h") || 
         Path.ends_with(".generated.H") ||
         Path.contains(".generated.h");
}

llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
IgnoreGeneratedFileSystem::openFileForRead(const llvm::Twine &Path) {
  std::string PathStr = Path.str();
  
  if (shouldIgnoreFile(PathStr)) {
    llvm::outs() << "[Meta] Ignoring .generated.h file: " << PathStr << "\n";
    return std::make_unique<EmptyFile>(PathStr);
  }
  
  // For other files, use the underlying file system
  return ProxyFileSystem::openFileForRead(Path);
}

llvm::ErrorOr<llvm::vfs::Status> IgnoreGeneratedFileSystem::status(const llvm::Twine &Path) {
  std::string PathStr = Path.str();
  
  if (shouldIgnoreFile(PathStr)) {
    // Return a fake status for .generated.h files
    return llvm::vfs::Status(PathStr, llvm::sys::fs::UniqueID(0, 0),
                             std::chrono::system_clock::now(), 0, 0, 0,
                             llvm::sys::fs::file_type::regular_file,
                             llvm::sys::fs::all_all);
  }
  
  // For other files, use the underlying file system
  return ProxyFileSystem::status(Path);
}

} // namespace meta
