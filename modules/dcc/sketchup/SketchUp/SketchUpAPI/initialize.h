// Copyright 2013-2020 Trimble Inc.  All Rights Reserved

/**
 * @file
 * @brief Functionality for the API interface itself.
 */
#ifndef SKETCHUP_INITIALIZE_H_
#define SKETCHUP_INITIALIZE_H_

#include <SketchUpAPI/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Initializes the slapi interface. Must be called before calling any other
       API function.
@attention This function should not be used from the Live API.
*/
SU_EXPORT void SUInitialize();

/**
@brief Signals termination of use of the slapi interface. Must be called when
       done using API functions.
@attention This function should not be used from the Live API.
*/
SU_EXPORT void SUTerminate();

/**
@brief Returns the major and minor API version numbers.
@param[out] major The major version number retrieved.
@param[out] minor The minor version number retrieved.
@note This function hasn't reliably reported the correct versions prior to
      version 8.2 of the SDK.
*/
SU_EXPORT void SUGetAPIVersion(size_t* major, size_t* minor);

#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // SKETCHUP_INITIALIZE_H_
