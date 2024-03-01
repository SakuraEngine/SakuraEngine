#pragma once
#include "SkrBase/config.h"

static const uint32_t kWin7Major = 6;
static const uint32_t kWin10Major = 10;
static const uint32_t kWin11Major = 10;

static const uint32_t kWin10_0000 = 10240;
static const uint32_t kWin10_1511 = 10586;
static const uint32_t kWin10_1607 = 14393;
static const uint32_t kWin10_1703 = 15063;
static const uint32_t kWin10_1709 = 16299;
static const uint32_t kWin10_1803 = 17134;
static const uint32_t kWin10_1809 = 17763;
static const uint32_t kWin10_1903 = 18362;
static const uint32_t kWin10_1909 = 18363;
static const uint32_t kWin10_2004 = 19041;
static const uint32_t kWin10_20H2 = 19042;
static const uint32_t kWin10_21H1 = 19043;
static const uint32_t kWin10_21H2 = 19044;

static const uint32_t kWin11_DevBuild = 21996;
static const uint32_t kWin11_21H2 = 22000;
static const uint32_t kWin11_22H2 = 22621;

SKR_EXTERN_C bool skr_win_is_file_on_ssd(const char8_t* file_path);
SKR_EXTERN_C bool skr_win_is_executable_on_ssd();
SKR_EXTERN_C bool skr_win_is_wine();
SKR_EXTERN_C bool skr_win_verify_version(uint32_t major, uint32_t minor);