// Copyright 2013-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Types related to the SketchUp model.
 */
#ifndef SKETCHUP_MODEL_DEFS_H_
#define SKETCHUP_MODEL_DEFS_H_

#include <SketchUpAPI/defs.h>

#pragma pack(push, 8)

DEFINE_SU_TYPE(SUArcCurveRef)
DEFINE_SU_TYPE(SUAttributeDictionaryRef)
DEFINE_SU_TYPE(SUAxesRef)
DEFINE_SU_TYPE(SUCameraRef)
DEFINE_SU_TYPE(SUClassificationsRef)
DEFINE_SU_TYPE(SUClassificationAttributeRef)
DEFINE_SU_TYPE(SUClassificationInfoRef)
DEFINE_SU_TYPE(SUComponentDefinitionRef)
DEFINE_SU_TYPE(SUComponentInstanceRef)
DEFINE_SU_TYPE(SUCurveRef)
DEFINE_SU_TYPE(SUDimensionRef)
DEFINE_SU_TYPE(SUDimensionLinearRef)
DEFINE_SU_TYPE(SUDimensionRadialRef)
DEFINE_SU_TYPE(SUDimensionStyleRef)
DEFINE_SU_TYPE(SUDrawingElementRef)
DEFINE_SU_TYPE(SUDynamicComponentInfoRef)
DEFINE_SU_TYPE(SUDynamicComponentAttributeRef)
DEFINE_SU_TYPE(SUEdgeRef)
DEFINE_SU_TYPE(SUEdgeUseRef)
DEFINE_SU_TYPE(SUEntitiesRef)
DEFINE_SU_TYPE(SUEntityListRef)
DEFINE_SU_TYPE(SUEntityListIteratorRef)
DEFINE_SU_TYPE(SUEntityRef)
DEFINE_SU_TYPE(SUFaceRef)
DEFINE_SU_TYPE(SUFontRef)
DEFINE_SU_TYPE(SUGeometryInputRef)
DEFINE_SU_TYPE(SUGroupRef)
DEFINE_SU_TYPE(SUGuideLineRef)
DEFINE_SU_TYPE(SUGuidePointRef)
DEFINE_SU_TYPE(SUImageRef)
DEFINE_SU_TYPE(SUImageRepRef)
DEFINE_SU_TYPE(SUInstancePathRef)
DEFINE_SU_TYPE(SULayerRef)
DEFINE_SU_TYPE(SULayerFolderRef)
DEFINE_SU_TYPE(SULineStyleRef)
DEFINE_SU_TYPE(SULineStylesRef)
DEFINE_SU_TYPE(SULocationRef)
DEFINE_SU_TYPE(SULoopInputRef)
DEFINE_SU_TYPE(SULoopRef)
DEFINE_SU_TYPE(SUMaterialRef)
DEFINE_SU_TYPE(SUMeshHelperRef)
DEFINE_SU_TYPE(SUModelRef)
DEFINE_SU_TYPE(SUOpeningRef)
DEFINE_SU_TYPE(SUOptionsManagerRef)
DEFINE_SU_TYPE(SUOptionsProviderRef)
DEFINE_SU_TYPE(SUPolyline3dRef)
DEFINE_SU_TYPE(SURenderingOptionsRef)
DEFINE_SU_TYPE(SUSceneRef)
DEFINE_SU_TYPE(SUSchemaRef)
DEFINE_SU_TYPE(SUSchemaTypeRef)
DEFINE_SU_TYPE(SUSectionPlaneRef)
DEFINE_SU_TYPE(SUSelectionRef)
DEFINE_SU_TYPE(SUShadowInfoRef)
DEFINE_SU_TYPE(SUStyleRef)
DEFINE_SU_TYPE(SUStylesRef)
DEFINE_SU_TYPE(SUTextRef)
DEFINE_SU_TYPE(SUTextureRef)
DEFINE_SU_TYPE(SUTextureWriterRef)
DEFINE_SU_TYPE(SUTypedValueRef)
DEFINE_SU_TYPE(SUUVHelperRef)
DEFINE_SU_TYPE(SUVertexRef)

/**
@enum SURefType
@brief Types of concrete object references.
*/
enum SURefType {
  SURefType_Unknown = 0,                ///< Unknown object type.
  SURefType_AttributeDictionary,        ///< SUAttributeDictionaryRef type
  SURefType_Camera,                     ///< SUCameraRef type
  SURefType_ComponentDefinition,        ///< SUComponentDefinitionRef type
  SURefType_ComponentInstance,          ///< SUComponentInstanceRef type
  SURefType_Curve,                      ///< SUCurveRef type
  SURefType_Edge,                       ///< SUEdgeRef type
  SURefType_EdgeUse,                    ///< SUEdgeUseRef type
  SURefType_Entities,                   ///< SUEntitiesRef type
  SURefType_Face,                       ///< SUFaceRef type
  SURefType_Group,                      ///< SUGroupRef type
  SURefType_Image,                      ///< SUImageRef type
  SURefType_Layer,                      ///< SULayerRef type
  SURefType_Location,                   ///< SULocationRef type
  SURefType_Loop,                       ///< SULoopRef type
  SURefType_MeshHelper,                 ///< SUMeshHelperRef type
  SURefType_Material,                   ///< SUMaterialRef type
  SURefType_Model,                      ///< SUModelRef type
  SURefType_Polyline3D,                 ///< SUPolyline3DRef type
  SURefType_Scene,                      ///< SUSceneRef type
  SURefType_Texture,                    ///< SUTextureRef type
  SURefType_TextureWriter,              ///< SUTextureWriterRef type
  SURefType_TypedValue,                 ///< SUTypedValueRef type
  SURefType_UVHelper,                   ///< SUUVHelperRef type
  SURefType_Vertex,                     ///< SUVertexRef type
  SURefType_RenderingOptions,           ///< SURenderingOptionsRef type
  SURefType_GuidePoint,                 ///< SUGuidePointRef type
  SURefType_GuideLine,                  ///< SUGuideLineRef type
  SURefType_Schema,                     ///< SUSchemaRef type
  SURefType_SchemaType,                 ///< SUSchemaTypeRef type
  SURefType_ShadowInfo,                 ///< SUShadowInfoRef type
  SURefType_Axes,                       ///< SUAxesRef type
  SURefType_ArcCurve,                   ///< SUArcCurveRef type
  SURefType_SectionPlane,               ///< SUSectionPlaneRef type
  SURefType_DynamicComponentInfo,       ///< SUDynamicComponentInfoRef type
  SURefType_DynamicComponentAttribute,  ///< SUDynamicComponentAttributeRef type
  SURefType_Style,                      ///< SUStyleRef type
  SURefType_Styles,                     ///< SUStylesRef type
  SURefType_ImageRep,                   ///< SUImageRepRef type
  SURefType_InstancePath,               ///< SUInstancePathRef type
  SURefType_Font,                       ///< SUFontRef type
  SURefType_Dimension,                  ///< SUDimensionRef type
  SURefType_DimensionLinear,            ///< SUDimensionLinearRef type
  SURefType_DimensionRadial,            ///< SUDimensionRadialRef type
  SURefType_DimensionStyle,             ///< SUDimensionStyleRef type
  SURefType_Text,                       ///< SUTextRef type
  SURefType_EntityList,                 ///< SUEntityListRef type
  SURefType_EntityListIterator,         ///< SUEntityListIteratorRef type
  SURefType_DrawingElement,             ///< SUDrawingElementRef type
  SURefType_Entity,                     ///< SUEntityRef type
  SURefType_LengthFormatter,            ///< SULengthFormatterRef type
  SURefType_LineStyle,                  ///< SULineStyleRef type
  SURefType_LineStyleManager,           ///< SULineStyleManagerRef type
  SURefType_Selection,                  ///< SUSelectionRef type
  SURefType_LayerFolder,                ///< SULayerFolderRef type

};

#pragma pack(pop)

#endif  // SKETCHUP_MODEL_DEFS_H_
