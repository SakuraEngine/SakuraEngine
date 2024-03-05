// Copyright 2013 Trimble Inc.  All Rights Reserved

/**
 * @file
 * @brief Interfaces for SUOptionsManagerRef.
 */
#ifndef SKETCHUP_MODEL_OPTIONS_MANAGER_H_
#define SKETCHUP_MODEL_OPTIONS_MANAGER_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUOptionsManagerRef
@brief  Provides access to the different options provider objects in the model.
        Available options providers are: PageOptions, SlideshowOptions,
        UnitsOptions and PrintOptions.
*/

/**
@brief  Gets the number of available options providers.
@param[in]  options_manager The options manager object.
@param[out] count           The number of options available.
@related SUOptionsManagerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if options_manager is not valid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUOptionsManagerGetNumOptionsProviders(
    SUOptionsManagerRef options_manager, size_t* count);

/**
@brief  Retrieves options providers associated with the options manager.
@param[in] options_manager         The options manager object.
@param[in] len                     The number of options provider names
                                   to retrieve.
@param[out] options_provider_names The options provider names retrieved.
@param[out] count                  The number of options provider names
                                   retrieved.
@related SUOptionsManagerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if options_manager is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if options_providers or count is NULL
- \ref SU_ERROR_INVALID_OUTPUT if any of the strings in options_provider_names
  are invalid
*/
SU_RESULT SUOptionsManagerGetOptionsProviderNames(
    SUOptionsManagerRef options_manager, size_t len, SUStringRef options_provider_names[],
    size_t* count);

/**
@brief  Retrieves the options provider given a name.
@param[in] options_manager   The options manager object.
@param[in] name              The name of the options provider object to get.
                             Assumed to be UTF-8 encoded.
@param[out] options_provider The options_provider object retrieved.
@related SUOptionsManagerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if options_manager is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if options_providers is NULL
- \ref SU_ERROR_NO_DATA if the requested options provider object does not exist
*/
SU_RESULT SUOptionsManagerGetOptionsProviderByName(
    SUOptionsManagerRef options_manager, const char* name, SUOptionsProviderRef* options_provider);

#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // SKETCHUP_MODEL_OPTIONS_MANAGER_H_
