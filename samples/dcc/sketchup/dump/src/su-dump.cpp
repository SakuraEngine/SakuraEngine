#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/log.h"
#include "SkrModule/module.hpp"
#include "SkrOS/thread.h"
#include "SkrTask/parallel_for.hpp"

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/vertex.h>
#include <SketchUpAPI/slapi.h>
#include <SketchUpAPI/unicodestring.h>

#include <iostream>

#include "SkrProfile/profile.h"

#ifdef NDEBUG
#define VERIFY_IF_ERROR(expression) (expression)
#else
#define VERIFY_IF_ERROR(expression) assert(expression == SU_ERROR_NONE)
#endif

class SUDumpModule : public skr::IDynamicModule {
  virtual void on_load(int argc, char8_t **argv) override;
  virtual int main_module_exec(int argc, char8_t **argv) override;
  virtual void on_unload() override;

  virtual void DumpModel(SUModelRef model);
  virtual void RecursiveDumpEntities(SUEntitiesRef entities);
  virtual void RecursiveDumpEntities(SUModelRef model);

  skr::task::scheduler_t scheduler;
};

IMPLEMENT_DYNAMIC_MODULE(SUDumpModule, SketchUpDump);

void SUDumpModule::on_load(int argc, char8_t **argv) {
    skr_log_initialize_async_worker();
  skr_log_set_level(SKR_LOG_LEVEL_INFO);
  SKR_LOG_INFO(u8"SUDumpModule module loaded!");

  scheduler.initialize(skr::task::scheudler_config_t());
  scheduler.bind();
}

void SUDumpModule::RecursiveDumpEntities(SUEntitiesRef entities) {
  // Get all the faces from the entities object
  size_t faceCount = 0;
  SUEntitiesGetNumFaces(entities, &faceCount);
  if (faceCount > 0) {
    std::vector<SUFaceRef> faces(faceCount);
    SUEntitiesGetFaces(entities, faceCount, &faces[0], &faceCount);
    // Get all the edges in this face
    for (size_t i = 0; i < faceCount; i++) {
      size_t edgeCount = 0;
      SUFaceGetNumEdges(faces[i], &edgeCount);
      if (edgeCount > 0) {
        std::vector<SUEdgeRef> edges(edgeCount);
        SUFaceGetEdges(faces[i], edgeCount, &edges[0], &edgeCount);
        // Get the vertex positions for each edge
        for (size_t j = 0; j < edgeCount; j++) {
          SUVertexRef startVertex = SU_INVALID;
          SUVertexRef endVertex = SU_INVALID;
          SUEdgeGetStartVertex(edges[j], &startVertex);
          SUEdgeGetEndVertex(edges[j], &endVertex);
          SUPoint3D start;
          SUPoint3D end;
          SUVertexGetPosition(startVertex, &start);
          SUVertexGetPosition(endVertex, &end);
          // Now do something with the point data
        }
      }
    }
  }

  // Get all the groups from the entities object
  size_t cntGroup = 0;
  SUEntitiesGetNumGroups(entities, &cntGroup);
  if (cntGroup > 0) {
    skr::Vector<SUGroupRef> groups(cntGroup);
    size_t _n = 0;
    SUEntitiesGetGroups(entities, cntGroup, &groups[0], &_n);
    for (size_t i = 0; i < cntGroup; i++) {
      SUEntitiesRef sub_entities = SU_INVALID;
      SUGroupGetEntities(groups[0], &sub_entities);
      RecursiveDumpEntities(sub_entities);
    }
  }
}

void SUDumpModule::RecursiveDumpEntities(SUModelRef model) {
  // Get the entity container of the model.
  SUEntitiesRef root_entities = SU_INVALID;
  SUModelGetEntities(model, &root_entities);
  RecursiveDumpEntities(root_entities);
}

void SUDumpModule::DumpModel(SUModelRef model) {
  // Dump model name
  {
    SUStringRef name = SU_INVALID;
    SUStringCreate(&name);
    SUModelGetName(model, &name);
    size_t name_length = 0;
    SUStringGetUTF8Length(name, &name_length);
    char *name_utf8 = new char[name_length + 1];
    SUStringGetUTF8(name, name_length + 1, name_utf8, &name_length);
    SKR_LOG_INFO(u8"Model name: %s", name_utf8);
    // Now we have the name in a form we can use
    SUStringRelease(&name);
    delete[] name_utf8;
  }
  // Dump materials
  {
    size_t mats_count = 0;
    VERIFY_IF_ERROR(SUModelGetNumMaterials(model, &mats_count));
    std::vector<SUMaterialRef> materials(mats_count);
    VERIFY_IF_ERROR(
        SUModelGetMaterials(model, mats_count, materials.data(), &mats_count));
    skr::parallel_for(materials.begin(), materials.end(), 1,
                      [&](auto mat_start, auto mat_end) {
                        SUMaterialRef material = *mat_start;
                        SUColor color;
                        SUMaterialGetColor(material, &color);
                        SUStringRef mat_name = SU_INVALID;
                        SUStringCreate(&mat_name);
                        SUMaterialGetName(material, &mat_name);
                        size_t name_length = 0;
                        SUStringGetUTF8Length(mat_name, &name_length);
                        char *name_utf8 = new char[name_length + 1];
                        SUStringGetUTF8(mat_name, name_length + 1, name_utf8,
                                        &name_length);
                        SKR_LOG_INFO(u8"Material name: %s", (const char8_t*)name_utf8);
                        // Now we have the name in a form we can use
                        delete[] name_utf8;
                        SUStringRelease(&mat_name);
                      });
  }
  RecursiveDumpEntities(model);
}

int SUDumpModule::main_module_exec(int argc, char8_t **argv) {
  SKR_LOG_INFO(u8"SUDumpModule module main_module_exec!");

  // Always initialize the API before using it
  SUInitialize();
  // Load the model from a file
  SUModelRef model = SU_INVALID;
  SUModelLoadStatus status;
  SUResult res = SUModelCreateFromFileWithStatus(
      &model, "./../resources/SketchUp/model.skp", &status);
  // It's best to always check the return code from each SU function call.
  // Only showing this check once to keep this example short.
  if (res != SU_ERROR_NONE) {
    std::cout << "Failed creating model from a file" << std::endl;
    return 1;
  }
  if (status == SUModelLoadStatus_Success_MoreRecent) {
    std::cout << "This model was created in a more recent SketchUp version "
                 "than that of the SDK. "
                 "It contains data which will not be read. Saving the model "
                 "over the original file may "
                 "lead to permanent data loss."
              << std::endl;
  }
  DumpModel(model);
  // Must release the model or there will be memory leaks
  SUModelRelease(&model);
  // Always terminate the API when done using it
  SUTerminate();
  return 0;
}

void SUDumpModule::on_unload() {
  scheduler.unbind();

  SKR_LOG_INFO(u8"SUDumpModule module unloaded!");
  skr_log_finalize_async_worker();
}