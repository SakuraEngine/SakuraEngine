#pragma once
#include "platform/configure.h"

namespace skr
{
using AsyncResult = int32_t;

static constexpr AsyncResult ASYNC_RESULT_OK = 1;
static constexpr AsyncResult ASYNC_RESULT_ERROR_THREAD_ALREADY_STARTES = -1;
static constexpr AsyncResult ASYNC_RESULT_ERROR_COND_CREATE_FAILED = -2;
static constexpr AsyncResult ASYNC_RESULT_ERROR_COND_MX_CREATE_FAILED = -3;
static constexpr AsyncResult ASYNC_RESULT_ERROR_TIMEOUT = -4;
static constexpr AsyncResult ASYNC_RESULT_ERROR_INVALID_STATE = -5;
static constexpr AsyncResult ASYNC_RESULT_ERROR_OUT_OF_MEMORY = -6;
static constexpr AsyncResult ASYNC_RESULT_ERROR_INVALID_PARAM = -7;
static constexpr AsyncResult ASYNC_RESULT_ERROR_JOB_NOTHREAD = -8;
static constexpr AsyncResult ASYNC_RESULT_ERROR_UNKNOWN = -999;

} // namespace skr