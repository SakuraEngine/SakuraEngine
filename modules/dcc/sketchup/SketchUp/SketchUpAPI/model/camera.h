// Copyright 2013-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUCameraRef.
 */
#ifndef SKETCHUP_MODEL_CAMERA_H_
#define SKETCHUP_MODEL_CAMERA_H_

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUCameraRef
@brief  References the camera object of a SketchUp model.
*/

/**
@brief  Creates a default camera object.
@since SketchUp 2015, API 3.0
@param[out] camera The camera object created.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if camera is NULL
- \ref SU_ERROR_OVERWRITE_VALID if camera already refers to a valid object
*/
SU_RESULT SUCameraCreate(SUCameraRef* camera);

/**
@brief  Releases a camera object created by SUCameraCreate.
@since SketchUp 2015, API 3.0
@param[in] camera The camera object.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if camera is NULL
*/
SU_RESULT SUCameraRelease(SUCameraRef* camera);

/**
@brief Retrieves the orientation of a camera object.
@param[in]  camera    The camera object.
@param[out] position  The position retrieved.
@param[out] target    The target retrieved.
@param[out] up_vector The up direction retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if position, target, or up_vector is NULL
*/
SU_RESULT SUCameraGetOrientation(
    SUCameraRef camera, struct SUPoint3D* position, struct SUPoint3D* target,
    struct SUVector3D* up_vector);

/**
@brief Sets the position of a camera object.
@param[in] camera    The camera object.
@param[in] position  The new eye position.
@param[in] target    The new target position.
@param[in] up_vector The new up direction.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if position, target or up_vector is NULL
*/
SU_RESULT SUCameraSetOrientation(
    SUCameraRef camera, const struct SUPoint3D* position, const struct SUPoint3D* target,
    const struct SUVector3D* up_vector);

/**
@brief Retrieves the look at matrix of the camera object.
@since SketchUp 2017, API 5.0
@param[in]  camera         The camera object.
@param[out] transformation The look at matrix retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success.
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transformation is NULL
*/
SU_RESULT SUCameraGetViewTransformation(
    SUCameraRef camera, struct SUTransformation* transformation);

/**
@brief Sets the field of view angle of a camera object. If the camera object is
       an orthographic camera, the camera object subsequently becomes a
       perspective camera. The field of view is measured along the vertical
       direction of the camera.
@param[in] camera The camera object.
@param[in] fov    The field of view angle in degrees.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
*/
SU_RESULT SUCameraSetPerspectiveFrustumFOV(SUCameraRef camera, double fov);

/**
@brief Retrieves the field of view in degrees of a camera object. The field of
       view is measured along the vertical direction of the camera.
@param[in] camera The camera object.
@param[out] fov   The field of view retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NO_DATA if camera is not a perspective camera (orthographic
       camera)
- \ref SU_ERROR_NULL_POINTER_OUTPUT if fov is NULL
*/
SU_RESULT SUCameraGetPerspectiveFrustumFOV(SUCameraRef camera, double* fov);

/**
@brief Sets the aspect ratio of a camera object.
@since SketchUp 2017, API 5.0
@param[in]  camera       The camera object.
@param[out] aspect_ratio The aspect ratio to be set.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE is aspect_ratio < 0.0
*/
SU_RESULT SUCameraSetAspectRatio(SUCameraRef camera, double aspect_ratio);

/**
@brief Retrieves the aspect ratio of a camera object.
@param[in]  camera       The camera object.
@param[out] aspect_ratio The aspect ratio retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NO_DATA if the camera uses the screen aspect ratio
- \ref SU_ERROR_NULL_POINTER_OUTPUT if aspect_ratio is NULL
*/
SU_RESULT SUCameraGetAspectRatio(SUCameraRef camera, double* aspect_ratio);

/**
@brief Sets the height of a camera object which is used to calculate the
       orthographic projection of a camera object. If the camera object is a
       perspective camera, the camera subsequently becomes an orthographic
       camera.
@param[in] camera The camera object.
@param[in] height The height of the camera view.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
*/
SU_RESULT SUCameraSetOrthographicFrustumHeight(SUCameraRef camera, double height);

/**
@brief Retrieves the height of an orthographic camera object.
@param[in]  camera The camera object.
@param[out] height The height retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NO_DATA if camera is not an orthographic camera (perspective
       camera)
- \ref SU_ERROR_NULL_POINTER_OUTPUT if height is NULL
*/
SU_RESULT SUCameraGetOrthographicFrustumHeight(SUCameraRef camera, double* height);

/**
@brief Sets a camera object perspective or orthographic.
@param[in] camera      The camera object.
@param[in] perspective The perspective flag.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
*/
SU_RESULT SUCameraSetPerspective(SUCameraRef camera, bool perspective);

/**
@brief Retrieves whether a camera object is a perspective camera or not
       (i.e. orthographic).
@param[in]  camera      The camera object.
@param[out] perspective The perspective flag retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT perspective is NULL
*/
SU_RESULT SUCameraGetPerspective(SUCameraRef camera, bool* perspective);

/**
@brief Retrieves the near and far clipping distances of the camera object.
@since SketchUp 2017, API 5.0
@param[in]  camera The camera object.
@param[out] znear  The near clipping distance.
@param[out] zfar   The far clipping distance.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if znear or zfar is NULL
*/
SU_RESULT SUCameraGetClippingDistances(SUCameraRef camera, double* znear, double* zfar);

/**
@brief Sets whether the field of view value represents the camera view height.
@since SketchUp 2017, API 5.0
@param[in] camera        The camera object.
@param[in] is_fov_height The field of view flag set.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
*/
SU_RESULT SUCameraSetFOVIsHeight(SUCameraRef camera, bool is_fov_height);

/**
@brief Retrieves whether the field of view value represents the camera view
       height.
@since SketchUp 2017, API 5.0
@param[in]  camera        The camera object.
@param[out] is_fov_height The field of view flag retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT is_fov_height is NULL
*/
SU_RESULT SUCameraGetFOVIsHeight(SUCameraRef camera, bool* is_fov_height);

/**
@brief Sets the size of the image on the "film" for a perspective camera. The
       value is given in millimeters. It is used in the conversions between
       field of view and focal length.
@since SketchUp 2017, API 5.0
@param[in] camera The camera object.
@param[in] width  The width set in millimeters.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
*/
SU_RESULT SUCameraSetImageWidth(SUCameraRef camera, double width);

/**
@brief Retrieves the size of the image on the image plane of the Camera. By
       default, this value is not set. If it is set, it is used in the
       calculation of the focal length from the field of view. Unlike most
       length values in SketchUp, this width is specified in millimeters rather
       than in inches.
@since SketchUp 2017, API 5.0
@param[in]  camera The camera object.
@param[out] width  The image width retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT width is NULL
*/
SU_RESULT SUCameraGetImageWidth(SUCameraRef camera, double* width);

/**
@brief Sets the description of a camera object.
@since SketchUp 2017, API 5.0
@param[in] camera The camera object.
@param[in] desc   The description to be set.
Assumed to be UTF-8 encoded.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if desc is NULL
*/
SU_RESULT SUCameraSetDescription(SUCameraRef camera, const char* desc);

/**
@brief Retrieves the description of a camera object.
@since SketchUp 2017, API 5.0
@param[in]  camera The camera object.
@param[out] desc   The description retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if desc is NULL
- \ref SU_ERROR_INVALID_OUTPUT if desc does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUCameraGetDescription(SUCameraRef camera, SUStringRef* desc);

/**
@brief Retrieves the camera's direction vector.
@since SketchUp 2017, API 5.0
@param[in]  camera    The camera object.
@param[out] direction The direction vector retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if direction is NULL
*/
SU_RESULT SUCameraGetDirection(SUCameraRef camera, struct SUVector3D* direction);

/**
@brief Sets whether a camera is two dimensional. 2 point perspective mode and
       PhotoMatch mode are 2d cameras.
@since SketchUp 2017, API 5.0
@param[in] camera  The camera object.
@param[in] make_2d The flag for specifying if the camera should be 2D.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
*/
SU_RESULT SUCameraSet2D(SUCameraRef camera, bool make_2d);

/**
@brief Retrieves whether a camera object is two dimensional.
@since SketchUp 2017, API 5.0
@param[in]  camera The camera object.
@param[out] is_2d  The 2D flag retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT is_2d is NULL
*/
SU_RESULT SUCameraGet2D(SUCameraRef camera, bool* is_2d);

/**
@brief Sets the camera's 2D scale factor.
@since SketchUp 2017, API 5.0
@param[in] camera The camera object.
@param[in] scale  The scale to be set.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
*/
SU_RESULT SUCameraSetScale2D(SUCameraRef camera, double scale);

/**
@brief Retrieves the camera's 2D scale factor.
@since SketchUp 2017, API 5.0
@param[in]  camera The camera object.
@param[out] scale  The scale factor retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if scale is NULL
*/
SU_RESULT SUCameraGetScale2D(SUCameraRef camera, double* scale);

/**
@brief Sets the camera's 2D center point. The point coordinates are in screen
       space.  Since this is setting the 2D center point the z component of the
       provided point is ignored.
@since SketchUp 2017, API 5.0
@param[in] camera The camera object.
@param[in] center The center to be set.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if center is NULL
*/
SU_RESULT SUCameraSetCenter2D(SUCameraRef camera, const struct SUPoint3D* center);

/**
@brief Retrieves the camera's 2D center point. Since this is accessing a 2D
       point with a 3D point structure the z coordinate is always set to 0.0.
@since SketchUp 2017, API 5.0
@param[in]  camera The camera object.
@param[out] center The center point retrieved.
@related SUCameraRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if camera is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if center is NULL
*/
SU_RESULT SUCameraGetCenter2D(SUCameraRef camera, struct SUPoint3D* center);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_CAMERA_H_
