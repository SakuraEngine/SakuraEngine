// Copyright 2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for reading metadata from SketchUp files without loading
 *        the whole file.
 */
#ifndef SKETCHUP_MODEL_SKP_H_
#define SKETCHUP_MODEL_SKP_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@since SketchUp 2021.0, API 9.0

@brief Reads the GUID, globally unique identifier, for a model without opening
  the whole file.

@see SUModelGetGuid
@see SUComponentDefinitionGetGuid

@param[in]  file_path  The model filepath to read from.
@param[out] guid       The guid string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if \p file_path is `NULL`
- \ref SU_ERROR_INVALID_INPUT if \p file_path does not exist
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p guid is `NULL`
- \ref SU_ERROR_INVALID_OUTPUT if \p guid does not point to a valid
  SUStringRef object.
- \ref SU_ERROR_MODEL_INVALID if the model is not valid
- \ref SU_ERROR_GENERIC if an unknown error occured reading the file
*/
SU_RESULT SUSkpReadGuid(const char* file_path, SUStringRef* guid);

#ifdef __cplusplus
}
#endif

#endif  // SKETCHUP_MODEL_SKP_H_
