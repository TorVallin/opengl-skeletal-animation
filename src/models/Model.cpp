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
  // TODO: remember to loop the animation by wrapping the current_animation_time
  std::vector<glm::mat4> parentTransforms;

  for (auto &nodeData : node_list) {
	std::string nodeName = nodeData.node_name;
	glm::mat4 nodeTransform = nodeData.transformation;

	auto bone = get_bone_by_name(nodeName);

	if (bone) {
	  bone->update_local_transformation(delta_time);
	  nodeTransform = bone->local_transformation;
	}

	glm::mat4
		parentTransform = nodeData.parent_index != -1 ? parentTransforms[nodeData.parent_index] : glm::mat4(1.0f);
	glm::mat4 globalTransformation = parentTransform * nodeTransform;
	parentTransforms.push_back(globalTransformation);

	if (bone) {
	  int index = bone_name_to_index[nodeName];
	  glm::mat4 offset = bone_offset_matrix[index];
	  skinning_matrices[index] = globalTransformation * offset;
	}
  }
}
