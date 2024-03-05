// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_TYPED_VALUE_H_
#define LAYOUT_MODEL_TYPED_VALUE_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOTypedValueRef
@brief References a variant object used to represent a value of an arbitrary
       type.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@enum LOTypedValueType
@brief Defines the set of types that a \ref LOTypedValueRef can represent.
*/
typedef enum {
  LOTypedValueType_Empty = 0,
  LOTypedValueType_Bool,
  LOTypedValueType_Int32,
  LOTypedValueType_Double,
  LOTypedValueType_String,
  LONumTypedValueTypes
} LOTypedValueType;

/**
@brief Creates a new typed value object.
@param[out] typed_value The created typed value object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if typed_value is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *typed_value already refers to a valid object
*/
LO_RESULT LOTypedValueCreate(LOTypedValueRef* typed_value);

/**
@brief Releases a typed value object. *typed_value will be set to invalid by
       this function.
@param[in] typed_value The typed value object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if typed_value is NULL
- \ref SU_ERROR_INVALID_INPUT if *typed_value references an invalid object
*/
LO_RESULT LOTypedValueRelease(LOTypedValueRef* typed_value);

/**
@brief Gets the type of value stored by a typed value object.
@param[in]  typed_value The typed value object.
@param[out] type        The type of value stored by typed_value.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if type_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
LO_RESULT LOTypedValueGetType(LOTypedValueRef typed_value, LOTypedValueType* type);

/**
@brief Gets the boolean value of a typed value object.
@param[in]  typed_value The typed value object.
@param[out] bool_value  The boolean value.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if bool_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
LO_RESULT LOTypedValueGetBool(LOTypedValueRef typed_value, bool* bool_value);

/**
@brief Sets the boolean value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] bool_value  The boolean value to set.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
LO_RESULT LOTypedValueSetBool(LOTypedValueRef typed_value, bool bool_value);

/**
@brief Gets the int32 value of a typed value object.
@param[in]  typed_value The typed value object.
@param[out] int32_value The int32 value.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if int32_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
LO_RESULT LOTypedValueGetInt32(LOTypedValueRef typed_value, int32_t* int32_value);

/**
@brief Sets the int32 value of a typed value object.
@param[in] typed_value The typed value object.
@param[in] int32_value The int32 value to set.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
LO_RESULT LOTypedValueSetInt32(LOTypedValueRef typed_value, int32_t int32_value);

/**
@brief Gets the double value of a typed value object.
@param[in]  typed_value  The typed value object.
@param[out] double_value  The double value.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if double_value is NULL
- \ref SU_ERROR_NO_DATA if typed_value is not of the requested type
*/
LO_RESULT LOTypedValueGetDouble(LOTypedValueRef typed_value, double* double_value);

/**
@brief Sets the double value of a typed value object.
@param[in] typed_value  The typed value object.
@param[in] double_value The double value to set.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value is not a valid object
*/
LO_RESULT LOTypedValueSetDouble(LOTypedValueRef typed_value, double double_value);

/**
@brief Gets a string value from the typed value object.
@since LayOut 2018, API 3.0
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if typed_value does not refer to a valid object
- \ref SU_ERROR_NO_DATA if the typed value does not contain a string.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if out_string is NULL
- \ref SU_ERROR_INVALID_OUTPUT if out_string does not refer to a valid object
*/
LO_RESULT LOTypedValueGetString(LOTypedValueRef typed_value, SUStringRef* out_string);

/**
@brief Sets the value of typed_value to the given string.
@since LayOut 2018, API 3.0
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if string is NULL
*/
LO_RESULT LOTypedValueSetString(LOTypedValueRef typed_value, const char* string);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_TYPED_VALUE_H_
