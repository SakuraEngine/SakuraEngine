// Copyright 2013 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Interfaces for SUStringRef.
 */
#ifndef SKETCHUP_UNICODESTRING_H_
#define SKETCHUP_UNICODESTRING_H_

#include <SketchUpAPI/defs.h>
#include <stddef.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUStringRef
@brief Stores a Unicode string for use as output string parameters in the API.
*/

/**
@brief Creates an empty string.

Constructs a string and initializes it to "", an empty string.
You must use SUStringRelease() on this string object to free its memory.
@param[out] out_string_ref The string object to be created.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p out_string_ref is `NULL`
- \ref SU_ERROR_OVERWRITE_VALID if *\p out_string_ref does not refer to an invalid
  object
*/
SU_RESULT SUStringCreate(SUStringRef* out_string_ref);

/**
@brief Creates a string from a UTF-8 string.

Constructs a string and initializes it to a copy of the provided string,
which is provided by a `'\0'` (`NULL`) terminated array of 8-bit characters.
This string is interpreted as UTF-8.
You must use SUStringRelease() on this string object to free its memory.
@param[out] out_string_ref The string object to be created
@param[in]  char_array     A NULL-terminated UTF-8 (or ASCII) string that
                           initializes the string.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if \p char_array is `NULL`
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p out_string_ref is `NULL`
- \ref SU_ERROR_OVERWRITE_VALID if *\p out_string_ref does not refer to an invalid
  object
*/
SU_RESULT SUStringCreateFromUTF8(SUStringRef* out_string_ref, const char* char_array);

/**
@brief Creates a string from a UTF-16 string.

Constructs a string and initializes it to a copy of the provided string,
which is provided by a 0 (`NULL`) terminated array of 16-bit characters.
This string is interpreted as UTF-16.
You must use SUStringRelease() on this string object to free its memory.
@param[out] out_string_ref The string object to be created
@param[in]  char_array     A NULL-terminated UTF-16 string that initializes the
                           string
@related SUStringRef
@return
- \ref SU_ERROR_NONE on Success
- \ref SU_ERROR_NULL_POINTER_INPUT if \p char_array is `NULL`
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p out_string_ref is `NULL`
- \ref SU_ERROR_OVERWRITE_VALID if *\p out_string_ref does not refer to an invalid
  object
*/
SU_RESULT SUStringCreateFromUTF16(SUStringRef* out_string_ref, const unichar* char_array);

/**
@brief Deletes a string object.

You must use SUStringRelease() when a SUStringRef object is no longer in use.
*string_ref is deleted and the reference is made invalid. (Calling
SUIsInvalid(*string_ref) would evaluate true.)
@param[in,out] string_ref The string object to be deleted.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if \p string_ref is `NULL`
- \ref SU_ERROR_INVALID_INPUT if *\p string_ref does not refer to a valid object
*/
SU_RESULT SUStringRelease(SUStringRef* string_ref);

/**
@brief Get the number of 8-bit characters required to store this string.

Gives you the length of the string when encoded in UTF-8. This may be
larger than the number of glyphs when multiple bytes are required.
This value does not include the space for a '\0' (`NULL`) terminator value
at the end of the string. It is a good idea when using this function with
SUStringGetUTF8() to add one to \p out_length.
@param[in]  string_ref The string object.
@param[out] out_length The length returned.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p string_ref does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p out_length is `NULL`
*/
SU_RESULT SUStringGetUTF8Length(SUStringRef string_ref, size_t* out_length);

/**
@brief Get the number of 16-bit characters required to store this string.

Gives you the length of the string when encoded in UTF-16. This may be
larger than the number of glyphs when multiple values are required.
This value does not include the space for a 0 (`NULL`) terminator value
at the end of the string. It is a good idea when using this function with
SUStringGetUTF16() to add one to \p out_length.

@param[in]  string_ref The string object.
@param[out] out_length The length returned.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p string_ref does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p out_length is `NULL`
*/
SU_RESULT SUStringGetUTF16Length(SUStringRef string_ref, size_t* out_length);

/**
@brief Sets the value of a string from a NULL-terminated UTF-8 character array.

@param[in] string_ref The string object.
@param[in] char_array The character array to be set.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p string_ref does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p char_array is `NULL`
*/
SU_RESULT SUStringSetUTF8(SUStringRef string_ref, const char* char_array);

/**
@brief Sets the value of a string from a NULL-terminated UTF-16 character array.

@param[in] string_ref The string object.
@param[in] char_array The character array to be set.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p string_ref does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p char_array is `NULL`
*/
SU_RESULT SUStringSetUTF16(SUStringRef string_ref, const unichar* char_array);

/**
@brief Writes the contents of the string into the provided character array.

This function does not allocate memory. You must provide an array of sufficient
length to get the entire string. The output string will be `NULL` terminated.
@param[in]  string_ref        The string object.
@param[in]  char_array_length The length of the given character array.
@param[out] out_char_array    The character array to be filled in.
@param[out] out_number_of_chars_copied The number of characters returned.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p string_ref does not refer to a valid string
- \ref SU_ERROR_NULL_POINTER_OUTPUT : \p out_char_array or
                                      \p out_number_of_chars_copied is `NULL`
*/
SU_RESULT SUStringGetUTF8(
    SUStringRef string_ref, size_t char_array_length, char* out_char_array,
    size_t* out_number_of_chars_copied);

/**
@brief Writes the contents of the string into the provided wide character array.

This function does not allocate memory. You must provide an array of sufficient
length to get the entire string. The output string will be NULL terminated.

@param[in]  string_ref        The string object.
@param[in]  char_array_length The length of the given character array.
@param[out] out_char_array    The character array to be filled in.
@param[out] out_number_of_chars_copied The number of characters returned.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if string_ref does not refer to a valid string
- \ref SU_ERROR_NULL_POINTER_OUTPUT : out_char_array or
                                      out_number_of_chars_copied is NULL
*/
SU_RESULT SUStringGetUTF16(
    SUStringRef string_ref, size_t char_array_length, unichar* out_char_array,
    size_t* out_number_of_chars_copied);

/**
@brief Trim leading white spaces from the string.
@since SketchUp 2017, API 5.0
@param[in]  string_ref The string object.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if string_ref does not refer to a valid string
*/
SU_RESULT SUStringTrimLeft(SUStringRef string_ref);

/**
@brief Trim tailing white spaces from the string.
@since SketchUp 2017, API 5.0
@param[in]  string_ref The string object.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if string_ref does not refer to a valid string
*/
SU_RESULT SUStringTrimRight(SUStringRef string_ref);

/**
@brief Compares two strings.
@since SketchUp 2017, API 5.0
@param[in]  a      The first string object.
@param[in]  b      The second string object.
@param[out] result The comparison result. 0 for equal, -1 for less than, 1 for
                     greater than.
@related SUStringRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if string_a or string_b do not refer to a valid string
- \ref SU_ERROR_NULL_POINTER_OUTPUT if result is NULL
*/
SU_RESULT SUStringCompare(SUStringRef a, SUStringRef b, int* result);

#ifdef __cplusplus
}  // extern "C" {
#endif
#pragma pack(pop)

#endif  // SKETCHUP_UNICODESTRING_H_
