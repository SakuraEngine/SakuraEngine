// Copyright 2013 Trimble Inc.  All Rights Reserved

/**
 * @file
 * @brief Interfaces for SUTypedValueRef.
 */
#ifndef SKETCHUP_MODEL_TYPED_VALUE_H_
#define SKETCHUP_MODEL_TYPED_VALUE_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUTypedValueRef
@brief  Variant object used to represent the value of a key-value attribute
        pair.
*/

/**
@enum SUTypedValueType
@brief The set of types that a \ref SUTypedValueRef can represent.
*/
enum SUTypedValueType {
  SUTypedValueType_Empty = 0,  ///< No value set
  SUTypedValueType_Byte,       ///< Byte value type
  SUTypedValueType_Short,      ///< Short value type
  SUTypedValueType_Int32,      ///< Int32 value type
  SUTypedValueType_Float,      ///< Float value type
  SUTypedValueType_Double,     ///< Double value type
  SUTypedValueType_Bool,       ///< Bool value type
  SUTypedValueType_Color,      ///< Color value type
  SUTypedValueType_Time,       ///< Time value type
  SUTypedValueType_String,     ///< String value type
  SUTypedValueType_Vector3D,   ///< Vector3D value type
  SUTypedValueType_Array       ///< Array value type
};

/**
@brief  Creates a typed value object.  The created object must be released
        with \ref SUTypedValueRelease().
@param[out] typed_value The created typed value object.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if typed_value is NULL
- \ref SU_ERROR_OVERWRITE_VALID if typed_value references a valid object
*/
SU_RESULT SUTypedValueCreate(SUTypedValueRef* typed_value);

/**
@brief  Releases a typed value object that was previously created with
        \ref SUTypedValueCreate().
@param[in] typed_value The typed value object.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if typed_value is NULL
- \ref SU_ERROR_INVALID_INPUT if typed_value references an invalid object
*/
SU_RESULT SUTypedValueRelease(SUTypedValueRef* typed_value);

/**
@brief  Retrieves the type information of a typed value object.
@param[in]  typed_value The typed value object.
@param[out] type        The type information retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if type_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
SU_RESULT SUTypedValueGetType(SUTypedValueRef typed_value, enum SUTypedValueType* type);

/**
@brief  Retrieves the byte value of a typed value object.
@param[in]  typed_value The typed value object.
@param[out] byte_value  The byte value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if byte_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetByte(SUTypedValueRef typed_value, char* byte_value);

/**
@brief  Sets the byte value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] byte_value  The byte value that is assigned.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
SU_RESULT SUTypedValueSetByte(SUTypedValueRef typed_value, char byte_value);

/**
@brief  Retrieves the int16 value of a typed value object.
@param[in]  typed_value The typed value object.
@param[out] int16_value The int16 value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if int16_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetInt16(SUTypedValueRef typed_value, int16_t* int16_value);

/**
@brief  Sets the int16 value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] int16_value The int16 value to set.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
SU_RESULT SUTypedValueSetInt16(SUTypedValueRef typed_value, int16_t int16_value);

/**
@brief  Retrieves the int32 value of a typed value object.
@param[in]  typed_value The typed value object.
@param[out] int32_value The int32 value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if int32_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetInt32(SUTypedValueRef typed_value, int32_t* int32_value);

/**
@brief  Sets the int32 value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] int32_value The int32 value to set.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
SU_RESULT SUTypedValueSetInt32(SUTypedValueRef typed_value, int32_t int32_value);

/**
@brief Retrieves the float value of a typed value object.
@param[in]  typed_value The typed value object.
@param[out] float_value The float value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if float_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetFloat(SUTypedValueRef typed_value, float* float_value);

/**
@brief  Sets the float value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] float_value The float value to set.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
SU_RESULT SUTypedValueSetFloat(SUTypedValueRef typed_value, float float_value);

/**
@brief  Retrieves the double value of a typed value object.
@param[in]  typed_value  The typed value object.
@param[out] double_value  The double value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if double_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetDouble(SUTypedValueRef typed_value, double* double_value);

/**
@brief  Sets the double value of a typed value object.
@param[in] typed_value  The typed value object.
@param[in] double_value The double value to set.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
SU_RESULT SUTypedValueSetDouble(SUTypedValueRef typed_value, double double_value);

/**
@brief  Retrieves the boolean value of a typed value object.
@param[in]  typed_value The typed value object.
@param[out] bool_value  The boolean value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if bool_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetBool(SUTypedValueRef typed_value, bool* bool_value);

/**
@brief  Sets the boolean value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] bool_value  The boolean value to set.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
SU_RESULT SUTypedValueSetBool(SUTypedValueRef typed_value, bool bool_value);

/**
@brief  Retrieves the color value of a typed value object.
@param[in]  typed_value The typed value object.
@param[out] color       The color value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetColor(SUTypedValueRef typed_value, SUColor* color);

/**
@brief  Sets the color value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] color       The color value to set.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if color is NULL
*/
SU_RESULT SUTypedValueSetColor(SUTypedValueRef typed_value, const SUColor* color);

/**
@brief  Retrieves the time value of a typed value object.  The time value is in
        seconds since January 1, 1970.
@param[in]  typed_value The typed value object.
@param[out] time_value  The time value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if time_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetTime(SUTypedValueRef typed_value, int64_t* time_value);

/**
@brief  Sets the time value of a typed value object.  The time value is in
        seconds since January 1, 1970.
@param[in] typed_value The typed value object.
@param[in] time_value  The time value that is set.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
SU_RESULT SUTypedValueSetTime(SUTypedValueRef typed_value, int64_t time_value);

/**
@brief  Retrieves the string value of a typed value object.
@param[in]  typed_value  The typed value object.
@param[out] string_value The string value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
- \ref SU_ERROR_NULL_POINTER_OUTPUT if string_value is NULL
- \ref SU_ERROR_INVALID_OUTPUT if string_value does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUTypedValueGetString(SUTypedValueRef typed_value, SUStringRef* string_value);

/**
@brief  Sets the string value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] string_value  The string value to set. Assumed to be UTF-8 encoded.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if string_value is NULL
*/
SU_RESULT SUTypedValueSetString(SUTypedValueRef typed_value, const char* string_value);

/**
@brief  Retrieves the 3-element vector value of a typed value object
@param[in] typed_value     The typed value object.
@param[out] vector3d_value The 3-element vector value retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vector3d_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetVector3d(SUTypedValueRef typed_value, double vector3d_value[3]);

/**
@brief  Sets the 3-element vector value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] vector3d_value  The 3-element vector value to set.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
SU_RESULT SUTypedValueSetVector3d(SUTypedValueRef typed_value, const double vector3d_value[3]);

/**
@brief  Sets the 3D unit vector value of a typed value object.
@param[in] typed_value    The typed value object.
@param[in] vector3d_value The 3 vector components. Magnitude is ignored.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
SU_RESULT SUTypedValueSetUnitVector3d(SUTypedValueRef typed_value, const double vector3d_value[3]);

/**
@brief  Retrieves the array of typed value objects from a typed value of type
    \ref SUTypedValueType_Array. Note that the returned \ref SUTypedValueRef
    objects will still be owned by their parent typed value array and therefore
    they must not be released by the caller.
@param[in]  typed_value The typed value object.
@param[in]  len         The length of the array to retrieve.
@param[out] values      The typed value objects retrieved.
@param[out] count       The actual number of typed value objects retrieved.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if values or count is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
SU_RESULT SUTypedValueGetArrayItems(
    SUTypedValueRef typed_value, size_t len, SUTypedValueRef values[], size_t* count);

/**
@brief  Sets the array of typed value objects of a typed value object.  The
        elements of the given array are copied to the type value object.
@param[in] typed_value The typed value object.
@param[in] len         The number of typed value objects to set.
@param[in] values      The array of typed value objects to set.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if values is NULL
*/
SU_RESULT SUTypedValueSetArrayItems(
    SUTypedValueRef typed_value, size_t len, SUTypedValueRef values[]);

/**
@brief  Retrieves the number of typed value objects from a typed value of type
    \ref SUTypedValueType_Array.
@param[in]  typed_value The typed value object.
@param[out] count       The number of typed value objects in the array.
@related SUTypedValueRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of type
  \ref SUTypedValueType_Array.
*/
SU_RESULT SUTypedValueGetNumArrayItems(SUTypedValueRef typed_value, size_t* count);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_TYPED_VALUE_H_
