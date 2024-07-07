// Copyright 2023 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUModelRef for live access
 */
#ifndef SKETCHUP_APPLICATION_MODEL_H_
#define SKETCHUP_APPLICATION_MODEL_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/application/overlay.h>

#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Creates an overlay and adds it to the model.
@since SketchUp 2024, API 12.0
@param[in] model The model object
@param[in] info The information needed to create the overlay object
@param[out] overlay The created overlay object
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p model is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if \p info is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p overlay is NULL
- \ref SU_ERROR_OVERWRITE_VALID if \p overlay is already a valid object
- \ref SU_ERROR_DUPLICATE if the given SUOverlayCreateInfo::id already exists in the model
@relatedalso SUModelRef
@relatedalso SUOverlayRef
*/
SU_RESULT SUModelCreateOverlay(
    SUModelRef model, const struct SUOverlayCreateInfo* info, SUOverlayRef* overlay);

/**
@brief Releases the overlay after removing it from the model.
@param[in] model The model object
@param[in,out] overlay The overlay object to be released, will be set to NULL on return.
@since SketchUp 2024, API 12.0
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p model or \p overlay is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if \p overlay is NULL
- \ref SU_ERROR_NO_DATA if \p overlay could be found in the model
@relatedalso SUModelRef
@relatedalso SUOverlayRef
*/
SU_RESULT SUModelReleaseOverlay(SUModelRef model, SUOverlayRef* overlay);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif  // SKETCHUP_APPLICATION_MODEL_H_
