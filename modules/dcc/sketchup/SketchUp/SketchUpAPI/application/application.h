// Copyright 2017-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for usage within the SketchUp application.
 */
#ifndef SKETCHUP_APPLICATION_APPLICATION_H_
#define SKETCHUP_APPLICATION_APPLICATION_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>
#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Gets a reference to the active model. The reference is owned by the
       SketchUp application and should not be released.
@since SketchUp 2019.2, API 7.1
@param[out] model The model object.
@see SUModelRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p model is `NULL`
- \ref SU_ERROR_NO_DATA if there is no active model
*/
SU_RESULT SUApplicationGetActiveModel(SUModelRef* model);

#ifdef __cplusplus
}
#endif
#pragma pack(pop)

#endif  // SKETCHUP_APPLICATION_APPLICATION_H_
