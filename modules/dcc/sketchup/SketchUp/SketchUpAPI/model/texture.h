// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUTextureRef.
 */
#ifndef SKETCHUP_MODEL_TEXTURE_H_
#define SKETCHUP_MODEL_TEXTURE_H_

#include <stddef.h>
#include <SketchUpAPI/color.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUTextureRef
@extends SUEntityRef
@brief  Used to manage image data that can be associated with any \ref
        SUEntityRef.
*/

/**
@brief  Converts from an \ref SUTextureRef to an \ref SUEntityRef. This is
        essentially an upcast operation.
@param[in] texture The texture reference.
@related SUTextureRef
@return
- The converted \ref SUEntityRef if texture is a valid object. If not, the
  returned reference will be invalid.
*/
SU_EXPORT SUEntityRef SUTextureToEntity(SUTextureRef texture);

/**
@brief  Converts from an \ref SUEntityRef to an \ref SUTextureRef. This is
        essentially a downcast operation so the given \ref SUEntityRef must be
        convertible to an \ref SUTextureRef.
@param[in] entity The entity reference.
@related SUTextureRef
@return
- The converted \ref SUTextureRef if the downcast operation succeeds. If
  not, the returned reference will be invalid.
*/
SU_EXPORT SUTextureRef SUTextureFromEntity(SUEntityRef entity);

/**
@brief  Creates a new texture object with the specified image data.  If the
        texture object is not subsequently associated with a parent object (e.g.
        material), then the texture object must be deallocated with \ref
        SUTextureRelease.
@deprecated Will be removed in the next version of the SketchUp API. The
            functionality is replaced by SUImageRepSetData() followed by
            SUTextureCreateFromImageRep().
@param[out] texture        The texture object created.
@param[in]  width          The width in pixels of the texture data.
@param[in]  height         The height in pixels of the texture data.
@param[in]  bits_per_pixel The number of bits per pixel of the image data.
@param[in]  pixel_data     The source of the pixel data.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if pixels is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT texture is NULL
- \ref SU_ERROR_OVERWRITE_VALID if texture already references a valid object
*/
SU_DEPRECATED_FUNCTION("SketchUp API 5.0")
SU_RESULT SUTextureCreateFromImageData(
    SUTextureRef* texture, size_t width, size_t height, size_t bits_per_pixel,
    const SUByte pixel_data[]);

/**
@brief Creates a new texture object from an image representation object.
@since SketchUp 2017, API 5.0
@param[out] texture The texture object created.
@param[in]  image   The image retrieved.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if texture is NULL
- \ref SU_ERROR_OVERWRITE_VALID if texture already references a valid object
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NO_DATA if the image contains no data
*/
SU_RESULT SUTextureCreateFromImageRep(SUTextureRef* texture, SUImageRepRef image);

/**
@brief  Creates a new texture object from an image file specified by a path.
        If the texture object is not subsequently associated with a parent
        object (e.g. material), then the texture object must be deallocated with
        SUTextureRelease.
@param[out] texture   The texture object created.
@param[in]  file_path The file path of the source image file. Assumed to be
                      UTF-8 encoded.
@param[in]  s_scale   The scale factor for s coordinate value.
@param[in]  t_scale   The scale factor for t coordinate value.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if file_path is NULL
- \ref SU_ERROR_SERIALIZATION if the image file could not be opened
- \ref SU_ERROR_GENERIC if a texture could not be created from the image file
- \ref SU_ERROR_NULL_POINTER_OUTPUT if texture is NULL
- \ref SU_ERROR_OVERWRITE_VALID is texture already references a valid object
*/
SU_RESULT SUTextureCreateFromFile(
    SUTextureRef* texture, const char* file_path, double s_scale, double t_scale);

/**
@brief  Deallocates a texture object and its resources. If the texture object
        is associated with a parent object (e.g. material) the parent object
        handles the deallocation of the resources of the texture object and the
        texture object must not be explicitly deallocated.
@param[in] texture The texture object to deallocate.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if texture is NULL
- \ref SU_ERROR_INVALID_INPUT if texture does not refer to a valid object
*/
SU_RESULT SUTextureRelease(SUTextureRef* texture);

/**
@brief  Retrieves the pixel width, height, and scale factors of the texture.
        The s_scale and t_scale values are useful when a face doesn't have a
        material applied directly, but instead inherit from a parent group or
        component instance. Then you want use these values to multiply the
        result of SUMeshHelperGetFrontSTQCoords() or SUUVHelperGetFrontUVQ().
        If the material is applied directly then this would not be needed.
@param[in] texture  The texture object whose dimensions are retrieved.
@param[out] width   The width in pixels.
@param[out] height  The height in pixels.
@param[out] s_scale The s coordinate scale factor to map a pixel into model
                    coordinates.
@param[out] t_scale The t coordinate scale factor to map a pixel into model
                    coordinates.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if width, height, s_scale, or t_scale is
  NULL
*/
SU_RESULT SUTextureGetDimensions(
    SUTextureRef texture, size_t* width, size_t* height, double* s_scale, double* t_scale);

/**
@brief  Returns the total size and bits-per-pixel value of a texture's image
        data. This function is useful to determine the size of the buffer
        necessary to be passed into \ref SUTextureGetImageData(). The returned
        data can be used along with the returned bits-per-pixel value and the
        texture dimensions to compute RGBA values at individual pixels of the
        texture image.
@deprecated Will be removed in the next version of the SketchUp API. The
            functionality is replaced by SUTextureGetImageRep followed by
            SUImageRepGetImageDataSize.
@param[in]  texture        The texture object.
@param[out] data_size      The total size of the image data in bytes.
@param[out] bits_per_pixel The number of bits per pixel of the image data.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if data_size or bits_per_pixel is NULL
*/
SU_DEPRECATED_FUNCTION("SketchUp API 5.0")
SU_RESULT SUTextureGetImageDataSize(
    SUTextureRef texture, size_t* data_size, size_t* bits_per_pixel);

/**
@brief  Returns the texture's image data. The given array must be large enough
        to hold the texture's image data. This size can be obtained by calling
        \ref SUTextureGetImageDataSize().
@deprecated Will be removed in the next version of the SketchUp API. The
            functionality is replaced by SUTextureGetImageRep followed by
            SUImageRepGetImageData.
@param[in] texture     The texture object.
@param[in] data_size   The size of the byte array.
@param[out] pixel_data The image data retrieved.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if pixel_data is NULL
- \ref SU_ERROR_INSUFFICIENT_SIZE if data_size is insufficient for the image
  data
*/
SU_DEPRECATED_FUNCTION("SketchUp API 5.0")
SU_RESULT SUTextureGetImageData(SUTextureRef texture, size_t data_size, SUByte pixel_data[]);

/**
@brief Retrieves a texture's image.  The given image object must have been
       constructed using one of the SUImageRepCreate*  functions. It must be
       released using SUImageRepRelease(). The difference between this
       function and SUTextureGetColorizedImageRep() is that
       SUTextureGetColorizedImageRep() will retrieve the colorized image
       rep, if the material has been colorized.
@since SketchUp 2017, API 5.0
@param[in]  texture The texture object.
@param[out] image The image object retrieved.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image is NULL
- \ref SU_ERROR_INVALID_OUTPUT if image does not point to a valid \ref
       SUImageRepRef object
- \ref SU_ERROR_NO_DATA if no image was created
*/
SU_RESULT SUTextureGetImageRep(SUTextureRef texture, SUImageRepRef* image);

/**
@brief  Writes a texture object as an image to disk. If the material has been
        colorized this will write out a colorized texture.
        Use \ref SUTextureWriteOriginalToFile() to obtain the original texture
        without colorization.
@param[in] texture   The texture object.
@param[in] file_path The file path destination of the texture image. Assumed to
                     be UTF-8 encoded.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if file_path is NULL
- \ref SU_ERROR_SERIALIZATION if image file could not be written to disk
*/
SU_RESULT SUTextureWriteToFile(SUTextureRef texture, const char* file_path);

/**
@brief Sets the image file name string associated with the texture object. If
       the input texture was constructed using \ref SUTextureCreateFromFile the
       name will already be set, so calling this function will override the
       texture's file name string.
@since SketchUp 2017, API 5.0
@param[in] texture The texture object.
@param[in] name    The name string to set as the file name.
Assumed to be UTF-8 encoded.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
*/
SU_RESULT SUTextureSetFileName(SUTextureRef texture, const char* name);

/**
@brief Retrieves the image file basename of a texture object. A full path may be
       stored with the texture, but this method will always return a file name
       string with no path. If the texture was created from an
       \ref SUImageRepRef created with SUImageRepLoadFile() then this will
       return only the file extension representing the file format of the image
       data (e.g. ".png").
@param[in]  texture   The texture object.
@param[out] file_name The file name retrieved.
@related SUTextureRef
@see SUTextureGetFilePath
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is not a valid object
- \ref SU_ERROR_GENERIC if the texture is an unknown file type
- \ref SU_ERROR_NULL_POINTER_OUTPUT if file_name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if file_name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUTextureGetFileName(SUTextureRef texture, SUStringRef* file_name);

/**
@brief Retrieves the image file path of a texture object.

If the texture was created from an \ref SUImageRepRef created with SUImageRepLoadFile()
then this will return only the file extension representing the file format of the image
data (e.g. ".png").

@param[in]  texture   The texture object.
@param[out] file_path The file path retrieved.
@related SUTextureRef
@since SketchUp 2023.1, API 11.1
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p texture is not a valid object
- \ref SU_ERROR_GENERIC if the \p texture is an unknown file type
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p file_path is NULL
- \ref SU_ERROR_INVALID_OUTPUT if \p file_path does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUTextureGetFilePath(SUTextureRef texture, SUStringRef* file_path);

/**
@brief  Retrieves the value of the flag that indicates whether a texture object
        uses the alpha channel.
@param[in] texture             The texture object.
@param[out] alpha_channel_used The destination of the retrieved value.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if alpha_channel_used is NULL
- \ref SU_ERROR_NO_DATA if the flag value could not be retrieved
*/
SU_RESULT SUTextureGetUseAlphaChannel(SUTextureRef texture, bool* alpha_channel_used);

/**
@brief  Retrieves the average color for the texture.
@param[in]  texture     The texture object
@param[out] color_val   The color object
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color_val is NULL
*/
SU_RESULT SUTextureGetAverageColor(SUTextureRef texture, SUColor* color_val);

/**
@brief  Retrieves the image rep object of a colorized texture. If a
        non-colorized texture is used, then the original image rep will be
        retrieved. The difference between this function and
        \ref SUTextureGetImageRep is that \ref SUTextureGetImageRep() will always
        retrieve the original image rep.
@since SketchUp 2018, API 6.0
@param[in]  texture   The texture object.
@param[out] image_rep The retrieved image rep.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image_rep is NULL
- \ref SU_ERROR_NO_DATA if no data was retrieved from the texture
- \ref SU_ERROR_OUT_OF_RANGE if the pixel data is out of range
*/
SU_RESULT SUTextureGetColorizedImageRep(SUTextureRef texture, SUImageRepRef* image_rep);

/**
@brief  Writes a texture object as an image to disk without any colorization.
        If the texture was created from a file on disk this will write out the
        original file data if the provided file extension matches. This will be
        the fastest way to extract the original texture from the model.
        Use SUTextureGetFilename() to obtain the original file format.
@since SketchUp 2019.2, API 7.1
@param[in] texture   The texture object.
@param[in] file_path The file path destination of the texture image. Assumed to
                     be UTF-8 encoded.
@related SUTextureRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if texture is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if file_path is NULL
- \ref SU_ERROR_SERIALIZATION if image file could not be written to disk
*/
SU_RESULT SUTextureWriteOriginalToFile(SUTextureRef texture, const char* file_path);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_TEXTURE_H_
