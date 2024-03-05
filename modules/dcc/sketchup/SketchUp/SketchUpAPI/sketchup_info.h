// Copyright 2015 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Interfaces for obtaining information about the executing SketchUp
 *   application.
 * @note This is only relevant for the Live API.
 */
#ifndef SKETCHUP_SKETCHUP_INFO_H_
#define SKETCHUP_SKETCHUP_INFO_H_

#include <SketchUpAPI/common.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@enum SUEdition
@brief  This is the edition of SketchUp currently running.
@since SketchUp 2016, API 4.0
*/
enum SUEdition {
  SUEdition_Unknown,
  SUEdition_Make,  ///< SketchUp Make
  SUEdition_Pro    ///< SketchUp Pro
};

/**
@brief Returns the version string for the current SketchUp version. This is
       exported only by the SketchUp executable. It is not part of the
       standalone SDK.
@since SketchUp 2016, API 4.0
@param[in]  length  Length of the string buffer passed in including null
                    terminator.
@param[out] version The UTF-8 encoded version string. This must be large enough
                    to hold the version string including null terminator.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if version is NULL
- \ref SU_ERROR_INSUFFICIENT_SIZE if length is too small
*/
SU_RESULT SUGetVersionStringUtf8(size_t length, char* version);

/**
@brief Returns the SketchUp edition (Pro or Make). This is only exported by
       the SketchUp executable. It is not part of the standalone SDK. Note:
       Starting with version 2018, SketchUp Make is no longer available. So this
       function will always return \ref SUEdition_Pro.
@since SketchUp 2016, API 4.0
@param[out] edition The edition of Sketchup
@see SUEdition
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edition is NULL
*/
SU_RESULT SUGetEdition(enum SUEdition* edition);

#ifdef __cplusplus
}  // extern "C" {
#endif
#pragma pack(pop)

#endif  // SKETCHUP_SKETCHUP_INFO_H_
