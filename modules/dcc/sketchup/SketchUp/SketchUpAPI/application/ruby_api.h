// Copyright 2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for exchanging data between SketchUp's Ruby API and C API.
 *
 * @note This is for use only within the SketchUp application.
 */
#ifndef SKETCHUP_APPLICATION_RUBY_API_H_
#define SKETCHUP_APPLICATION_RUBY_API_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>
#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief This macro is defining a type matching Ruby's own `VALUE` type and
       can be used interchangeably.
*/
#ifdef __APPLE__
  #define RUBY_VALUE unsigned long
#elif _WIN32
  #define RUBY_VALUE unsigned long long
#else
  #warning "Unsupported platform"
  #define RUBY_VALUE unsigned long
#endif

/**
@brief Converts a C API entity to a Ruby API entity.

@since SketchUp 2020.2, API 8.2

@param[in]  entity       The C API entity reference.
@param[out] ruby_entity  The retrieved Ruby API entity reference.
@see SUModelRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p entity is an invalid object
- \ref SU_ERROR_INVALID_ARGUMENT if \p entity is not owned by a model
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p ruby_entity is `NULL`
- \ref SU_ERROR_NO_DATA if this isn't called from within SketchUp
*/
SU_RESULT SUEntityToRuby(SUEntityRef entity, RUBY_VALUE* ruby_entity);

/**
@brief Converts a Ruby API entity to a C API entity.

@since SketchUp 2020.2, API 8.2

@param[in]  ruby_entity  The Ruby API entity reference.
@param[out] entity       The retrieved C API entity reference.
@see SUModelRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_ARGUMENT if \p ruby_entity refers to a deleted entity
- \ref SU_ERROR_INVALID_ARGUMENT if \p ruby_entity is not an entity type
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p entity is `NULL`
- \ref SU_ERROR_OVERWRITE_VALID if \p entity is already a valid reference
- \ref SU_ERROR_NO_DATA if this isn't called from within SketchUp
*/
SU_RESULT SUEntityFromRuby(RUBY_VALUE ruby_entity, SUEntityRef* entity);

/**
@brief Converts a C API entity to a Ruby API imagerep.

@note Ruby takes ownership of the object it should not be released by
      SUImageRepRelease. It will be released when Ruby garbage collects the
      Ruby references to it.

@since SketchUp 2020.2, API 8.2

@param[in]  imagerep       The C API imagerep reference.
@param[out] ruby_imagerep  The retrieved Ruby API imagerep reference.
@see SUModelRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p imagerep is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p ruby_imagerep is `NULL`
- \ref SU_ERROR_NO_DATA if this isn't called from within SketchUp
*/
SU_RESULT SUImageRepToRuby(SUImageRepRef imagerep, RUBY_VALUE* ruby_imagerep);

/**
@brief Converts a Ruby API imagerep to a C API imagerep.

@note Ruby retains the ownership of the object it should not be released by
      SUImageRepRelease.

@warning The returned C API reference points to an object that is owned by the
         Ruby runtime and it may be garbage collected. To avoid this, ensure
         that the Ruby API reference has a longer lifetime than the C API
         reference.

@since SketchUp 2020.2, API 8.2

@param[in]  ruby_imagerep  The Ruby API imagerep reference.
@param[out] imagerep       The retrieved C API imagerep reference.
@see SUModelRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_ARGUMENT if \p ruby_imagerep is not an imagerep type
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p imagerep is `NULL`
- \ref SU_ERROR_OVERWRITE_VALID if \p imagerep is already a valid reference
- \ref SU_ERROR_NO_DATA if this isn't called from within SketchUp
*/
SU_RESULT SUImageRepFromRuby(RUBY_VALUE ruby_imagerep, SUImageRepRef* imagerep);

#ifdef __cplusplus
}
#endif
#pragma pack(pop)

#endif  // SKETCHUP_APPLICATION_RUBY_API_H_
