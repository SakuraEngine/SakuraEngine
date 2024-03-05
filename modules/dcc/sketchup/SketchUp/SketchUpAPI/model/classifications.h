// Copyright 2014-2021 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUClassificationsRef.
 */
#ifndef SKETCHUP_MODEL_CLASSIFICATIONS_H_
#define SKETCHUP_MODEL_CLASSIFICATIONS_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUClassificationsRef
@brief  Used to manage a Classifications object
*/

/**
@brief Loads a schema into a classifications object.
@param[in]  classifications  The classificationss object.
@param[in]  schema_file_name The full path of the schema to load.
@related SUClassificationsRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if classifications is not a valid object
- \ref SU_ERROR_INVALID_INPUT if schema_file_name is not a valid path to a schema
       or is NULL
*/
SU_RESULT SUClassificationsLoadSchema(
    SUClassificationsRef classifications, const char* schema_file_name);

/**
@brief Gets a schema from a classifications object.
@param[in]  classifications  The classifications object.
@param[in]  schema_name      The name of the schema to get.
@param[out] schema_ref       The schema retrieved.
@related SUClassificationsRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if classifications is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if schema_name is NULL
- \ref SU_ERROR_INVALID_INPUT if schema_name is not a loaded schema
- \ref SU_ERROR_NULL_POINTER_OUTPUT if schema_ref is NULL
*/
SU_RESULT SUClassificationsGetSchema(
    SUClassificationsRef classifications, const char* schema_name, SUSchemaRef* schema_ref);

/**
@brief Gets the number of schemata from a classifications object.
@since SketchUp 2022.0, API 10.0
@param[in]  classifications  The classifications object.
@param[out] num_schemas      The number of schemas in the classifications object.
@related SUClassificationsRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p classifications is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p num_schemas is NULL
*/
SU_RESULT SUClassificationsGetNumSchemas(SUClassificationsRef classifications, size_t* num_schemas);

/**
@brief Retrieves the schemas in the classifications object.
@since SketchUp 2022.0, API 10.0
@param[in]  classifications  The classifications object.
@param[in]  len              The number of schemas to retrieve.
@param[out] schemas          The schemas retrieved.
@param[out] count            The number of schemas retrieved.
@related SUClassificationsRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p classifications is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p schemas or \p count is NULL
- \ref SU_ERROR_NO_DATA if \p len is greater than 0, but no schemas were found in \p classifications
*/
SU_RESULT SUClassificationsGetSchemas(
    SUClassificationsRef classifications, size_t len, SUSchemaRef schemas[], size_t* count);

#ifdef __cplusplus
}
#endif

#endif  // SKETCHUP_MODEL_CLASSIFICATIONS_H_
