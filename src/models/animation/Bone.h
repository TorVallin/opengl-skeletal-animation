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
#include "models/Conversions.h"

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
  int bone_id = -1;
  std::string bone_name{};
  std::vector<Vec3KeyFrame> position_keyframes{};
  std::vector<Vec3KeyFrame> scale_keyframes{};
  std::vector<QuatKeyFrame> rotation_keyframes{};
  glm::mat4 local_transformation = glm::mat4{1.0f};

  Bone(int bone_id, aiNodeAnim *channel) {
	this->bone_id = bone_id;
	this->bone_name = channel->mNodeName.data;

	// Convert all channel keyframes to our format
	for (unsigned int pos_index = 0; pos_index < channel->mNumPositionKeys; pos_index++) {
	  auto keyframe = channel->mPositionKeys[pos_index];
	  position_keyframes.emplace_back(Conversions::convertAssimpVecToGLM(keyframe.mValue), keyframe.mTime);
	}

	for (unsigned int rot_index = 0; rot_index < channel->mNumRotationKeys; rot_index++) {
	  auto keyframe = channel->mRotationKeys[rot_index];
	  rotation_keyframes.emplace_back(Conversions::convertAssimpQuatToGLM(keyframe.mValue), keyframe.mTime);
	}

	for (unsigned int scale_index = 0; scale_index < channel->mNumScalingKeys; scale_index++) {
	  auto keyframe = channel->mScalingKeys[scale_index];
	  scale_keyframes.emplace_back(Conversions::convertAssimpVecToGLM(keyframe.mValue), keyframe.mTime);
	}
  }

  // animation_timestamp is the current time of the animation,
  // it is NOT a delta time.
  void update_local_transformation(double animation_timestamp) {
	auto current_pos = interpolate_vec3(position_keyframes, animation_timestamp);
	auto current_rot = interpolate_quat(rotation_keyframes, animation_timestamp);
	auto current_scale = interpolate_vec3(scale_keyframes, animation_timestamp);

	local_transformation = glm::mat4(1.0f);
	local_transformation = glm::translate(local_transformation, current_pos);
	local_transformation *= glm::toMat4(current_rot);
	local_transformation = glm::scale(local_transformation, current_scale);
  }

 private:
  [[nodiscard]] static glm::vec3 interpolate_vec3(const std::vector<Vec3KeyFrame> &keyframes, double timestamp) {
	// If the size is 1, there is nothing to interpolate between
	if (keyframes.size() == 1) {
	  return keyframes[0].vec;
	}

	unsigned int i = 0;
	for (i = 0; i < keyframes.size() - 1; i++) {
	  if (keyframes[i + 1].timestamp > timestamp) {
		break;
	  }
	}

	auto mix = compute_mix(keyframes[i].timestamp, keyframes[i + 1].timestamp, timestamp);
	return glm::mix(keyframes[i].vec, keyframes[i + 1].vec, mix);
  }

  [[nodiscard]] static glm::quat interpolate_quat(const std::vector<QuatKeyFrame> &keyframes, double timestamp) {
	// If the size is 1, there is nothing to interpolate between
	if (keyframes.size() == 1) {
	  return keyframes[0].quat;
	}

	unsigned int i = 0;
	for (i = 0; i < keyframes.size() - 1; i++) {
	  if (keyframes[i + 1].timestamp > timestamp) {
		break;
	  }
	}

	auto mix = (float)compute_mix(keyframes[i].timestamp, keyframes[i + 1].timestamp, timestamp);
	return glm::normalize(glm::slerp(keyframes[i].quat, keyframes[i + 1].quat, mix));
  }

  [[nodiscard]] static double compute_mix(double timestamp1, double timestamp2, double current_time) {
	return (current_time - timestamp1) / (timestamp2 - timestamp1);
  }
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_MODELS_ANIMATION_BONE_H_
