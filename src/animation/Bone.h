//
// Created by tor on 3/26/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_MODELS_ANIMATION_BONE_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_MODELS_ANIMATION_BONE_H_
#include <string>
#include <vector>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Conversions.h"

struct Vec3KeyFrame {
  glm::vec3 vec;
  double timestamp;
  Vec3KeyFrame(const glm::vec3 &vec, double timestamp) : vec(vec), timestamp(timestamp) {}
};

struct QuatKeyFrame {
  glm::quat quat;
  double timestamp;
  QuatKeyFrame(const glm::quat &quat, double timestamp) : quat(quat), timestamp(timestamp) {}
};

class Bone {
 public:
  Bone(int bone_id, aiNodeAnim *channel);

  // animation_timestamp is the current time of the animation,
  // it is NOT a delta time.
  void update_local_transformation(double animation_timestamp);

  [[nodiscard]] const glm::mat4 &get_local_transform() const {
	return local_transformation;
  }

  [[nodiscard]] int get_bone_id() const {
	return bone_id;
  }

  [[nodiscard]] const std::string &get_bone_name() const {
	return bone_name;
  }

 private:
  int bone_id = -1;
  std::string bone_name{};
  std::vector<Vec3KeyFrame> position_keyframes{};
  std::vector<Vec3KeyFrame> scale_keyframes{};
  std::vector<QuatKeyFrame> rotation_keyframes{};
  glm::mat4 local_transformation = glm::mat4{1.0f};

  [[nodiscard]] static glm::vec3 interpolate_vec3(const std::vector<Vec3KeyFrame> &keyframes, double timestamp);
  [[nodiscard]] static glm::quat interpolate_quat(const std::vector<QuatKeyFrame> &keyframes, double timestamp);
  [[nodiscard]] static double compute_mix(double timestamp1, double timestamp2, double current_time);
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_MODELS_ANIMATION_BONE_H_
