// Copyright 2015 Trimble Navigation Ltd., All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_GEOMETRY_GEOMETRY_H_
#define LAYOUT_GEOMETRY_GEOMETRY_H_

#include <SketchUpAPI/geometry.h>

#pragma pack(push, 8)

/**
@typedef LOPoint2D
@brief Represents a point in 2-dimensional paper space.
*/
typedef SUPoint2D LOPoint2D;

/**
@typedef LOPoint3D
@brief Represents a point in 3-dimensional model space.
*/
typedef SUPoint3D LOPoint3D;

/**
@typedef LOVector2D
@brief Represents a vector in 2-dimensional paper space.
@since LayOut 2017, API 2.0
*/
typedef SUVector2D LOVector2D;


/**
@struct LOAxisAlignedRect2D
@brief Represents a 2D rectangle that is aligned with the X and Y axis of the
       coordinate system.
*/
typedef SUAxisAlignedRect2D LOAxisAlignedRect2D;

/**
@struct LOOrientedRect2D
@brief Represents a 2D rectangle that might be rotated. Each corner of the
       rectangle is represented by a separate 2D point.
*/
struct LOOrientedRect2D {
  LOPoint2D upper_left;
  LOPoint2D upper_right;
  LOPoint2D lower_right;
  LOPoint2D lower_left;
};

/**
@struct LOTransformMatrix2D
@brief 2x3 matrix for representing 2D transforms on LayOut entities. The matrix
       is stored in column-major format:
<pre>
 m11 m21 tx
 m12 m22 ty
</pre>
*/
typedef SUTransformation2D LOTransformMatrix2D;

/**
@typedef LOPoint2D
@brief Represents a size in 2D space.
*/
typedef LOPoint2D LOSize2D;

/**
@struct LORect2D
@brief Represents a 2D rectangle.
*/
struct LORect2D {
  LOPoint2D origin;
  LOSize2D size;
};

#pragma pack(pop)

#endif  // LAYOUT_GEOMETRY_GEOMETRY_H_
