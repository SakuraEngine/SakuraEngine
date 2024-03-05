// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_IMAGEREP_H_
#define LAYOUT_MODEL_IMAGEREP_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>
#include <SketchUpAPI/color.h>

/**
@enum LOImageRepOutputFormat
@brief Represents the file formats that an image representation can be saved as
       when saving to a file.
*/
typedef enum {
  LOImageRepOutputFormat_PNG = 0,  ///< PNG file format.
  LOImageRepOutputFormat_JPG,      ///< JPG file format.
  LONumImageRepOutputFormats
} LOImageRepOutputFormat;

/**
@struct LOImageRepRef
@brief References the bitmap representation for a raster image.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Gets the dimensions in pixels of an image representation.
@param[in]  imagerep The image representation object.
@param[out] width    The width of the image.
@param[out] height   The height of the image.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if imagerep does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if width is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if height is NULL
*/
LO_RESULT LOImageRepGetPixelDimensions(LOImageRepRef imagerep, size_t* width, size_t* height);

/**
@brief Gets the DPI of an image representation.
@since LayOut 2018, API 3.0
@param[in]  imagerep The image representation object.
@param[out] x_dpi    The DPI in horizontal direction.
@param[out] y_dpi    The DPI in vertical direction.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if imagerep does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if x_dpi is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if y_dpi is NULL
*/
LO_RESULT LOImageRepGetDPI(LOImageRepRef imagerep, double* x_dpi, double* y_dpi);

/**
@brief  Returns the total size and bits-per-pixel value of an image
       representation. This function is useful to determine the size of the
       buffer necessary to be passed into \ref LOImageRepGetData. The returned
       data can be used along with the returned bits-per-pixel value and the
       image dimensions to compute RGBA values at individual pixels of the
       image.
@param[in]  imagerep       The image representation object.
@param[out] data_size      The total size of the image data in bytes.
@param[out] bits_per_pixel The number of bits per pixel of the image data.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if imagerep is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if data_size or bits_per_pixel is NULL
*/
LO_RESULT LOImageRepGetDataSize(LOImageRepRef imagerep, size_t* data_size, size_t* bits_per_pixel);

/**
@brief Returns the pixel data for an image representation. The given array
       must be large enough to hold the image representation's data. This size
       can be obtained by calling \ref LOImageRepGetDataSize.
@param[in]  imagerep   The image representation object.
@param[in]  data_size  The size of the byte array.
@param[out] pixel_data The image data retrieved.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if imagerep is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if pixel_data is NULL
- \ref SU_ERROR_INSUFFICIENT_SIZE if data_size is insufficient for the image
  data
*/
LO_RESULT LOImageRepGetData(LOImageRepRef imagerep, size_t data_size, SUByte pixel_data[]);

/**
@brief Saves the image representation to the file at the indicated path.
@param[in] imagerep The image representation object.
@param[in] filename The file path to where the image should go on disk.
@param[in] format   What format to save the image as.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if imagerep is invalid
- \ref SU_ERROR_NULL_POINTER_INPUT if filename is NULL
- \ref SU_ERROR_SERIALIZATION if there was an error writing the image file
*/
LO_RESULT LOImageRepSaveAs(
    LOImageRepRef imagerep, const char* filename, LOImageRepOutputFormat format);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_IMAGEREP_H_
