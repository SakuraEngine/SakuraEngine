// Copyright 2014 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Interfaces for SUExtensionLicense.
 */
#ifndef SKETCHUP_EXTENSION_LICENSE_H_
#define SKETCHUP_EXTENSION_LICENSE_H_

#include <SketchUpAPI/common.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Indicates the current state of an extension license.
*/
enum SUExtensionLicenseState {
  SUExtensionLicenseState_Licensed,      ///< Extension is licensed to run
  SUExtensionLicenseState_Expired,       ///< License had an expiration date and it expired
  SUExtensionLicenseState_Trial,         ///< Extension is in trial
  SUExtensionLicenseState_TrialExpired,  ///< Trial period has ended
  SUExtensionLicenseState_NotLicensed    ///< Extension is not licensed
};

/**
@brief Stores extension license information.
*/
struct SUExtensionLicense {
  /// Main flag indicating whether the extension is allowed to run or not.
  bool is_licensed;
  /// Additional state information.
  enum SUExtensionLicenseState state;
  /// Number of days until license expiration. 0 if permanent or not licensed.
  size_t days_remaining;
  /// Error description in case of failure to acquire a license. This is meant
  /// to aid in debugging only. Extensions should not rely on any exact error
  /// description.
  char error_description[512];
};

/**
@brief Acquires a license for a given extension.
@param[in]  extension_id The Extension Warehouse UUID for the extension.
@param[out] out_license  the licensing retrieved.
@related SUExtensionLicense
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_GENERIC if retrieving the extension license failed
- \ref SU_ERROR_NULL_POINTER_INPUT if extension_id is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if out_license is NULL
*/
SU_RESULT SUGetExtensionLicense(const char* extension_id, struct SUExtensionLicense* out_license);

#ifdef __cplusplus
}  // extern "C" {
#endif
#pragma pack(pop)

#endif  // SKETCHUP_EXTENSION_LICENSE_H_
