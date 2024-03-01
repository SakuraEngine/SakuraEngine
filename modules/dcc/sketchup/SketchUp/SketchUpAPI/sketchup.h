// Copyright 2015-2020 Trimble Inc., All rights reserved.
// This file is intended for public distribution.

/**
 * @file
 * @brief Utility header that loads all other headers in the SDK package.
 */
#ifndef SKETCHUP_SKETCHUP_H_
#define SKETCHUP_SKETCHUP_H_

/**
 * @dir SketchUpAPI
 * @brief Interfaces for the SketchUp SDK.
 */

/**
 * @dir SketchUpAPI/application
 * @brief Interfaces for usage within the SketchUp application.
 */

/**
 * @dir SketchUpAPI/geometry
 * @brief Interfaces for geometric operations.
 */

/**
 * @dir SketchUpAPI/import_export
 * @brief Interfaces for importers and exporters.
 */

/**
 * @dir SketchUpAPI/model
 * @brief Interfaces for the SketchUp model.
 */

/**
 * @dir SketchUpAPI/utils
 * @brief General utility interfaces.
 */

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/color.h>
#include <SketchUpAPI/extension_license.h>
#include <SketchUpAPI/geometry/bounding_box.h>
#include <SketchUpAPI/geometry/plane3d.h>
#include <SketchUpAPI/geometry/point2d.h>
#include <SketchUpAPI/geometry/point3d.h>
#include <SketchUpAPI/geometry/ray3d.h>
#include <SketchUpAPI/geometry/transformation.h>
#include <SketchUpAPI/geometry/transformation2d.h>
#include <SketchUpAPI/geometry/vector2d.h>
#include <SketchUpAPI/geometry/vector3d.h>
#include <SketchUpAPI/length_formatter.h>
#include <SketchUpAPI/sketchup_info.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/utils/math_helpers.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/application/application.h>
#include <SketchUpAPI/model/arccurve.h>
#include <SketchUpAPI/model/attribute_dictionary.h>
#include <SketchUpAPI/model/axes.h>
#include <SketchUpAPI/model/camera.h>
#include <SketchUpAPI/model/classifications.h>
#include <SketchUpAPI/model/classification_attribute.h>
#include <SketchUpAPI/model/classification_info.h>
#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/component_instance.h>
#include <SketchUpAPI/model/curve.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/dimension.h>
#include <SketchUpAPI/model/dimension_linear.h>
#include <SketchUpAPI/model/dimension_radial.h>
#include <SketchUpAPI/model/dimension_style.h>
#include <SketchUpAPI/model/drawing_element.h>
#include <SketchUpAPI/model/dynamic_component_info.h>
#include <SketchUpAPI/model/dynamic_component_attribute.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/edge_use.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/entity.h>
#include <SketchUpAPI/model/entity_list.h>
#include <SketchUpAPI/model/entity_list_iterator.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/font.h>
#include <SketchUpAPI/model/geometry_input.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/guide_line.h>
#include <SketchUpAPI/model/guide_point.h>
#include <SketchUpAPI/model/image.h>
#include <SketchUpAPI/model/image_rep.h>
#include <SketchUpAPI/model/instancepath.h>
#include <SketchUpAPI/model/layer.h>
#include <SketchUpAPI/model/layer_folder.h>
#include <SketchUpAPI/model/line_style.h>
#include <SketchUpAPI/model/line_styles.h>
#include <SketchUpAPI/model/location.h>
#include <SketchUpAPI/model/loop.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/mesh_helper.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/model_version.h>
#include <SketchUpAPI/model/opening.h>
#include <SketchUpAPI/model/options_manager.h>
#include <SketchUpAPI/model/options_provider.h>
#include <SketchUpAPI/model/polyline3d.h>
#include <SketchUpAPI/model/rendering_options.h>
#include <SketchUpAPI/model/scene.h>
#include <SketchUpAPI/model/schema.h>
#include <SketchUpAPI/model/schema_type.h>
#include <SketchUpAPI/model/section_plane.h>
#include <SketchUpAPI/model/selection.h>
#include <SketchUpAPI/model/skp.h>
#include <SketchUpAPI/model/shadow_info.h>
#include <SketchUpAPI/model/style.h>
#include <SketchUpAPI/model/styles.h>
#include <SketchUpAPI/model/text.h>
#include <SketchUpAPI/model/texture.h>
#include <SketchUpAPI/model/texture_writer.h>
#include <SketchUpAPI/model/typed_value.h>
#include <SketchUpAPI/model/uv_helper.h>
#include <SketchUpAPI/model/vertex.h>
#endif  // SKETCHUP_SKETCHUP_H_
