// Copyright 2015 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUStyleRef.
 */
#ifndef SKETCHUP_MODEL_STYLE_H_
#define SKETCHUP_MODEL_STYLE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUStyleRef
@extends SUEntityRef
@brief  A style entity reference.
@since SketchUp 2017, API 5.0
*/

/**
@brief Creates an empty style object.
@since SketchUp 2017, API 5.0
@param[out] style The style object.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if style is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *style already refers to a valid object
*/
SU_RESULT SUStyleCreate(SUStyleRef* style);

/**
@brief Creates a style object from a file at the given path.
@since SketchUp 2017, API 5.0
@param[out] style The style object.
@param[in]  path  The file path.
Assumed to be UTF-8 encoded.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if style is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *style already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_SERIALIZATION if style couldn't be created from the file at path
*/
SU_RESULT SUStyleCreateFromFile(SUStyleRef* style, const char* path);

/**
 @brief Releases a style object.
 @since SketchUp 2017, API 5.0
 @param[in] style The style object.
 @related SUStylesRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_NULL_POINTER_INPUT if style is NULL
 - \ref SU_ERROR_INVALID_INPUT if *style does not refer to a valid object
 */
SU_RESULT SUStyleRelease(SUStyleRef* style);

/**
@brief Converts from an \ref SUStyleRef to an \ref SUEntityRef. This is
       essentially an upcast operation.
@since SketchUp 2017, API 5.0
@param[in] style The style object.
@related SUStylesRef
@return
- The converted \ref SUEntityRef if style is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUStyleToEntity(SUStyleRef style);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUStyleRef.
       This is essentially a downcast operation so the given \ref SUEntityRef
       must be convertible to an \ref SUStyleRef.
@since SketchUp 2017, API 5.0
@param[in] entity The entity object.
@related SUStylesRef
@return
- The converted \ref SUStyleRef if the downcast operation succeeds
- If the downcast operation fails, the returned reference will be invalid
*/
SU_EXPORT SUStyleRef SUStyleFromEntity(SUEntityRef entity);

/**
@brief Sets the name of a style object.
@since SketchUp 2017, API 5.0
@param[in] style The style object.
@param[in] name  The name string to set the style object.
Assumed to be UTF-8 encoded.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
*/
SU_RESULT SUStyleSetName(SUStyleRef style, const char* name);

/**
@brief Retrieves the name of a style object.
@since SketchUp 2017, API 5.0
@param[in]  style The style object.
@param[out] name  The name retrieved.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUStyleGetName(SUStyleRef style, SUStringRef* name);

/**
@brief Retrieves the display name of a style object.  If the name begins with a
       wildcard character "*" the wildcard will be replaced be a default
       string.  The default string defaults to "*", so is no default string is
       set this function will always return the same string as GetName.
@since SketchUp 2017, API 5.0
@param[in]  style The style object.
@param[out] name  The display name retrieved.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUStyleGetDisplayName(SUStyleRef style, SUStringRef* name);

/**
@brief Sets the description of a style object.
@since SketchUp 2017, API 5.0
@param[in] style       The style object.
@param[in] description The description string to set the style object.
Assumed to be UTF-8 encoded.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if description is NULL
*/
SU_RESULT SUStyleSetDescription(SUStyleRef style, const char* description);

/**
@brief Retrieves the description of a style object.
@since SketchUp 2017, API 5.0
@param[in]  style       The style object.
@param[out] description The description retrieved.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if description is NULL
- \ref SU_ERROR_INVALID_OUTPUT if description does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUStyleGetDescription(SUStyleRef style, SUStringRef* description);

/**
@brief Retrieves the filepath to the file which was used to import this style.
@since SketchUp 2017, API 5.0
@param[in]  style The style object.
@param[out] path  The path retrieved.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if path is NULL
- \ref SU_ERROR_INVALID_OUTPUT if path does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUStyleGetPath(SUStyleRef style, SUStringRef* path);

/**
@brief Retrieves the GUID of a style object.
@since SketchUp 2017, API 5.0
@param[in]  style The style object.
@param[out] guid  The GUID retrieved.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if guid is NULL
- \ref SU_ERROR_INVALID_OUTPUT if guid does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUStyleGetGuid(SUStyleRef style, SUStringRef* guid);

/**
@brief Retrieves a boolean indicating whether the style displays a watermark.
@since SketchUp 2017, API 5.0
@param[in]  style      The style object.
@param[out] shows_mark The boolean retrieved.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if shows_mark is NULL
*/
SU_RESULT SUStyleGetDisplaysWatermark(SUStyleRef style, bool* shows_mark);

/**
@brief Saves the style data to the specified path.
@since SketchUp 2017, API 5.0
@param[in] style The style object.
@param[in] path  The path to where the data should be saved. Assumed to be
UTF-8 encoded.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_SERIALIZATION if the save operation fails
*/
SU_RESULT SUStyleSaveToFile(SUStyleRef style, const char* path);

/**
@enum SUStylePropertyType
@brief The set style properties that can be set/retrieved.  Each property
       supports a single data type indicated by comments in the list.
*/
enum SUStylePropertyType {
  // Edge Parameters (0 - 63)
  SUStyleEdgesColor = 0,              ///< data type: SUTypedValueType_Color.
  SUStyleEdgesExtensionsEnabled = 1,  ///< data type: SUTypedValueType_Bool.
  SUStyleEdgesExtensionLength = 2,    ///< data type: SUTypedValueType_Int32.
  SUStyleEdgesProfilesEnabled = 3,    ///< data type: SUTypedValueType_Bool.
  SUStyleEdgesProfileWidth = 4,       ///< data type: SUTypedValueType_Int32.
  SUStyleEdgesDepthCueEnabled = 5,    ///< data type: SUTypedValueType_Bool.
  SUStyleEdgesDepthCueLevels = 6,     ///< data type: SUTypedValueType_Int32.
  // Background Parameters (64 - 127)
  SUStyleBackgroundColor = 64  ///< data type: SUTypedValueType_Color.
};

/**
@brief Sets the value of the specified \ref SUStylePropertyType.
@since SketchUp 2017, API 5.0
@param[in] style The style object.
@param[in] type  The style type to set.
@param[in] value The value to set for type.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style or value are not valid objects or
       value has the incorrect data type
- \ref SU_ERROR_OUT_OF_RANGE if attempting to set an unhandled property type
- \ref SU_ERROR_NO_DATA if attempting to set an unhandled data type
*/
SU_RESULT SUStyleSetProperty(
    SUStyleRef style, enum SUStylePropertyType type, SUTypedValueRef value);

/**
@brief Retrieves a \ref SUTypedValueRef containing the value of the specified
       \ref SUStylePropertyType.
@since SketchUp 2017, API 5.0
@param[in]  style The style object.
@param[in]  type  The style type to retrieve.
@param[out] value The \ref SUTypedValueRef retrieved.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if value is NULL
- \ref SU_ERROR_INVALID_OUTPUT if value is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if attempting to get an unhandled property type
- \ref SU_ERROR_NO_DATA if the style doesn't contain the specified property.
- \ref SU_ERROR_GENERIC if the retrieved data type is incorrect.
*/
SU_RESULT SUStyleGetProperty(
    SUStyleRef style, enum SUStylePropertyType type, SUTypedValueRef* value);


/**
@brief Retrieves an image containing the style's thumbnail.  The given image
       representation object must have been constructed using
       SUImageRepCreate(). It must be released using SUImageRepRelease().
@since SketchUp 2017, API 5.0
@param[in]  style The style object.
@param[out] image The image object retrieved.
@related SUStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image is NULL
- \ref SU_ERROR_INVALID_OUTPUT if image does not point to a valid \ref
       SUImageRepRef object
- \ref SU_ERROR_NO_DATA if there was no image data
*/
SU_RESULT SUStyleGetThumbnail(SUStyleRef style, SUImageRepRef* image);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_STYLE_H_
