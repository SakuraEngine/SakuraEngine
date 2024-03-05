// Copyright 2017 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Interfaces for SUTransformation2D.
 */
#ifndef SKETCHUP_GEOMETRY_TRANSFORMATION2D_H_
#define SKETCHUP_GEOMETRY_TRANSFORMATION2D_H_

#include <SketchUpAPI/geometry.h>

#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a translation transformation using the given vector.
@since SketchUp 2019, API 7.0
@param[out] transform The transformation to be set.
@param[in]  vector    The 2D vector specifying the translation for the
                      transformation.
@related SUTransformation2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
*/
SU_RESULT SUTransformation2DTranslation(
    struct SUTransformation2D* transform, const struct SUVector2D* vector);

/**
@brief Creates a scale transformation using the given scale value.
@since SketchUp 2019, API 7.0
@param[out] transform The transformation to be set.
@param[in]  scale     The scale value for the transformation.
@related SUTransformation2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
*/
SU_RESULT SUTransformation2DScale(struct SUTransformation2D* transform, double scale);

/**
@brief Creates a scale transformation using the given scale values.
@since SketchUp 2019, API 7.0
@param[out] transform The transformation to be set.
@param[in]  x_scale   The x-axis scale value for the transformation.
@param[in]  y_scale   The y-axis scale value for the transformation.
@related SUTransformation2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if x_scale or y_scale are NULL
*/
SU_RESULT SUTransformation2DNonUniformScale(
    struct SUTransformation2D* transform, double x_scale, double y_scale);

/**
@brief Creates a scale transformation using the given scale value and origin.
@since SketchUp 2019, API 7.0
@param[out] transform The transformation to be set.
@param[in]  point     The point specifying the translation component of the
                      transformation.
@param[in]  scale     The scale value for the transformation.
@related SUTransformation2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if point or scale are NULL
*/
SU_RESULT SUTransformation2DScaleAboutPoint(
    struct SUTransformation2D* transform, const struct SUPoint2D* point, double scale);

/**
@brief Creates a scale transformation using the given scale values and origin.
@since SketchUp 2019, API 7.0
@param[out] transform The transformation to be set.
@param[in]  point     The point specifying the translation component of the
                      transformation.
@param[in]  x_scale   The x-axis scale value for the transformation.
@param[in]  y_scale   The y-axis scale value for the transformation.
@related SUTransformation2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if point, x_scale, or y_scale are NULL
*/
SU_RESULT SUTransformation2DNonUniformScaleAboutPoint(
    struct SUTransformation2D* transform, const struct SUPoint2D* point, double x_scale,
    double y_scale);

/**
@brief Creates a transformation given a point and angle.
@since SketchUp 2019, API 7.0
@param[out] transform The calculated transformation.
@param[in]  point     The point specifying the translation component of the
                      transformation.
@param[in]  angle     The rotation in radians for the transformation.
@related SUTransformation2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_identity is NULL
- \ref SU_ERROR_OUT_OF_RANGE if weight is not between 0.0 and 1.0
*/
SU_RESULT SUTransformation2DRotation(
    struct SUTransformation2D* transform, const struct SUPoint2D* point, double angle);

/**
@brief Gets whether the transformation is an identity transformation.
@since SketchUp 2019, API 7.0
@param[in]  transform   The transformation object.
@param[out] is_identity Whether the transformation is identity.
@related SUTransformation2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_identity is NULL
*/
SU_RESULT SUTransformation2DIsIdentity(
    const struct SUTransformation2D* transform, bool* is_identity);

/**
@brief Gets the inverse transformation of the given transformation object.
@since SketchUp 2019, API 7.0
@param[in]  transform The transformation object.
@param[out] inverse   The inverse transformation object.
@related SUTransformation2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if inverse is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if the transform cannot be inverted
*/
SU_RESULT SUTransformation2DGetInverse(
    const struct SUTransformation2D* transform, struct SUTransformation2D* inverse);

/**
@brief Multiplies a transformation by another transformation.
@since SketchUp 2019, API 7.0
@param[in]   transform1    The transformation object to be multiplied.
@param[in]   transform2    The transformation object to multiply by.
@param[out]  out_transform The result of the matrix multiplication
                           [transform1 * transform2].
@related SUTransformation2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if out_transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if transform1 or transform2 are NULL
*/
SU_RESULT SUTransformation2DMultiply(
    const struct SUTransformation2D* transform1, const struct SUTransformation2D* transform2,
    struct SUTransformation2D* out_transform);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#pragma pack(pop)

#endif  // SKETCHUP_GEOMETRY_TRANSFORMATION2D_H_