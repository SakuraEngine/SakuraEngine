// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUMaterialRef.
 */
#ifndef SKETCHUP_MODEL_MATERIAL_H_
#define SKETCHUP_MODEL_MATERIAL_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUMaterialRef
@extends SUEntityRef
@brief  References a material object.
*/


/**
@enum SUMaterialType
@brief Indicates material type.
*/
enum SUMaterialType {
  SUMaterialType_Colored = 0,      ///< Colored material
  SUMaterialType_Textured,         ///< Textured material
  SUMaterialType_ColorizedTexture  ///< Colored and textured material
};

/**
@enum SUMaterialOwnerType
@brief Indicates material owner type.
@since SketchUp 2019.2, API 7.1
*/
enum SUMaterialOwnerType {
  SUMaterialOwnerType_None = 0,        ///< Not owned
  SUMaterialOwnerType_DrawingElement,  ///< Can be applied to SUDrawingElements
  SUMaterialOwnerType_Image,           ///< Owned exclusively by an Image
  SUMaterialOwnerType_Layer            ///< Owned exclusively by a Layer
};

/**
@enum SUMaterialColorizeType
@brief Indicates material type.
@since SketchUp 2019.2, API 7.1
*/
enum SUMaterialColorizeType {
  SUMaterialColorizeType_Shift = 0,  ///< Shifts the texture's Hue
  SUMaterialColorizeType_Tint,       ///< Colorize the texture
};

/**
@brief Converts from an \ref SUMaterialRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] material The given material reference.
@related SUMaterialRef
@return
- The converted \ref SUEntityRef if material is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUMaterialToEntity(SUMaterialRef material);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUMaterialRef.
       This is essentially a downcast operation so the given SUEntityRef must be
       convertible to an \ref SUMaterialRef.
@param[in] entity The given entity reference.
@related SUMaterialRef
@return
- The converted \ref SUMaterialRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUMaterialRef SUMaterialFromEntity(SUEntityRef entity);

/**
@brief Creates a material.

If the material is not associated with any face, it must be deallocated with
\ref SUMaterialRelease().
@param[out] material The material created.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_OUTPUT if the input parameter is NULL
*/
SU_RESULT SUMaterialCreate(SUMaterialRef* material);

/**
@brief Releases a material and its resources.

The material must not be associated with a parent object such as a face.
@param[in] material The material to be released.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT material is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if material is NULL
*/
SU_RESULT SUMaterialRelease(SUMaterialRef* material);

/**
@brief Sets the name of a material object.

@warning Breaking Change: A new failure mode was added in SketchUp 2018,
         API 6.0. Returns \ref SU_ERROR_INVALID_ARGUMENT if the material is
         managed by a model and the provided name was previously associated
         with a different material in the model.

@param[in] material The material object.
@param[in] name     The name to set the material name. Assumed to be UTF-8
                    encoded.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if material is managed and name is not unique
*/
SU_RESULT SUMaterialSetName(SUMaterialRef material, const char* name);

/**
@brief Retrieves the internal name of a material object. The internal name is
       the unprocessed  identifier string stored with the material.

@warning Breaking Change: The behavior of this method was changed in
         SketchUp 2017, API 5.0. In previous releases this method retrieved the
         material's non-localized display name but it was changed to retrieve
         the internal name. If the old functionality is required, use \ref
         SUMaterialGetNameLegacyBehavior.

@param[in]  material The material object.
@param[out] name     The name retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUMaterialGetName(SUMaterialRef material, SUStringRef* name);

/**
@brief Retrieves the name of a material object. This method was added for users
       who require the functionality of \ref SUMaterialGetName() prior to
       SketchUp 2017, API 5.0. If the internal name is encased in square
       brackets, [], this method will return the name without brackets,
       otherwise the name will match the name retrieved by \ref
       SUMaterialGetName.
@since SketchUp 2017, API 5.0
@param[in]  material The material object.
@param[out] name     The name retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUMaterialGetNameLegacyBehavior(SUMaterialRef material, SUStringRef* name);

/**
@brief Sets the color of a material object.
@param[in] material The material object.
@param[in] color    The color value to set the material color.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if color is NULL
*/
SU_RESULT SUMaterialSetColor(SUMaterialRef material, const SUColor* color);

/**
@brief Retrieves the color value of a material object.
@param[in]  material The material object.
@param[out] color    The color value retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
- \ref SU_ERROR_NO_DATA if the material object does not have a color value
*/
SU_RESULT SUMaterialGetColor(SUMaterialRef material, SUColor* color);

/**
@brief Sets the texture of a material object. Materials take ownership of their
       assigned textures, so textures should not be shared accross different
       materials.
@param[in] material The material object.
@param[in] texture  The texture object to set the material texture.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material or texture is not a valid object
- \ref SU_ERROR_GENERIC if texture contains invalid image data
*/
SU_RESULT SUMaterialSetTexture(SUMaterialRef material, SUTextureRef texture);

/**
@brief Retrieves the texture of a material object.
@param[in]  material The material object.
@param[out] texture  The texture object retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if texture is NULL
- \ref SU_ERROR_INVALID_OUTPUT if texture is not a valid object
- \ref SU_ERROR_NO_DATA if the material object does not have a texture
*/
SU_RESULT SUMaterialGetTexture(SUMaterialRef material, SUTextureRef* texture);

/**
@brief Retrieves the alpha value (0.0 - 1.0) of a material object.
@param[in]  material The material object.
@param[out] alpha    The alpha value retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if alpha is NULL
*/
SU_RESULT SUMaterialGetOpacity(SUMaterialRef material, double* alpha);

/**
@brief Sets the alpha value of a material object.
@param[in] material The material object.
@param[in] alpha    The alpha value to set. Must be within range [0.0, 1.0].
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if alpha is not within the acceptable range
*/
SU_RESULT SUMaterialSetOpacity(SUMaterialRef material, double alpha);

/**
@brief Retrieves the flag indicating whether alpha values are used from a
       material object.
@param[in]  material    The material object.
@param[out] use_opacity The flag retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if use_opacity is NULL
*/
SU_RESULT SUMaterialGetUseOpacity(SUMaterialRef material, bool* use_opacity);

/**
@brief Sets the flag indicating whether alpha values are used on a material
       object.
@param[in] material    The material object.
@param[in] use_opacity The flag boolean value to set.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
*/
SU_RESULT SUMaterialSetUseOpacity(SUMaterialRef material, bool use_opacity);

/**
@brief Sets the type of a material object.
@param[in] material The material object.
@param[in] type     The type to set.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
*/
SU_RESULT SUMaterialSetType(SUMaterialRef material, enum SUMaterialType type);

/**
@brief Retrieves the type of a material object.
@param[in]  material The material object.
@param[out] type     The type retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
SU_RESULT SUMaterialGetType(SUMaterialRef material, enum SUMaterialType* type);

/**
@brief Retrieves the flag indicating whether the material is drawn with
       transparency.
@since SketchUp 2018, API 6.0
@param[in]  material     The material object.
@param[out] transparency The flag retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_drawn_transparent is NULL
*/
SU_RESULT SUMaterialIsDrawnTransparent(SUMaterialRef material, bool* transparency);

/**
@brief Retrieves the owner type of a material object.

@warning Materials owned by SUImageRef and SULayerRef may not be applied
         to any other entity in the model.

@since SketchUp 2019.2, API 7.1
@param[in]  material The material object.
@param[out] type     The type retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
SU_RESULT SUMaterialGetOwnerType(SUMaterialRef material, enum SUMaterialOwnerType* type);

/**
@brief Sets the colorization type of a material object. This is used when the
       material's color is set to a custom value. Call this function after
       calling SUMaterialSetColor as otherwise the colorize type will be reset.
@since SketchUp 2019.2, API 7.1
@param[in] material The material object.
@param[in] type     The type to set.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_INVALID_ARGUMENT if type is not a valid value
*/
SU_RESULT SUMaterialSetColorizeType(SUMaterialRef material, enum SUMaterialColorizeType type);

/**
@brief Retrieves the colorization type of a material object.
@since SketchUp 2019.2, API 7.1
@param[in]  material The material object.
@param[out] type     The type retrieved.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
SU_RESULT SUMaterialGetColorizeType(SUMaterialRef material, enum SUMaterialColorizeType* type);

/**
@brief The colorize_deltas method retrieves the HLS deltas for colorized
       materials.
@since SketchUp 2019.2, API 7.1
@param[in]  material   The material object.
@param[out] hue        The Hue delta.
@param[out] saturation The Saturation delta.
@param[out] lightness  The Lightness delta.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if material is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if either hue, saturation or lightness
                                    is NULL
*/
SU_RESULT SUMaterialGetColorizeDeltas(
    SUMaterialRef material, double* hue, double* saturation, double* lightness);
/**
@brief Writes a material to a SKM file.
@since SketchUp 2021.1, API 9.1
@param[in]  material   The material object.
@param[in]  file_path  The location to save the material to. Assumed to be
                       UTF-8 encoded.
@related SUMaterialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p material is not a valid object
- \ref SU_ERROR_INVALID_INPUT if \p material is not attached to a model
- \ref SU_ERROR_NULL_POINTER_INPUT if \p file_path is NULL
- \ref SU_ERROR_SERIALIZATION if the serialization operation itself fails
*/
SU_RESULT SUMaterialWriteToFile(SUMaterialRef material, const char* file_path);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_MATERIAL_H_
