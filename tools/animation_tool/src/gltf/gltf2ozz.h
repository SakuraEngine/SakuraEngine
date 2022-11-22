#pragma once
#include "SkrAnimTool/ozz/raw_animation_utils.h"
#include "SkrAnimTool/ozz/tools/import2ozz.h"
#include "tiny_gltf.h"

class GltfImporter : public ozz::animation::offline::OzzImporter {
 public:
  GltfImporter();

 private:
  bool Load(const char* _filename) override;

  // Find all unique root joints of skeletons used by given skins and add them
  // to `roots`
  void FindSkinRootJointIndices(const ozz::vector<tinygltf::Skin>& skins,
                                ozz::vector<int>& roots);

  bool Import(ozz::animation::offline::RawSkeleton* _skeleton,
              const NodeType& _types) override;

  // Recursively import a node's children
  bool ImportNode(const tinygltf::Node& _node,
                  ozz::animation::offline::RawSkeleton::Joint* _joint);

  // Returns all animations in the gltf document.
  AnimationNames GetAnimationNames() override;

  bool Import(const char* _animation_name,
              const ozz::animation::Skeleton& skeleton, float _sampling_rate,
              ozz::animation::offline::RawAnimation* _animation) override;

  bool SampleAnimationChannel(
      const tinygltf::Model& _model, const tinygltf::AnimationSampler& _sampler,
      const std::string& _target_path, float _sampling_rate, float* _duration,
      ozz::animation::offline::RawAnimation::JointTrack* _track);

  // Returns all skins belonging to a given gltf scene
  ozz::vector<tinygltf::Skin> GetSkinsForScene(
      const tinygltf::Scene& _scene) const;

  const tinygltf::Node* FindNodeByName(const std::string& _name) const;

  // no support for user-defined tracks
  NodeProperties GetNodeProperties(const char*) override {
    return NodeProperties();
  }
  bool Import(const char*, const char*, const char*, NodeProperty::Type, float,
              ozz::animation::offline::RawFloatTrack*) override {
    return false;
  }
  bool Import(const char*, const char*, const char*, NodeProperty::Type, float,
              ozz::animation::offline::RawFloat2Track*) override {
    return false;
  }
  bool Import(const char*, const char*, const char*, NodeProperty::Type, float,
              ozz::animation::offline::RawFloat3Track*) override {
    return false;
  }
  bool Import(const char*, const char*, const char*, NodeProperty::Type, float,
              ozz::animation::offline::RawFloat4Track*) override {
    return false;
  }

  tinygltf::TinyGLTF m_loader;
  tinygltf::Model m_model;
};