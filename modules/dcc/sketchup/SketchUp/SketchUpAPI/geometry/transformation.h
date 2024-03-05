// Copyright 2017 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Interfaces for SUTransformation.
 */
#ifndef SKETCHUP_GEOMETRY_TRANSFORMATION_H_
#define SKETCHUP_GEOMETRY_TRANSFORMATION_H_

#include <SketchUpAPI/geometry.h>

#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Sets the transformation based on the provided point and z-axis vector.
       The resulting transformation transforms points/vectors to a new
       coordinate system where the provided point is the new origin and the
       vector is the new z-axis. The other two axes in the transformed space
       are computed using the "Arbitrary axis algorithm".
@since SketchUp 2017, API 5.0
@param[out] transform The transformation to be set.
@param[in]  point     The point specifying the translation component of the
                      transformation.
@param[in]  normal    The 3D vector specifying the rotation component of the
                      transformation. This is treated as a unit vector, so any
                      scaling will be ignored.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if point or normal are NULL
*/
SU_RESULT SUTransformationSetFromPointAndNormal(
    struct SUTransformation* transform, const struct SUPoint3D* point,
    const struct SUVector3D* normal);

/**
@brief Sets the transformation based on the provided origin and axes.
@since SketchUp 2018, API 6.0
@param[out] transform The transformation to be set.
@param[in]  point     The point specifying the translation component of the
                      transformation.
@param[in]  x_axis    The 3D vector specifying the x-axis for the transformation.
                      This is treated as a unit vector, so any scaling will be
                      ignored.
@param[in]  y_axis    The 3D vector specifying the y-axis for the transformation.
                      This is treated as a unit vector, so any scaling will be
                      ignored.
@param[in]  z_axis    The 3D vector specifying the z-axis for the transformation.
                      This is treated as a unit vector, so any scaling will be
                      ignored.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if point, x_axis, y_axis, or z_axis are NULL
*/
SU_RESULT SUTransformationSetFromPointAndAxes(
    struct SUTransformation* transform, const struct SUPoint3D* point,
    const struct SUVector3D* x_axis, const struct SUVector3D* y_axis,
    const struct SUVector3D* z_axis);

/**
@brief Creates a translation transformation using the given vector.
@since SketchUp 2018, API 6.0
@param[out] transform  The transformation to be set.
@param[in]  vector     The 3D vector specifying the translation for the
                       transformation.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
*/
SU_RESULT SUTransformationTranslation(
    struct SUTransformation* transform, const struct SUVector3D* vector);

/**
@brief Creates a scale transformation using the given scale value.
@since SketchUp 2018, API 6.0
@param[out] transform  The transformation to be set.
@param[in]  scale      The scale value for the transformation.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
*/
SU_RESULT SUTransformationScale(struct SUTransformation* transform, double scale);

/**
@brief Creates a scale transformation using the given scale values.
@since SketchUp 2018, API 6.0
@param[out] transform  The transformation to be set.
@param[in]  x_scale    The x-axis scale value for the transformation.
@param[in]  y_scale    The y-axis scale value for the transformation.
@param[in]  z_scale    The z-axis scale value for the transformation.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if x_scale, y_scale, or z_scale are NULL
*/
SU_RESULT SUTransformationNonUniformScale(
    struct SUTransformation* transform, double x_scale, double y_scale, double z_scale);

/**
@brief Creates a scale transformation using the given scale value and origin.
@since SketchUp 2018, API 6.0
@param[out] transform  The transformation to be set.
@param[in]  point      The point specifying the translation component of the
                       transformation.
@param[in]  scale      The scale value for the transformation.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if point or scale are NULL
*/
SU_RESULT SUTransformationScaleAboutPoint(
    struct SUTransformation* transform, const struct SUPoint3D* point, double scale);

/**
@brief Creates a scale transformation using the given scale values and origin.
@since SketchUp 2018, API 6.0
@param[out] transform  The transformation to be set.
@param[in]  point      The point specifying the translation component of the
                       transformation.
@param[in]  x_scale    The x-axis scale value for the transformation.
@param[in]  y_scale    The y-axis scale value for the transformation.
@param[in]  z_scale    The z-axis scale value for the transformation.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if point, x_scale, y_scale, or z_scale are
  NULL
*/
SU_RESULT SUTransformationNonUniformScaleAboutPoint(
    struct SUTransformation* transform, const struct SUPoint3D* point, double x_scale,
    double y_scale, double z_scale);

/**
@brief Creates a transformation given an origin, vector of rotation, and angle.
@since SketchUp 2018, API 6.0
@param[out] transform  The calculated transformation.
@param[in]  point      The point specifying the translation component of the
                       transformation.
@param[in]  vector     The vector about which rotation will occur.
@param[in]  angle      The rotation in radians for the transformation.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if point or vector is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_OUT_OF_RANGE if weight is not between 0.0 and 1.0
*/
SU_RESULT SUTransformationRotation(
    struct SUTransformation* transform, const struct SUPoint3D* point,
    const struct SUVector3D* vector, double angle);

/**
@brief Performs an interpolation between two transformations. The weight
       determines the amount of interpolation. A weight of 0.0 would return
       a transformation of t1, while a weight of 1.0 would return a
       transformation of t2.
@since SketchUp 2018, API 6.0
@param[out] transform  The result of the interpolation.
@param[in]  t1         The first transformation object.
@param[in]  t2         The second transformation object.
@param[in]  weight     The weight determines the amount of interpolation from
                       t1 to t2.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if t1 or t2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_OUT_OF_RANGE if weight is not between 0.0 and 1.0
*/
SU_RESULT SUTransformationInterpolate(
    struct SUTransformation* transform, const struct SUTransformation* t1,
    const struct SUTransformation* t2, double weight);

/**
@brief Gets whether the transformation is an identity transformation.
@since SketchUp 2018, API 6.0
@param[in]  transform   The transformation object.
@param[out] is_identity Whether the transformation is identity.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_identity is NULL
*/
SU_RESULT SUTransformationIsIdentity(const struct SUTransformation* transform, bool* is_identity);

/**
@brief Gets the inverse transformation of the given transformation object.
@since SketchUp 2018, API 6.0
@param[in]  transform The transformation object.
@param[out] inverse   The inverse transformation object.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if inverse is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if the transform cannot be inverted
*/
SU_RESULT SUTransformationGetInverse(
    const struct SUTransformation* transform, struct SUTransformation* inverse);

/**
@brief Gets the origin point of the given transformation object.
@since SketchUp 2018, API 6.0
@param[in]  transform   The transformation object.
@param[out] origin      The origin point to be retrieved.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if origin is NULL
*/
SU_RESULT SUTransformationGetOrigin(
    const struct SUTransformation* transform, struct SUPoint3D* origin);

/**
@brief Gets the x axis vector of the given transformation object.
@since SketchUp 2018, API 6.0
@param[in]  transform   The transformation object.
@param[out] x_axis      The x axis vector to be retrieved.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if x_axis is NULL
*/
SU_RESULT SUTransformationGetXAxis(
    const struct SUTransformation* transform, struct SUVector3D* x_axis);

/**
@brief Gets the y axis vector of the given transformation object.
@since SketchUp 2018, API 6.0
@param[in]  transform   The transformation object.
@param[out] y_axis      The y axis vector to be retrieved.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if y_axis is NULL
*/
SU_RESULT SUTransformationGetYAxis(
    const struct SUTransformation* transform, struct SUVector3D* y_axis);

/**
@brief Gets the z_axis vector of the given transformation object.
@since SketchUp 2018, API 6.0
@param[in]  transform   The transformation object.
@param[out] z_axis      The z_axis vector to be retrieved.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if z_axis is NULL
*/
SU_RESULT SUTransformationGetZAxis(
    const struct SUTransformation* transform, struct SUVector3D* z_axis);

/**
@brief Gets the rotation about the z axis from the given transformation object.
@since SketchUp 2018, API 6.0
@param[in]  transform   The transformation object.
@param[out] z_rotation  The rotation to be retrieved.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if z_rotation is NULL
*/
SU_RESULT SUTransformationGetZRotation(
    const struct SUTransformation* transform, double* z_rotation);

/**
@brief Multiplies a transformation by another transformation.
@since SketchUp 2018, API 6.0
@param[in]   transform1     The transformation object to be multiplied.
@param[in]   transform2     The transformation object to multiply by.
@param[out]  out_transform  The result of the matrix multiplication
                            [transform1 * transform2].
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if out_transform is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if transform1 or transform2 is NULL
*/
SU_RESULT SUTransformationMultiply(
    const struct SUTransformation* transform1, const struct SUTransformation* transform2,
    struct SUTransformation* out_transform);

/**
@brief Returns true if transformation has been mirrored.
@since SketchUp 2019, API 7.0
@param[in]  transform    The transform object.
@param[out] is_mirrored  Indicates if mirrored.
@related SUTransformation
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_mirrored is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
 */
SU_RESULT SUTransformationIsMirrored(const struct SUTransformation* transform, bool* is_mirrored);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#pragma pack(pop)

#endif  // SKETCHUP_GEOMETRY_TRANSFORMATION_H_
