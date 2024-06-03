#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrContainers/hashmap.hpp"
#include "SkrCore/log.h"
#include "SkrCore/module/module.hpp"
#include "SkrOS/thread.h"
#include "SkrTask/parallel_for.hpp"

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/mesh_helper.h>
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

class Cache {
private:
  std::unordered_map<int64_t, std::wstring> node_ids;

public:
  void SetNodeId(int32_t entity_id, int32_t mat_id,
                 const std::wstring &node_id) {
    // key: entity_identity_id + mat_id
    // 因为此缓存是为了判断是否可以duplicate，需要保证组件实例的实际材质一致
    int64_t entity_and_mat = (int64_t(entity_id) << 32) + mat_id;
    node_ids[entity_and_mat] = node_id;
  }
  std::wstring GetNodeId(int32_t entity_id, int32_t mat_id) {
    int64_t entity_and_mat = (int64_t(entity_id) << 32) + mat_id;
    return node_ids[entity_and_mat];
  }

private:
  skr::ParallelFlatHashMap<void *, SUMeshHelperRef> face_mesh_map;

public:
  //! Note: Don't release return object
  SUMeshHelperRef GetFaceMesh(const SUFaceRef &face) {
    SUMeshHelperRef mesh_helper_ref = SU_INVALID;
    auto &&found = face_mesh_map.find(face.ptr);
    if (found != face_mesh_map.end())
      mesh_helper_ref = found->second;
    else {
      VERIFY_IF_ERROR(SUMeshHelperCreate(&mesh_helper_ref, face));
      face_mesh_map.emplace(face.ptr, mesh_helper_ref);
    }
    return mesh_helper_ref;
  }

  Cache() = default;
  ~Cache() = default;
  void reset() {
    node_ids.clear();

    // release the mesh_helper
    for (auto &pair : face_mesh_map) {
      SUMeshHelperRef mesh_helper = pair.second;
      SUMeshHelperRelease(&mesh_helper);
    };
    face_mesh_map.clear();
  }
};

class SUDumpModule : public skr::IDynamicModule {
  virtual void on_load(int argc, char8_t **argv) override;
  virtual int main_module_exec(int argc, char8_t **argv) override;
  virtual void on_unload() override;

  virtual void DumpTriangles(SUEntitiesRef entities);

  virtual void RecursiveDumpEntities(SUEntitiesRef entities);
  virtual void RecursiveDumpEntities(SUModelRef model);
  virtual void DumpModel(SUModelRef model);

  skr::task::scheduler_t scheduler;
  Cache model_cache;
};

IMPLEMENT_DYNAMIC_MODULE(SUDumpModule, SketchUpDump);

void SUDumpModule::on_load(int argc, char8_t **argv) {
  skr_log_initialize_async_worker();
  skr_log_set_level(SKR_LOG_LEVEL_INFO);
  SKR_LOG_INFO(u8"SUDumpModule module loaded!");

  scheduler.initialize(skr::task::scheudler_config_t());
  scheduler.bind();
}

void SUDumpModule::DumpTriangles(SUEntitiesRef entities) {
  // Get all the faces from the entities object
  size_t faceCount = 0;
  SUEntitiesGetNumFaces(entities, &faceCount);
  if (faceCount > 0) {
    std::vector<SUFaceRef> faces(faceCount);
    SUEntitiesGetFaces(entities, faceCount, &faces[0], &faceCount);
    // Get all the edges in this face
    skr::parallel_for(
        faces.begin(), faces.end(), 1, [this, &faces](auto pstart, auto pend) {
          SUFaceRef face = *pstart;
          SUMaterialRef mat = SU_INVALID;
          bool front = true;

          if (SUFaceGetFrontMaterial(face, &mat) == SU_ERROR_NONE)
            front = true;
          else if (SUFaceGetBackMaterial(face, &mat) == SU_ERROR_NONE)
            front = false;

          SUMeshHelperRef meshHelper = model_cache.GetFaceMesh(face);
          size_t numVertices = 0, numTriangles = 0, cnt = 0;
          SUMeshHelperGetNumVertices(meshHelper, &numVertices);
          SUMeshHelperGetNumTriangles(meshHelper, &numTriangles);

          if (numVertices > 0) { // sometimes a face has no vertices.
            // initialize meshData normalData uvData indexData : Ptr and their
            // size
            size_t indexSize = 0;

            // indexSize is the num of triangles mlutiply 3
            indexSize = numTriangles * 3;
            std::vector<size_t> indices(indexSize);
            SUMeshHelperGetVertexIndices(meshHelper, indexSize, indices.data(),
                                         &cnt);
            std::vector<SUPoint3D> vertices(numVertices);
            SUMeshHelperGetVertices(meshHelper, numVertices, vertices.data(),
                                    &cnt);
            std::vector<SUVector3D> normals(numVertices);
            SUMeshHelperGetNormals(meshHelper, numVertices, normals.data(),
                                   &cnt);

            // uv from SUMeshHelper
            std::vector<SUPoint3D> stq_coords(numVertices);
            if (front)
              SUMeshHelperGetFrontSTQCoords(meshHelper, numVertices,
                                            stq_coords.data(), &numVertices);
            else
              SUMeshHelperGetBackSTQCoords(meshHelper, numVertices,
                                           stq_coords.data(), &numVertices);

            SKR_LOG_INFO(u8"Face%s no.%d has %d vertices",
                         front ? "(front)" : "(back))", pstart - faces.begin(),
                         numVertices);
          }
        });
  }
}

void SUDumpModule::RecursiveDumpEntities(SUEntitiesRef entities) {
  DumpTriangles(entities);

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
    skr::parallel_for(
        materials.begin(), materials.end(), 1,
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
          SUStringGetUTF8(mat_name, name_length + 1, name_utf8, &name_length);
          SKR_LOG_INFO(u8"Material name: %s", (const char8_t *)name_utf8);
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