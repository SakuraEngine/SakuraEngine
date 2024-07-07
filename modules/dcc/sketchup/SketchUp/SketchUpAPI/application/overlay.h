// Copyright 2023 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief An Overlay provides a way to render an image over the SU rendering.
 */
#ifndef SKETCHUP_APPLICATION_OVERLAY_H_
#define SKETCHUP_APPLICATION_OVERLAY_H_

#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/geometry.h>

#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUOverlayRef
@brief Manages a Overlay object, which is used to blend external images on top of SketchUp
rendering.
*/

/**
@brief Signature for SUOverlayCreateInfo::get_extents callback
@related SUOverlayRef
*/
typedef struct SUBoundingBox3D (*SUOverlayGetExtentsFuncT)(SUOverlayRef, void*);

/**
@brief Available versions for the SUOverlayCreateInfo::version field.
@related SUOverlayRef
*/
enum SUOverlayBeginFrameInfoVersion : uint32_t {
  SUOVERLAY_BEGIN_FRAME_INFO_VERSION_1 = 1,
};

/**
@brief Current version of the SUBeginFrameInfo struct
@related SUOverlayRef
*/
#define SUOVERLAY_BEGIN_FRAME_INFO_VERSION (SUOVERLAY_BEGIN_FRAME_INFO_VERSION_1)

/**
@struct SUBeginFrameInfo
@brief Information about the frame camera and viewport that is passed to
SUOverlayCreateInfo::begin_frame
@related SUOverlayRef
*/
struct SUBeginFrameInfo {
  /// Version of SUBeginFrameInfo struct. This will be set by SketchUp.
  enum SUOverlayBeginFrameInfoVersion version;

  struct SUPoint3D position;      ///< Camera position
  struct SUPoint3D target;        ///< Camera target position
  struct SUVector3D up;           ///< Camera up direction
  double aspect_ratio;            ///< Aspect ratio of the view port
  double near_clipping_distance;  ///< Distance to near clipping plane from the camera
  double far_clipping_distance;   ///< Distance to far clipping plane from the camera
  double projection_matrix[16];   ///< Projection Matrix.
  double view_matrix[16];         ///< View Matrix
  double viewport_width;          ///< Viewport width
  double viewport_height;         ///< Viewport height

  double fov;     ///< Field of view of the perspective camera (in degrees). Valid only when
                  ///< is_perspective is true.
  double height;  ///< Parallel projection frustum height. Valid only when is_perspective is false.
  bool is_perspective;  ///< Whether the camera is perspective or orthographic.

  void* reserved;  ///< Reserved for internal use. This will be set to NULL by SketchUp and should
                   ///< not be modified.
};

/**
@brief Signature for SUOverlayCreateInfo::begin_frame callback
@related SUOverlayRef
*/
typedef void (*SUOverlayBeginFrameFuncT)(SUOverlayRef, const struct SUBeginFrameInfo*, void*);

/**
@brief Available versions for SUOverlayDrawFrameInfo::version
@related SUOverlayRef
*/
enum SUOverlayDrawFrameInfoVersion : uint32_t { SUOVERLAY_DRAW_FRAME_INFO_VERSION_1 = 1 };

/**
@brief Current version of SUOverlayDrawFrameInfo struct
@related SUOverlayRef
*/
#define SUOVERLAY_DRAW_FRAME_INFO_VERSION (SUOVERLAY_DRAW_FRAME_INFO_VERSION_1)

/**
@struct SUDrawFrameMemoryBuffer
@brief Memory buffer (for color and depth) info used in SUOverlayDrawFrameInfo
@related SUOverlayRef
*/
struct SUDrawFrameMemoryBuffer {
  /// Pointer to the buffer
  void* ptr;
  /// Size (in bytes) from the beginning of one line of the buffer to the next line
  uint32_t row_pitch;
  /// Total size (in bytes) of the buffer
  uint32_t size;
};

/**
@brief Supported image formats
@related SUOverlayRef
*/
enum SUOverlayImageFormat : uint8_t {
  SUOVERLAY_IMAGE_FORMAT_RGBA = 0,
  SUOVERLAY_IMAGE_FORMAT_BGRA = 1,
};

/**
@brief Supported image orientations
@related SUOverlayRef
*/
enum SUOverlayImageOrientation : uint8_t {
  SUOVERLAY_IMAGE_ORIENTATION_TOP_DOWN = 0,
  SUOVERLAY_IMAGE_ORIENTATION_BOTTOM_UP = 1,
};

/**
@struct SUOverlayDrawFrameInfo
@brief Data that SketchUp will use the to blend the given color/depth buffer onto SketchUp
rendering.
@related SUOverlayRef
*/
struct SUOverlayDrawFrameInfo {
  /// Version of the struct. This will be set by SketchUp.
  enum SUOverlayDrawFrameInfoVersion version;

  /// Alpha channel of the given color buffer will be multiplied with this value.
  /// Must be set to 1.0 if an override is not needed.
  double blending_factor;

  /// Color buffer info. ptr points to a uint8_t buffer with 4 channels.
  struct SUDrawFrameMemoryBuffer color;

  /// Depth buffer info. ptr points to a float buffer.
  struct SUDrawFrameMemoryBuffer depth;

  /// Reserved for internal use. This will be set to NULL by SketchUp and should not be modified.
  void* reserved;
};

/**
@brief Signature for SUOverlayCreateInfo::draw_frame callback
@related SUOverlayRef
*/
typedef void (*SUOverlayDrawFrameFuncT)(SUOverlayRef, struct SUOverlayDrawFrameInfo*, void*);

/**
@brief Signature for SUOverlayCreateInfo::end_frame callback
@related SUOverlayRef
*/
typedef void (*SUOverlayEndFrameFuncT)(SUOverlayRef, void*);

/**
@brief Signature for SUOverlayCreateInfo::start callback
@related SUOverlayRef
*/
typedef void (*SUOverlayStartFuncT)(SUOverlayRef, void*);

/**
@brief Signature for SUOverlayCreateInfo::stop callback
@related SUOverlayRef
*/
typedef void (*SUOverlayStopFuncT)(SUOverlayRef, void*);

/**
@brief Available versions for the SUOverlayCreateInfo::version field.
@related SUOverlayRef
*/
enum SUOverlayCreateInfoVersion : uint32_t {
  SUOVERLAY_CREATE_INFO_VERSION_1 = 1,
};

/**
@brief Current version of the SUOverlayCreateInfo struct.
@related SUOverlayRef
*/
#define SUOVERLAY_CREATE_INFO_VERSION (SUOVERLAY_CREATE_INFO_VERSION_1)

/**
@struct SUOverlayCreateInfo
@brief Arguments for creating an overlay object.
@related SUOverlayRef
*/
struct SUOverlayCreateInfo {
  /// Version of the struct. This must always be set to SUOVERLAY_CREATE_INFO_VERSION
  enum SUOverlayCreateInfoVersion version;

  /// A pointer that will passed directly back into provided callbacks. SketchUp will not access
  /// this pointer.
  void* user_data;

  /// Unique id for the overlay
  const char* id;
  /// Name of the overlay. This will be displayed to user.
  const char* name;
  /// Description of the overlay. This will be displayed to user.
  const char* desc;
  /// Source of the overlay. This will be displayed to user.
  const char* source;

  /// Image format of the overlay color buffer
  enum SUOverlayImageFormat image_format;
  /// Image orientation of the overlay color and depth buffer
  enum SUOverlayImageOrientation image_orientation;

  /// A function that will be called when user enables the overlay.
  SUOverlayStartFuncT start;
  /// A function that will be called when user disables the overlay.
  SUOverlayStopFuncT stop;
  /// A function that should return bounds of the overlay.
  SUOverlayGetExtentsFuncT get_extents;

  /// Function that SketchUp will call at the beginning of a frame.
  SUOverlayBeginFrameFuncT begin_frame;
  /// Function that SketchUp will call to get overlay buffers and blending information.
  SUOverlayDrawFrameFuncT draw_frame;
  /// Function that SketchUp will call to signal the resources passed in draw_frame are no longer
  /// needed.
  SUOverlayEndFrameFuncT end_frame;
};

/**
@brief Enables or disables an overlay.
@related SUOverlayRef
@param[in] overlay The overlay object
@param[in] enabled Whether to enable or disable
@since SketchUp 2024, API 12.0
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if overlay is an invalid object
*/
SU_RESULT SUOverlayEnable(SUOverlayRef overlay, bool enabled);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif  // SKETCHUP_APPLICATION_OVERLAY_H_
