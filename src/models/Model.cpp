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

void Model::update_skinning_matrix(double delta_time) {
  current_animation_time += delta_time;
  std::vector<glm::mat4> parent_transforms{};

  for (const auto &node : node_list) {
	auto bone = get_bone_by_name(node.node_name);
	auto local_transform = node.transformation;

	// Some nodes in node_list are not bones, but "dummy nodes" that are used to create the node hierarchy
	if (bone) {
	  // TODO: update bones properly
	  bone->update_local_transformation(current_animation_time);
	  local_transform = bone->local_transformation;
	}

	glm::mat4 parent_transform = node.parent_index != -1 ? parent_transforms[node.parent_index] : glm::mat4(1.0f);
	glm::mat4 global_transform = parent_transform * local_transform;
	parent_transforms.emplace_back(global_transform);

	// This is the step that computes the actual skinning matrix.
	if (bone) {
	  int index = bone_name_to_index[bone->bone_name];
	  glm::mat4 bone_inverse_bind_pose = bone_offset_matrix[index];
	  skinning_matrices[index] = global_transform * bone_inverse_bind_pose;
	}
  }
}
