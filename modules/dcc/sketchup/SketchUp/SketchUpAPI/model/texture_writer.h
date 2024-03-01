// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUTextureWriterRef.
 */
#ifndef SKETCHUP_MODEL_TEXTURE_WRITER_H_
#define SKETCHUP_MODEL_TEXTURE_WRITER_H_

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/face.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUTextureWriterRef
@brief  Used to write out textures of various SketchUp model elements to local
        disk.  For face objects texture writer modifies non-affine textures on
        write so that the resulting texture image can be mapped with
        2-dimensional texture coordinates.  The modified UV coordinates are
        retrieved from a mesh object created with
        SUMeshHelperCreateWithTextureWriter().
*/

/**
@brief  Creates a new texture writer object. The texture writer must be
        subsequently deallocated with SUTextureWriterRelease().
@param[out] writer The created texture writer object.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if writer is NULL
*/
SU_RESULT SUTextureWriterCreate(SUTextureWriterRef* writer);

/**
@brief  Deallocates a texture writer object.
@param[in] writer The texture writer object.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on return
- \ref SU_ERROR_INVALID_INPUT if writer does not reference a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if writer is NULL
*/
SU_RESULT SUTextureWriterRelease(SUTextureWriterRef* writer);

/**
@brief  Loads an entity to a texture writer object in order to have its texture
        written to disk. Acceptable entity types are:
        \ref SUComponentInstanceRef, \ref SUImageRef, \ref SUGroupRef and
        \ref SULayerRef.
@param[in]  writer     The texture writer object.
@param[in]  entity     The entity object.
@param[out] texture_id The id of the texture.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer or entity is not a valid object
- \ref SU_ERROR_GENERIC if entity is not one of the acceptable types or there is
  no texture to write
- \ref SU_ERROR_NULL_POINTER_OUTPUT if texture_id is NULL
*/
SU_RESULT SUTextureWriterLoadEntity(
    SUTextureWriterRef writer, SUEntityRef entity, long* texture_id);

/**
@brief  Loads a face object to a texture writer object in order to have its
        front and/or back texture written to local disk.
@param[in]  writer           The texture writer object.
@param[in]  face             The face object.
@param[out] front_texture_id The texture ID of the front texture.
@param[out] back_texture_id  The texture ID of the back texture.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer or face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if the face object has a front face texture
  to write, and front_texture_id is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if the face object has a back face texture
  to write, and back_texture_id is NULL
- \ref SU_ERROR_GENERIC if the face object does not a texture to write
*/
SU_RESULT SUTextureWriterLoadFace(
    SUTextureWriterRef writer, SUFaceRef face, long* front_texture_id, long* back_texture_id);

/**
@brief  Retrieves the total number of textures that are loaded into the texture
        writer object.
@param[in]  writer The texture writer object.
@param[out] count  The number of textures.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUTextureWriterGetNumTextures(SUTextureWriterRef writer, size_t* count);

/**
@brief  Writes a texture to a file on disk.
@param[in] writer      The texture writer object.
@param[in] texture_id  The id of the texture.
@param[in] path        The file location on disk to write the texture. If a file
                       is present at the location it is overwritten. The file
                       extension of the file path is indicates the file format.
                       The extension must be one of "jpg", "bmp", "tif", or
                       "png". Assumed to be UTF-8 encoded.
@param[in] reduce_size Indicates whether the texture image should be reduced in
                       size through scaling.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_SERIALIZATION if a file could not be written to the specified
  location, or if an invalid file format was specified
*/
SU_RESULT SUTextureWriterWriteTexture(
    SUTextureWriterRef writer, long texture_id, const char* path, bool reduce_size);

/**
 @brief  Retrieves an image from the given texture_id. The given image
         representation object must have been constructed using one of the
         SUImageRepCreate* functions. It must be released using
         SUImageRepRelease().
@since SketchUp 2017 M2, API 5.2
@param[in] writer      The texture writer object.
@param[in] texture_id  The id of the texture.
@param[out] image      The image object retrieved.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image is NULL
- \ref SU_ERROR_INVALID_OUTPUT if image is not a valid object.
- \ref SU_ERROR_NO_DATA if there is no texture in the given texture_id.
*/
SU_RESULT SUTextureWriterGetImageRep(
    SUTextureWriterRef writer, long texture_id, SUImageRepRef* image);

/**
@brief  Writes out all the textures loaded into a texture writer object. The
        file names and formats are those of the image file used to create the
        texture.  Preexisting files are overwritten.
@param[in] writer    The texture writer object.
@param[in] directory The directory on disk to write the textures. Assumed to be
                     UTF-8 encoded.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if directory is NULL
- \ref SU_ERROR_INVALID_INPUT if directory is not a object
- \ref SU_ERROR_SERIALIZATION if the textures could not be written to disk
*/
SU_RESULT SUTextureWriterWriteAllTextures(SUTextureWriterRef writer, const char* directory);

/**
@brief  Retrieves a flag indicating whether a texture object loaded into a
        texture writer object is linearly interpolated (affine) or perspective
        corrected.
@param[in]  writer     The texture writer object.
@param[in]  texture_id The id of the texture.
@param[out] is_affine  The affine flag retrieved.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer is not a valid object
- \ref SU_ERROR_NO_DATA if texture_id is not a handle to a loaded texture object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_affine is NULL
*/
SU_RESULT SUTextureWriterIsTextureAffine(
    SUTextureWriterRef writer, long texture_id, bool* is_affine);

/**
@brief  Retrieves the file path from a texture image written using \ref
        SUTextureWriterWriteAllTextures.
@param[in]  writer     The texture writer object.
@param[in]  texture_id The id of the texture.
@param[out] file_path  The file path retrieved.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer is not a valid object
- \ref SU_ERROR_NO_DATA if texture_id is not a handle to a loaded and written
  texture object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if file_path is NULL
- \ref SU_ERROR_INVALID_OUTPUT if file_path does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUTextureWriterGetTextureFilePath(
    SUTextureWriterRef writer, long texture_id, SUStringRef* file_path);

/**
@brief  Given an array of vertex positions, retrieves the corresponding UV
        coordinates of the front face texture of a face object that has been
        loaded into the given texture writer object.
@param[in]  writer    The texture writer object.
@param[in]  face      The face object.
@param[in]  len       The number of vertex positions.
@param[in]  points    The vertex positions.
@param[out] uv_coords The UV coordinates retrieved.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer or face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if uv_coords is NULL
- \ref SU_ERROR_NO_DATA if the face object does not have a front face texture
*/
SU_RESULT SUTextureWriterGetFrontFaceUVCoords(
    SUTextureWriterRef writer, SUFaceRef face, size_t len, const struct SUPoint3D points[],
    struct SUPoint2D uv_coords[]);

/**
@brief  Given an array of vertex positions, retrieves the corresponding UV
        coordinates of the back face texture of a face object that has been
        loaded into the given texture writer object.
@param[in]  writer    The texture writer object.
@param[in]  face      The face object.
@param[in]  len       The number of vertex positions.
@param[in]  points    The vertex positions.
@param[out] uv_coords The UV coordinates retrieved.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer or face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if uv_coords is NULL
- \ref SU_ERROR_NO_DATA if the face object does not have a back face texture
*/
SU_RESULT SUTextureWriterGetBackFaceUVCoords(
    SUTextureWriterRef writer, SUFaceRef face, size_t len, const struct SUPoint3D points[],
    struct SUPoint2D uv_coords[]);

/**
@brief  Gets the texture id of a previously loaded entity. Acceptable entity
        types are: \ref SUComponentInstanceRef, \ref SUImageRef, \ref SUGroupRef
        and \ref SULayerRef.
@param[in]  writer     The texture writer object.
@param[in]  entity     The entity object.
@param[out] texture_id The texture id retrieved.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer or entity is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if texture_id is NULL
- \ref SU_ERROR_GENERIC if the entity is not one of the acceptable types or it
  does not have a previously written texture_id
*/
SU_RESULT SUTextureWriterGetTextureIdForEntity(
    SUTextureWriterRef writer, SUEntityRef entity, long* texture_id);

/**
@brief  Gets the texture id of a previously loaded face.
@param[in]  writer The texture writer object.
@param[in]  face   The face object.
@param[in]  front  The side of the face we are interested in.  True if we want
                   texture for the front face, false if we want the texture for
                   the back face.
@param[out] texture_id The texture id retrieved.
@related SUTextureWriterRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if writer or face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if the face object has a texture, and
  texture_id is NULL
- \ref SU_ERROR_GENERIC if the face object does not have a previously written
  texture_id
*/
SU_RESULT SUTextureWriterGetTextureIdForFace(
    SUTextureWriterRef writer, SUFaceRef face, bool front, long* texture_id);

#ifdef __cplusplus
}  // #ifdef __cplusplus
#endif

#endif  // SKETCHUP_MODEL_TEXTURE_WRITER_H_
