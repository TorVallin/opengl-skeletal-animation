//
// Created by tor on 3/23/23.
//

#include <optional>
#include <iostream>
#include "Model.h"

std::optional<Bone> Model::get_bone_by_name(const std::string &bone_name) {
  for (auto &bone : bone_list) {
	if (bone.bone_name == bone_name) {
	  return bone;
	}
  }
  return std::nullopt;
}

Model::Model() {
  skinning_matrices.reserve(MAX_BONES_PER_MODEL);
  for (int i = 0; i < MAX_BONES_PER_MODEL; i++) {
	skinning_matrices.emplace_back(1.0f);
  }
}

void Model::update_skinning_matrix(double delta_time) {
  auto current_time = update_time(delta_time);
  std::vector<glm::mat4> parentTransforms;

  for (const auto &nodeData : node_list) {
	auto bone = get_bone_by_name(nodeData.node_name);

	auto nodeTransform = nodeData.transformation;
	if (bone) {
	  bone->update_local_transformation(current_time);
	  nodeTransform = bone->local_transformation;
	}

	glm::mat4
		parentTransform = nodeData.parent_index != -1 ? parentTransforms[nodeData.parent_index] : glm::mat4(1.0f);
	glm::mat4 globalTransformation = parentTransform * nodeTransform;
	parentTransforms.push_back(globalTransformation);

	if (bone) {
	  glm::mat4 offset = bone_offset_matrix[bone->bone_id];
	  skinning_matrices[bone->bone_id] = globalTransformation * offset;
	}
  }
}

double Model::update_time(double delta_time) {
  if (ticks_per_second > 0.0f) {
	current_animation_time += delta_time * ticks_per_second;
  } else {
	current_animation_time += delta_time;
  }

  current_animation_time = std::fmod(current_animation_time, animation_duration);
  return current_animation_time;
}
