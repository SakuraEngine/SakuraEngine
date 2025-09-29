#pragma once

#include "llvm/Support/VirtualFileSystem.h"

namespace meta {

// Custom FileSystem that returns empty content for .generated.h files
class IgnoreGeneratedFileSystem : public llvm::vfs::ProxyFileSystem {
public:
  explicit IgnoreGeneratedFileSystem(llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> UnderlyingFS)
      : ProxyFileSystem(UnderlyingFS) {}

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(const llvm::Twine &Path) override;

  llvm::ErrorOr<llvm::vfs::Status> status(const llvm::Twine &Path) override;

private:
  bool shouldIgnoreFile(llvm::StringRef Path) const;
};

} // namespace meta
