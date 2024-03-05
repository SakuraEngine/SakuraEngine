// Copyright 2016-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for \ref SUArrowType.
 */
#ifndef SKETCHUP_MODEL_ARROW_TYPE_H_
#define SKETCHUP_MODEL_ARROW_TYPE_H_

/**
 @enum SUArrowType
 @brief Indicates the supported arrow types, currently used by SUDimensionRef
        and SUTextRef.
 */
enum SUArrowType { SUArrowNone = 0, SUArrowSlash, SUArrowDot, SUArrowClosed, SUArrowOpen };

#endif  // SKETCHUP_MODEL_ARROW_TYPE_H_
