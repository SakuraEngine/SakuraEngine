// Copyright 2013 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Interfaces for SUColor.
 */
#ifndef SKETCHUP_COLOR_H_
#define SKETCHUP_COLOR_H_

#include <SketchUpAPI/unicodestring.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@typedef SUByte
@brief A type that stores a byte.
*/
typedef unsigned char SUByte;

/**
@struct SUColor
@brief Stores a RGBA color with 8 bit channels.
*/
typedef struct {
  SUByte red;    ///< Red color channel
  SUByte green;  ///< Green color channel
  SUByte blue;   ///< Blue color channel
  SUByte alpha;  ///< Alpha color channel
} SUColor;

/**
@brief The blend method is used to blend two colors.
       The blended color will be the result of taking
       (1 - weight) * sucolor1 + weight * sucolor2.
@since SketchUp 2018, API 6.0
@param[in]  color1      A \ref SUColor to blend color2 with.
@param[in]  color2      A \ref SUColor to blend color1 with.
@param[in]  weight      A value that determines the weight
@param[out] blended_color The blended \ref SUColor.
@related SUColor
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_OUT_OF_RANGE if weight is out of range
- \ref SU_ERROR_NULL_POINTER_OUTPUT if blended_color is NULL
*/
SU_RESULT SUColorBlend(
    const SUColor color1, const SUColor color2, const double weight, SUColor* blended_color);

/**
@brief Retrieves the number of color names recognized by SketchUp.
@since SketchUp 2018, API 6.0
@param[out] size The number of color names.
@related SUColor
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if size is NULL
*/
SU_RESULT SUColorGetNumNames(size_t* size);

/**
@brief Retrives all the color names recognized by SketchUp.
@since SketchUp 2018, API 6.0
@param[out] names An array of all the SketchUp Color names.
@param[in]  size  The size of the array.
@related SUColor
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if names is NULL
*/
SU_RESULT SUColorGetNames(SUStringRef names[], const size_t size);

/**
@brief Sets the color represented by the name.
@since SketchUp 2018, API 6.0
@param[out] color   The struct representing the color.
@param[in]  name    The string representing the color.
@related SUColor
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if name is empty
*/
SU_RESULT SUColorSetByName(SUColor* color, const char* name);

/**
@brief Sets the color with the provided value. The passed in value can either be
       integer or hexadecimal. Alpha will always be 255 but RGB. For example:
       if the value is 0x66ccff, rgb will be (102, 204, 255) respectively.
@since SketchUp 2018, API 6.0
@param[out] color   The struct representing the color.
@param[in]  value   A value that represents the color.
@related SUColor
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
SU_RESULT SUColorSetByValue(SUColor* color, const size_t value);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus
#pragma pack(pop)

#endif  // SKETCHUP_COLOR_H_
