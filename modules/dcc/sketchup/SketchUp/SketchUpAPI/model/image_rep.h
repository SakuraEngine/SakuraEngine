// Copyright 2016 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUImageRepRef.
 */
#ifndef SKETCHUP_MODEL_IMAGE_REP_H_
#define SKETCHUP_MODEL_IMAGE_REP_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUColorOrder
@brief A simple struct with four indices indicating the ordering of color data
       within 32-bit bitmap images. 32-bit bitmap images have 4 channels per
       pixel: red, green, blue, and alpha where the alpha channel indicates the
       transparency of the pixel. SketchUpAPI expects the channels to be in
       different orders on Windows vs. Mac OS. Bitmap data is exposed in BGRA
       and RGBA byte orders on Windows and Mac OS, respectively. The color
       order indices facilitate platform independent code when it is necessary
       to manipulate image pixel data. The struct's data also applies to 24-bit
       bitmap images except that such images don't have an alpha channel so the
       \ref SUColorOrder.alpha_index varaible can be ignored.
@since SketchUp 2017, API 5.2
*/
struct SUColorOrder {
  short red_index;    ///< Indicates the position of the red byte within a single
                      ///< pixel's data.
  short green_index;  ///< Indicates the position of the green byte within a
                      ///< single pixel's data.
  short blue_index;   ///< Indicates the position of the blue byte within a
                      ///< single pixel's data.
  short alpha_index;  ///< Indicates the position of the alpha byte within a
                      ///< single pixel's data.
};

/**
@brief Retrieves a \ref SUColorOrder indicating the order of color bytes within
       32-bit images' pixel data.
@since SketchUp 2017, API 5.2
@related SUColorOrder
@return a \ref SUColorOrder indicating the pixel color ordering of 32bit images.
*/
SU_EXPORT struct SUColorOrder SUGetColorOrder();

/**
@struct SUImageRepRef
@brief References an image representation object.
@since SketchUp 2017, API 5.0
*/

/**
@brief Creates a new image object with no image data. Image data can be set
       using any of SUImageRepCopy, SUImageRepSetData, SUImageRepLoadFile.
@since SketchUp 2017, API 5.0
@param[out] image The image object created.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image is NULL
- \ref SU_ERROR_OVERWRITE_VALID if image already references a valid object
*/
SU_RESULT SUImageRepCreate(SUImageRepRef* image);

/**
@brief Releases an image object.
@since SketchUp 2017, API 5.0
@param[in] image The image object.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if image is NULL
*/
SU_RESULT SUImageRepRelease(SUImageRepRef* image);

/**
@brief Copies data from the copy_image to image.
@since SketchUp 2017, API 5.0
@param[in,out] image      The image object to be altered.
@param[in]     copy_image The original image to copy from.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image or copy_image are not a valid objects
*/
SU_RESULT SUImageRepCopy(SUImageRepRef image, SUImageRepRef copy_image);

/**
@brief Sets the image data for the given image. Makes a copy of the data rather
       than taking ownership.
@since SketchUp 2017, API 5.0
@param[in,out] image          The image object used to load the data.
@param[in]     width          The width of the image in pixels.
@param[in]     height         The height of the image in pixels.
@param[in]     bits_per_pixel The number of bits per pixel.
@param[in]     row_padding    The size in Bytes of row padding in each row of
                              pixel_data.
@param[in]     pixel_data     The raw image data.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if pixel_data is NULL
- \ref SU_ERROR_OUT_OF_RANGE if width or height are 0
- \ref SU_ERROR_OUT_OF_RANGE if bits per pixel is not 8, 24, or 32
*/
SU_RESULT SUImageRepSetData(
    SUImageRepRef image, size_t width, size_t height, size_t bits_per_pixel, size_t row_padding,
    const SUByte pixel_data[]);

/**
@brief Loads image data from the specified file into the provided image.
@since SketchUp 2017, API 5.0
@param[in,out] image     The image object used to load the file.
@param[in]     file_path The file path of the source image file. Assumed to be
                         UTF-8 encoded.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if file_path is NULL
- \ref SU_ERROR_SERIALIZATION if the load operation fails
*/
SU_RESULT SUImageRepLoadFile(SUImageRepRef image, const char* file_path);

/**
@brief Saves an image object to an image file specified by a path.
@since SketchUp 2017, API 5.0
@param[in] image     The image object.
@param[in] file_path The file path of the destination image file. Assumed to be
                     UTF-8 encoded.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if file_path is NULL
- \ref SU_ERROR_NO_DATA if image contains no data
- \ref SU_ERROR_SERIALIZATION if the save operation fails
*/
SU_RESULT SUImageRepSaveToFile(SUImageRepRef image, const char* file_path);

/**
@brief Retrieves the width and height dimensions of an image object in pixels.
@since SketchUp 2017, API 5.0
@param[in]  image  The image object.
@param[out] width  The width dimension retrieved.
@param[out] height The height dimension retrieved.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if width or height is NULL
*/
SU_RESULT SUImageRepGetPixelDimensions(SUImageRepRef image, size_t* width, size_t* height);

/**
@brief Retrieves the size of the row padding of an image, in bytes.
@since SketchUp 2017, API 5.0
@param[in]  image           The image object.
@param[out] row_padding The row padding retrieved.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if row_padding is NULL
 */
SU_RESULT SUImageRepGetRowPadding(SUImageRepRef image, size_t* row_padding);

/**
@brief Resizes the dimensions of an image object to the given width and height
       in pixels.
@since SketchUp 2017, API 5.0
@param[in] image The image object.
@param[in] width  The new width.
@param[in] height The new height.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if width or height are 0
*/
SU_RESULT SUImageRepResize(SUImageRepRef image, size_t width, size_t height);

/**
@brief Converts an image object to be 32 bits per pixel.
@since SketchUp 2017, API 5.0
@param[in] image  The image object.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NO_DATA if image contains no data
*/
SU_RESULT SUImageRepConvertTo32BitsPerPixel(SUImageRepRef image);

/**
@brief  Returns the total size and bits-per-pixel value of an image. This
        function is useful to determine the size of the buffer necessary to be
        passed into \ref SUImageRepGetData(). The returned data can be used
        along with the returned bits-per-pixel value and the image dimensions to
        compute RGBA values at individual pixels of the image.
@since SketchUp 2017, API 5.0
@param[in]  image          The image object.
@param[out] data_size      The total size of the image data in bytes.
@param[out] bits_per_pixel The number of bits per pixel of the image data.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if data_size or bits_per_pixel is NULL
*/
SU_RESULT SUImageRepGetDataSize(SUImageRepRef image, size_t* data_size, size_t* bits_per_pixel);

/**
@brief Returns the pixel data for an image. The given array must be large
       enough to hold the image's data. This size can be obtained by calling
       \ref SUImageRepGetDataSize().
@since SketchUp 2017, API 5.0
@param[in]  image      The image object.
@param[in]  data_size  The size of the byte array.
@param[out] pixel_data The image data retrieved.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if pixel_data is NULL
- \ref SU_ERROR_INSUFFICIENT_SIZE if data_size is insufficient for the image
data
*/
SU_RESULT SUImageRepGetData(SUImageRepRef image, size_t data_size, SUByte pixel_data[]);

/**
@brief Returns the color data of an image in a \ref SUColor array.
@since SketchUp 2018, API 6.0
@param[in]  image      The image object.
@param[out] color_data The SUColor data retrieved.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color_data is NULL
- \ref SU_ERROR_UNSUPPORTED if the bytes per pixel of the image is invalid
*/
SU_RESULT SUImageRepGetDataAsColors(SUImageRepRef image, SUColor color_data[]);

/**
@brief Returns the color data given by the UV texture coordinates.
@since SketchUp 2018, API 6.0
@param[in]  image    The image object.
@param[in]  u        The U texture coordinate.
@param[in]  v        The V texture coordinate.
@param[in]  bilinear The flag to set bilinear texture filtering. This
                     interpolates the colors instead of picking the nearest
                     neighbor.
@param[out] color    The returned color.
@related SUImageRepRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
SU_RESULT SUImageRepGetColorAtUV(
    SUImageRepRef image, double u, double v, bool bilinear, SUColor* color);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_IMAGE_REP_H_
