#include "SkrBase/misc/debug.h" 
#if defined(_MSC_VER)
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <limits.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <memory>
#include <crtdbg.h>
#include <dbghelp.h>
// clang-format on
#if defined(_MSC_VER)
#pragma comment(lib, "dbghelp.lib")
#endif
using so_handle = HMODULE;

#ifdef UNICODE
#   define CR_WINDOWS_ConvertPath(_newpath, _path)     std::wstring _newpath(cr_utf8_to_wstring(_path))

static std::wstring cr_utf8_to_wstring(const std::string &str) {
    int wlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
    wchar_t wpath_small[MAX_PATH];
    std::unique_ptr<wchar_t[]> wpath_big;
    wchar_t *wpath = wpath_small;
    if (wlen > _countof(wpath_small)) {
        wpath_big = std::unique_ptr<wchar_t[]>(new wchar_t[wlen]);
        wpath = wpath_big.get();
    }

    if (MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wpath, wlen) != wlen) {
        return L"";
    }

    return wpath;
}
#else
#   define CR_WINDOWS_ConvertPath(_newpath, _path)     const std::string &_newpath = _path
#endif  // UNICODE

template <class T>
static T struct_cast(void *ptr, LONG offset = 0) {
    return reinterpret_cast<T>(reinterpret_cast<intptr_t>(ptr) + offset);
}
// RSDS Debug Information for PDB files
using DebugInfoSignature = DWORD;
#define CR_RSDS_SIGNATURE 'SDSR'
struct cr_rsds_hdr {
    DebugInfoSignature signature;
    GUID guid;
    long version;
    char filename[1];
};

static bool cr_pe_debugdir_rva(PIMAGE_OPTIONAL_HEADER optionalHeader,
                               DWORD &debugDirRva, DWORD &debugDirSize) {
    if (optionalHeader->Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        auto optionalHeader64 =
            struct_cast<PIMAGE_OPTIONAL_HEADER64>(optionalHeader);
        debugDirRva =
            optionalHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG]
                .VirtualAddress;
        debugDirSize =
            optionalHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
    } else {
        auto optionalHeader32 =
            struct_cast<PIMAGE_OPTIONAL_HEADER32>(optionalHeader);
        debugDirRva =
            optionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG]
                .VirtualAddress;
        debugDirSize =
            optionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
    }

    if (debugDirRva == 0 && debugDirSize == 0) {
        return true;
    } else if (debugDirRva == 0 || debugDirSize == 0) {
        return false;
    }

    return true;
}

static bool cr_pe_fileoffset_rva(PIMAGE_NT_HEADERS ntHeaders, DWORD rva,
                                 DWORD &fileOffset) {
    bool found = false;
    auto *sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections;
         i++, sectionHeader++) {
        auto sectionSize = sectionHeader->Misc.VirtualSize;
        if ((rva >= sectionHeader->VirtualAddress) &&
            (rva < sectionHeader->VirtualAddress + sectionSize)) {
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    const int diff = static_cast<int>(sectionHeader->VirtualAddress -
                                sectionHeader->PointerToRawData);
    fileOffset = rva - diff;
    return true;
}

static char *cr_pdb_find(LPBYTE imageBase, PIMAGE_DEBUG_DIRECTORY debugDir) {
    SKR_ASSERT(debugDir && imageBase);
    LPBYTE debugInfo = imageBase + debugDir->PointerToRawData;
    const auto debugInfoSize = debugDir->SizeOfData;
    if (debugInfo == 0 || debugInfoSize == 0) {
        return nullptr;
    }

    if (IsBadReadPtr(debugInfo, debugInfoSize)) {
        return nullptr;
    }

    if (debugInfoSize < sizeof(DebugInfoSignature)) {
        return nullptr;
    }

    if (debugDir->Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
        auto signature = *(DWORD *)debugInfo;
        if (signature == CR_RSDS_SIGNATURE) {
            auto *info = (cr_rsds_hdr *)(debugInfo);
            if (IsBadReadPtr(debugInfo, sizeof(cr_rsds_hdr))) {
                return nullptr;
            }

            if (IsBadStringPtrA((const char *)info->filename, UINT_MAX)) {
                return nullptr;
            }

            return info->filename;
        }
    }

    return nullptr;
}

bool cr_pdb_replace(const std::string &filename, const std::string &pdbname,
                           std::string &orig_pdb) {
    CR_WINDOWS_ConvertPath(_filename, filename);

    HANDLE fp = nullptr;
    HANDLE filemap = nullptr;
    LPVOID mem = 0;
    bool result = false;
    do {
        fp = CreateFile(_filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, nullptr);
        if ((fp == INVALID_HANDLE_VALUE) || (fp == nullptr)) {
            break;
        }

        filemap = CreateFileMapping(fp, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        if (filemap == nullptr) {
            break;
        }

        mem = MapViewOfFile(filemap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (mem == nullptr) {
            break;
        }

        auto dosHeader = struct_cast<PIMAGE_DOS_HEADER>(mem);
        if (dosHeader == 0) {
            break;
        }

        if (IsBadReadPtr(dosHeader, sizeof(IMAGE_DOS_HEADER))) {
            break;
        }

        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            break;
        }

        auto ntHeaders =
            struct_cast<PIMAGE_NT_HEADERS>(dosHeader, dosHeader->e_lfanew);
        if (ntHeaders == 0) {
            break;
        }

        if (IsBadReadPtr(ntHeaders, sizeof(ntHeaders->Signature))) {
            break;
        }

        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            break;
        }

        if (IsBadReadPtr(&ntHeaders->FileHeader, sizeof(IMAGE_FILE_HEADER))) {
            break;
        }

        if (IsBadReadPtr(&ntHeaders->OptionalHeader,
                         ntHeaders->FileHeader.SizeOfOptionalHeader)) {
            break;
        }

        if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
            ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            break;
        }

        auto sectionHeaders = IMAGE_FIRST_SECTION(ntHeaders);
        if (IsBadReadPtr(sectionHeaders,
                         ntHeaders->FileHeader.NumberOfSections *
                             sizeof(IMAGE_SECTION_HEADER))) {
            break;
        }

        DWORD debugDirRva = 0;
        DWORD debugDirSize = 0;
        if (!cr_pe_debugdir_rva(&ntHeaders->OptionalHeader, debugDirRva,
                                debugDirSize)) {
            break;
        }

        if (debugDirRva == 0 || debugDirSize == 0) {
            break;
        }

        DWORD debugDirOffset = 0;
        if (!cr_pe_fileoffset_rva(ntHeaders, debugDirRva, debugDirOffset)) {
            break;
        }

        auto debugDir =
            struct_cast<PIMAGE_DEBUG_DIRECTORY>(mem, debugDirOffset);
        if (debugDir == 0) {
            break;
        }

        if (IsBadReadPtr(debugDir, debugDirSize)) {
            break;
        }

        if (debugDirSize < sizeof(IMAGE_DEBUG_DIRECTORY)) {
            break;
        }

        int numEntries = debugDirSize / sizeof(IMAGE_DEBUG_DIRECTORY);
        if (numEntries == 0) {
            break;
        }

        for (int i = 1; i <= numEntries; i++, debugDir++) {
            char *pdb = cr_pdb_find((LPBYTE)mem, debugDir);
            if (pdb) {
                auto len = strlen(pdb);
                if (len >= strlen(pdbname.c_str())) {
                    orig_pdb = pdb;
                    memcpy_s(pdb, len, pdbname.c_str(), pdbname.length());
                    pdb[pdbname.length()] = 0;
                    result = true;
                }
            }
        }
    } while (0);

    if (mem != nullptr) {
        UnmapViewOfFile(mem);
    }

    if (filemap != nullptr) {
        CloseHandle(filemap);
    }

    if ((fp != nullptr) && (fp != INVALID_HANDLE_VALUE)) {
        CloseHandle(fp);
    }

    return result;
}
#endif