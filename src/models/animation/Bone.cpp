//
// Created by tor on 3/26/23.
//

#include "Bone.h"
Bone::Bone(int bone_id, aiNodeAnim *channel) {
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
void Bone::update_local_transformation(double animation_timestamp) {
  auto current_pos = interpolate_vec3(position_keyframes, animation_timestamp);
  auto current_rot = interpolate_quat(rotation_keyframes, animation_timestamp);
  auto current_scale = interpolate_vec3(scale_keyframes, animation_timestamp);

  local_transformation = glm::mat4(1.0f);
  local_transformation = glm::translate(local_transformation, current_pos);
  local_transformation *= glm::toMat4(current_rot);
  local_transformation = glm::scale(local_transformation, current_scale);
}
glm::vec3 Bone::interpolate_vec3(const std::vector<Vec3KeyFrame> &keyframes, double timestamp) {
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
glm::quat Bone::interpolate_quat(const std::vector<QuatKeyFrame> &keyframes, double timestamp) {
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
double Bone::compute_mix(double timestamp1, double timestamp2, double current_time) {
  return (current_time - timestamp1) / (timestamp2 - timestamp1);
}
