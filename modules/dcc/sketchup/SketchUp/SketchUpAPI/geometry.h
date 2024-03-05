// Copyright 2013 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Define types for geometric operations.
 */
#ifndef SKETCHUP_GEOMETRY_H_
#define SKETCHUP_GEOMETRY_H_

// includes
#include <SketchUpAPI/common.h>

#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@struct SUPoint2D
@brief Represents a point in 2-dimensional space.
*/
struct SUPoint2D {
  double x;  ///< X coordinate
  double y;  ///< Y coordinate
};

/**
@struct SUVector2D
@brief Represents a vector in 2-dimensional space.
*/
struct SUVector2D {
  double x;  ///< X magnitude
  double y;  ///< Y magnitude
};

/**
@struct SUPoint3D
@brief Represents a point in 3-dimensional space.
*/
struct SUPoint3D {
  double x;  ///< X coordinate
  double y;  ///< Y coordinate
  double z;  ///< Z coordinate
};

/**
@struct SUVector3D
@brief Represents a vector in 3-dimensional space.
*/
struct SUVector3D {
  double x;  ///< X magnitude
  double y;  ///< Y magnitude
  double z;  ///< Z magnitude
};

/**
@struct SUPlane3D
@brief Represents a 3D plane by the parameters a, b, c, d.
       For any point on the plane, ax + by + cz + d = 0.
       The coeficients are normalized so that a*a + b*b + c*c = 1.
*/
struct SUPlane3D {
  double a;  ///< The "a" factor in the plane equation.
  double b;  ///< The "b" factor in the plane equation.
  double c;  ///< The "c" factor in the plane equation.
  double d;  ///< The "d" factor in the plane equation.
};

/**
@struct SUBoundingBox3D
@brief Represents a 3D axis-aligned bounding box represented by the extreme
       diagonal corner points with minimum and maximum x,y,z coordinates.
*/
struct SUBoundingBox3D {
  struct SUPoint3D min_point;  ///< A 3D point where x, y and z are minimum
                               ///< values in the bounding box
  struct SUPoint3D max_point;  ///< A 3D point where x, y and z are maximum
                               ///< values in the bounding box
};

/**
@struct SUAxisAlignedRect2D
@brief Represents a 2D rectangle that is aligned with the X and Y axis of the
       coordinate system.
*/
struct SUAxisAlignedRect2D {
  struct SUPoint2D upper_left;   ///< Upper left corner of the bounding box.
  struct SUPoint2D lower_right;  ///< Lower right corner of the bounding box.
};

/**
@struct SURay3D
@brief Represents a 3D ray defined by a point and normal vector.
@since SketchUp 2018, API 6.0
*/
struct SURay3D {
  struct SUPoint3D point;    ///< Origin of the ray.
  struct SUVector3D normal;  ///< Direction of the ray.
};

/**
@struct SUTransformation
@brief Represents a 3D (4x4) geometric transformation matrix.

Matrix values are in column-major order.
The transformation is stored as:

@code{.txt}
     -     -
     | R T |
 M = | 0 w |
     -     -
@endcode

where:
<ul>
<li> M is a 4x4 matrix
<li> R is a 3x3 sub-matrix. It is the rotation part
<li> T is a 3x1 sub-matrix. It is the translation part
<li> w is a scalar.         It is 1/scale.
</ul>
*/
struct SUTransformation {
  double values[16];  ///< Matrix values in column-major order.
};

/**
@struct SUTransformation2D
@brief Represents a 2D (2x3) affine transformation matrix. The matrix
       is stored in column-major format:
<pre>
 m11 m21 tx
 m12 m22 ty
</pre>
*/
struct SUTransformation2D {
  double m11;  ///< the m11 component
  double m12;  ///< the m12 component
  double m21;  ///< the m21 component
  double m22;  ///< the m22 component
  double tx;   ///< the tx component
  double ty;   ///< the ty component
};

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#pragma pack(pop)

#endif  // SKETCHUP_GEOMETRY_H_
