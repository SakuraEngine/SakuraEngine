// Copyright 2014-2021 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUSchemaRef.
 */
#ifndef SKETCHUP_MODEL_SCHEMA_H_
#define SKETCHUP_MODEL_SCHEMA_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUSchemaRef
@brief  Used to manage a Schema object
*/

/**
@brief Gets a schema type from a schema.
@param[in]  schema_ref       The schema object.
@param[in]  schema_type_name The name of the schema type to get.
@param[out] schema_type_ref  The schema type retrieved.
@related SUSchemaRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if schema_ref is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if schema_type_name is NULL
- \ref SU_ERROR_INVALID_INPUT if schema_type_name is not a type from this
  schema
- \ref SU_ERROR_NULL_POINTER_OUTPUT if schema_type_ref is not a valid object
*/
SU_RESULT SUSchemaGetSchemaType(
    SUSchemaRef schema_ref, const char* schema_type_name, SUSchemaTypeRef* schema_type_ref);

/**
@brief Gets the name from a schema object.
@since SketchUp 2022.0, API 10.0
@param[in]  schema_ref   The schema object.
@param[out] schema_name  The name of the schema.
@related SUSchemaRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p schema_ref is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p schema_name is NULL
*/
SU_RESULT SUSchemaGetSchemaName(SUSchemaRef schema_ref, SUStringRef* schema_name);

#ifdef __cplusplus
}
#endif

#endif  // SKETCHUP_MODEL_SCHEMA_H_
