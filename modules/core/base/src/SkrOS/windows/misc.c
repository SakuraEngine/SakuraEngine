#include "stdbool.h"
#include "SkrOS/windows/misc.h"
#include "SkrBase/misc/debug.h" 
#ifdef _WIN32
#include "./../winheaders.h"
#endif

HANDLE __GetVolumeHandleForFile(const wchar_t* filePath)
{
    wchar_t volume_path[MAX_PATH];
    if (!GetVolumePathName(filePath, volume_path, ARRAYSIZE(volume_path)))
        return NULL;

    wchar_t volume_name[MAX_PATH];
    if (!GetVolumeNameForVolumeMountPoint(volume_path,
        volume_name, ARRAYSIZE(volume_name)))
        return NULL;

    uint64_t length = wcslen(volume_name);
    if (length && volume_name[length - 1] == L'\\')
        volume_name[length - 1] = L'\0';

    return CreateFile(volume_name, 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
}

bool __IsFileOnSSD(const wchar_t* file_path)
{
    bool is_ssd = false;
    HANDLE volume = __GetVolumeHandleForFile(file_path);
    if (volume == INVALID_HANDLE_VALUE)
    {
        return false; /*invalid path! throw?*/
    }

    STORAGE_PROPERTY_QUERY query = {0};
    query.PropertyId = StorageDeviceSeekPenaltyProperty;
    query.QueryType = PropertyStandardQuery;
    DWORD count;
    DEVICE_SEEK_PENALTY_DESCRIPTOR result = {0};
    if (DeviceIoControl(volume, IOCTL_STORAGE_QUERY_PROPERTY,
        &query, sizeof(query), &result, sizeof(result), &count, NULL))
    {
        is_ssd = !result.IncursSeekPenalty;
    }
    else { /*fails for network path, etc*/ }
    CloseHandle(volume);
    return is_ssd;
}

bool __IsFileOnSSD_U8(const char8_t* file_path)
{
    int wlen = MultiByteToWideChar(CP_UTF8, 0, file_path, -1, 0, 0);
    wchar_t wpath_small[MAX_PATH];
    wchar_t* wpath_big = NULL;
    wchar_t* wpath = wpath_small;
    if (wlen > _countof(wpath_small)) 
    {
        wpath = wpath_big = (wchar_t*)malloc(wlen * sizeof(wchar_t));
    }

    if (MultiByteToWideChar(CP_UTF8, 0, file_path, -1, wpath, wlen) != wlen) 
    {
        SKR_ASSERT(0);
        return false;
    }
    
    bool result = __IsFileOnSSD(wpath);
    if (wpath_big) 
    {
        free(wpath_big);
    }
    return result;
}

bool __IsProgramOnSSD()
{
    TCHAR path[MAX_PATH];
    if (GetModuleFileName(NULL, path, MAX_PATH))
    {
        return __IsFileOnSSD(path);
    }
    return false;
}

const char* __GetWineVersion()
{
    HMODULE hntdll = GetModuleHandle(TEXT("ntdll.dll"));
    if (!hntdll) 
        return NULL;
    static const char * (CDECL *pwine_get_version)(void);
    pwine_get_version = (void*)GetProcAddress(hntdll, "wine_get_version");
    if (pwine_get_version)
        return pwine_get_version();
    return NULL;
}

bool __VerifyWindowsVersion(uint32_t MajorVersion, uint32_t MinorVersion)
{
	OSVERSIONINFOEX Version;
	Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	Version.dwMajorVersion = MajorVersion;
	Version.dwMinorVersion = MinorVersion;

	ULONGLONG ConditionMask = 0;
	ConditionMask = VerSetConditionMask(ConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	ConditionMask = VerSetConditionMask(ConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

	return !!VerifyVersionInfo(&Version, VER_MAJORVERSION | VER_MINORVERSION, ConditionMask);
}

bool skr_win_is_file_on_ssd(const char8_t* file_path)
{
    return __IsFileOnSSD_U8(file_path);
}

bool skr_win_is_executable_on_ssd()
{
    return __IsProgramOnSSD();
}

bool skr_win_is_wine()
{
    return (__GetWineVersion() != NULL);
}
