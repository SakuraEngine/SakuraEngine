// Copyright 2018 Trimble Inc. All Rights Reserverd.

/**
 * @file
 * @brief Interfaces for SULineStyleRef.
 */
#ifndef SKETCHUP_SOURCE_SKORE_SKETCHUP_PUBLIC_MODEL_LINESTYLE_H_
#define SKETCHUP_SOURCE_SKORE_SKETCHUP_PUBLIC_MODEL_LINESTYLE_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SULineStyleRef
@extends SUEntityRef
@brief  References a line style.
*/

/**
@brief Converts from a \ref SULineStyleRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@since SketchUp 2020.1, API 8.1
@param[in] line_style The given line style reference.
@related SULineStyleRef
@return
- The converted \ref SUEntityRef if line_style is a valid line style
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SULineStyleToEntity(SULineStyleRef line_style);

/**
@brief Converts from an \ref SUEntityRef to a \ref SULineStyleRef.
       This is essentially a downcast operation so the given entity must be
       convertible to a \ref SULineStyleRef.
@since SketchUp 2020.1, API 8.1
@param[in] entity_ref  The given entity reference.
@related SULineStyleRef
@return
- The converted \ref SULineStyleRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SULineStyleRef SULineStyleFromEntity(SUEntityRef entity_ref);

/**
@brief Retrieves the name of a line style object.
@since SketchUp 2019, API 7.0
@param[in]  line_style The line style object.
@param[out] name       The name retrieved.
@related SULineStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
       SUStringRef object
*/
SU_RESULT SULineStyleGetName(SULineStyleRef line_style, SUStringRef* name);
#ifdef __cplusplus
}
#endif
#pragma pack(pop)

#endif  // SKETCHUP_SOURCE_SKORE_SKETCHUP_PUBLIC_MODEL_LINESTYLE_H_
