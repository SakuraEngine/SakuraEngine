// Copyright 2013 Trimble Inc.  All Rights Reserved

/**
 * @file
 * @brief Interfaces for SUImageRef.
 */
#ifndef SKETCHUP_MODEL_IMAGE_H_
#define SKETCHUP_MODEL_IMAGE_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/transformation.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUImageRef
@extends SUDrawingElementRef
@brief References an image object.
*/

/**
@brief Converts from an \ref SUImageRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] image The given image reference.
@related SUImageRef
@return
- The converted \ref SUEntityRef if image is a valid image.
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUEntityRef SUImageToEntity(SUImageRef image);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUImageRef.
       This is essentially a downcast operation so the given entity must be
       convertible to an \ref SUImageRef.
@param[in] entity The given entity reference.
@related SUImageRef
@return
- The converted \ref SUImageRef if the downcast operation succeeds. If not, the
returned reference will be invalid
*/
SU_EXPORT SUImageRef SUImageFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUImageRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@param[in] image The given image reference.
@related SUImageRef
@return
- The converted \ref SUEntityRef if image is a valid image
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef SUImageToDrawingElement(SUImageRef image);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref SUImageRef.
       This is essentially a downcast operation so the given element must be
       convertible to an \ref SUImageRef.
@param[in] drawing_elem The given element reference.
@related SUImageRef
@return
- The converted \ref SUImageRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUImageRef SUImageFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Creates a new image object from an image file specified by a path.
       The created image must be subsequently added to the Entities of a model,
       component definition or a group. Use SUModelRemoveComponentDefinitions()
       to remove the image from a model.
@param[out] image     The image object created.
@param[in]  file_path The file path of the source image file.
                      Assumed to be UTF-8 encoded.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if file_path is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image is NULL
- \ref SU_ERROR_OVERWRITE_VALID if image already references a valid object
*/
SU_RESULT SUImageCreateFromFile(SUImageRef* image, const char* file_path);

/**
@brief Creates a new SketchUp model image object from an image representation
       object. The created image must be subsequently added to the Entities of
       a model, component definition or a group. Use
       SUModelRemoveComponentDefinitions() to remove the image from a model.
@since SketchUp 2017, API 5.0
@param[out] image     The image object created.
@param[in]  image_rep The basic image object retrieved.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image is NULL
- \ref SU_ERROR_OVERWRITE_VALID if image already references a valid object
- \ref SU_ERROR_INVALID_INPUT if image_rep is not a valid object
- \ref SU_ERROR_NO_DATA if the image_rep contains no data
*/
SU_RESULT SUImageCreateFromImageRep(SUImageRef* image, SUImageRepRef image_rep);

/**
@brief Retrieves a basic image from a SketchUp model image.  The given image
       representation object must have been constructed using one of the
       SUImageRepCreate* functions. It must be released using
       SUImageRepRelease().
afterwards.
@since SketchUp 2017, API 5.0
@param[in]  image        The texture object.
@param[out] image_rep The basic image object retrieved.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image_rep is NULL
- \ref SU_ERROR_INVALID_OUTPUT if image_rep does not point to a valid \ref
       SUImageRepRef object
- \ref SU_ERROR_NO_DATA if there was no image data
*/
SU_RESULT SUImageGetImageRep(SUImageRef image, SUImageRepRef* image_rep);

/**
@brief Retrieves the name of an image object.
@param[in]  image The image object.
@param[out] name  The name retrieved.
@related SUImageRef
@deprecated This function returns a property that should not exist.
            Use SUImageGetDefinition() and SUComponentDefinitionGetName()
            instead.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_DEPRECATED_FUNCTION("SketchUp API 9.1")
SU_RESULT SUImageGetName(SUImageRef image, SUStringRef* name);

/**
@brief Sets the name of an image object.
@param[in] image The image object.
@param[in] name  The name to set. Assumed to be UTF-8 encoded.
@related SUImageRef
@deprecated This function sets a property that should not exist.
            Use SUImageGetDefinition() and SUComponentDefinitionSetName()
            instead.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
*/
SU_DEPRECATED_FUNCTION("SketchUp API 9.1")
SU_RESULT SUImageSetName(SUImageRef image, const char* name);

/**
@brief Retrieves the 3-dimensional homogeneous transform of an image object.
@param[in]  image     The image object.
@param[out] transform The transform retrieved.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
*/
SU_RESULT SUImageGetTransform(SUImageRef image, struct SUTransformation* transform);

/**
@brief Sets the 3-dimensional homogeneous transform of an image object.
@param[in] image     The image object.
@param[in] transform The affine transform to set.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
*/
SU_RESULT SUImageSetTransform(SUImageRef image, const struct SUTransformation* transform);

/**
@brief Retrieves the image file name of an image object.
@param[in]  image     The image object.
@param[out] file_name The image file name retrieved.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if file_name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if file_name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUImageGetFileName(SUImageRef image, SUStringRef* file_name);

/**
@brief Retrieves the world dimensions of an image object.
@param[in]  image  The image object.
@param[out] width  The width dimension retrieved.
@param[out] height The height dimension retrieved.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if width or height is NULL
*/
SU_RESULT SUImageGetDimensions(SUImageRef image, double* width, double* height);

/**
@brief Retrieves the component definition of an image object.
@since SketchUp 2021.1, API 9.1
@param[in]  image     The image object.
@param[out] component The component definition retrieved.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p image is not a valid object
- \ref SU_ERROR_OVERWRITE_VALID if \p component already refers to a valid object.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p component is NULL
*/
SU_RESULT SUImageGetDefinition(SUImageRef image, SUComponentDefinitionRef* component);

/**
@brief Retrieves the width and height dimensions of an image object in pixels.
@deprecated Will be removed in the next version of the SketchUp API. The
            functionality is replaced by SUImageGetImageRep followed by
            SUImageRepGetPixelDimensions.
@param[in]  image  The image object.
@param[out] width  The width dimension retrieved.
@param[out] height The height dimension retrieved.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if width or height is NULL
*/
SU_DEPRECATED_FUNCTION("SketchUp API 5.0")
SU_RESULT SUImageGetPixelDimensions(SUImageRef image, size_t* width, size_t* height);

/**
@brief  Returns the total size and bits-per-pixel value of an image. This
        function is useful to determine the size of the buffer necessary to be
        passed into \ref SUImageGetData(). The returned data can be used along
        with the returned bits-per-pixel value and the image dimensions to
        compute RGBA values at individual pixels of the image.
@deprecated Will be removed in the next version of the SketchUp API. The
            functionality is replaced by SUImageGetImageRep followed by
            SUImageRepGetDataSize.
@param[in]  image          The image object.
@param[out] data_size      The total size of the image data in bytes.
@param[out] bits_per_pixel The number of bits per pixel of the image data.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if data_size or bits_per_pixel is NULL
*/
SU_DEPRECATED_FUNCTION("SketchUp API 5.0")
SU_RESULT SUImageGetDataSize(SUImageRef image, size_t* data_size, size_t* bits_per_pixel);

/**
@brief  Returns the pixel data for an image. The given array must be large enough
        to hold the image's data. This size can be obtained by calling
        \ref SUImageGetDataSize().
@deprecated Will be removed in the next version of the SketchUp API. The
            functionality is replaced by SUImageGetImageRep followed by
            SUImageRepGetData.
@param[in]  image      The image object.
@param[in]  data_size  The size of the byte array.
@param[out] pixel_data The image data retrieved.
@related SUImageRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if pixel_data is NULL
- \ref SU_ERROR_INSUFFICIENT_SIZE if data_size is insufficient for the image
  data
*/
SU_DEPRECATED_FUNCTION("SketchUp API 5.0")
SU_RESULT SUImageGetData(SUImageRef image, size_t data_size, SUByte pixel_data[]);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_IMAGE_H_
