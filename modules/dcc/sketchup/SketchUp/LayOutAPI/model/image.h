// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_IMAGE_H_
#define LAYOUT_MODEL_IMAGE_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOImageRef
@brief References a raster image entity.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a new image object from an image file on disk.
@param[out] image     The image object.
@param[in]  bounds    The bounding box for the image.
@param[in]  file_path The file path to the image on disk.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *image already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if file_path is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds has a width or height of zero
- \ref SU_ERROR_SERIALIZATION if there was an error reading the image file or
  allocating the memory for the image
- \ref SU_ERROR_NO_DATA if the image file could not be found
*/
LO_RESULT LOImageCreateFromFile(
    LOImageRef* image, const LOAxisAlignedRect2D* bounds, const char* file_path);

/**
@brief Adds a reference to an image object.
@param[in] image The image object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image does not refer to a valid object
*/
LO_RESULT LOImageAddReference(LOImageRef image);

/**
@brief Releases an image object. The object will be invalidated if
       releasing the last reference.
@param[in] image The image object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if image is NULL
- \ref SU_ERROR_INVALID_INPUT if *image does not refer to a valid object
*/
LO_RESULT LOImageRelease(LOImageRef* image);

/**
@brief Converts from a \ref LOEntityRef to a \ref LOImageRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LOImageRef.
@param[in] entity The entity object.
@return
- The converted \ref LOImageRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOImageRef LOImageFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOImageRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@param[in] image The image object.
@return
- The converted \ref LOEntityRef if image is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOImageToEntity(LOImageRef image);

/**
@brief Gets an image object's image representation.
@param[in]  image    The image object.
@param[out] imagerep The image representation object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if imagerep is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *imagerep already refers to a valid object
*/
LO_RESULT LOImageGetImageRep(LOImageRef image, LOImageRepRef* imagerep);

/**
@brief Gets an image object's image representation in the document's output resolution.

This should be used by exporters to retrieve the image data at the quality set by \ref
LOPageInfoSetImageOutputResolution.
@since LayOut 2023.1, API 8.1
@param[in]  image    The image object.
@param[out] imagerep The image representation object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p image does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p imagerep is NULL
- \ref SU_ERROR_OVERWRITE_VALID if \p imagerep already refers to a valid object
*/
LO_RESULT LOImageGetOutputImageRep(LOImageRef image, LOImageRepRef* imagerep);

/**
@brief Returns any clip mask assigned to the image.
@param[in]  image     The image object.
@param[out] clip_mask The clip mask of the image.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if clip_mask is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *clip_mask already refers to a valid object
- \ref SU_ERROR_NO_DATA if the image is not being clipped by a clip mask
*/
LO_RESULT LOImageGetClipMask(LOImageRef image, LOEntityRef* clip_mask);

/**
@brief Sets the clip mask of the image. A clip mask defines a region
       of the entity that is visible. This allows you to crop with arbitrary
       shapes. This operation will replace any clip mask that is already
       assigned to this image. The entity being used must not be already part
       of a document or group. The clip mask entity must be either a
       rectangle, ellipse or a path.
@note  Starting in LayOut 2020.1, API 5.1, clip_mask may be SU_INVALID, which
       will remove the existing clip mask, if any.
@since LayOut 2017, API 2.0
@param[in] image     The image object.
@param[in] clip_mask The new clip mask for the image.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if image does not refer to a valid object
- \ref SU_ERROR_INVALID_ARGUMENT if clip_mask is already in a document or group
- \ref SU_ERROR_UNSUPPORTED if clip_mask is not a rectangle, ellipse, or path
- \ref SU_ERROR_LAYER_LOCKED if image is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if image is locked
*/
LO_RESULT LOImageSetClipMask(LOImageRef image, LOEntityRef clip_mask);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus
#endif  // LAYOUT_MODEL_IMAGE_H_
