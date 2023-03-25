//
// Created by tor on 3/23/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_LOADERS_MODELLOADER_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_LOADERS_MODELLOADER_H_

#include <string>

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <iostream>
#include <optional>
#include <stack>
#include <glm/glm.hpp>

#include "Model.h"
#include "Conversions.h"
class AnimatedModelLoader {
 public:

  [[nodiscard]] static std::optional<Model> load_model(const std::string &path) {

	aiPropertyStore *props = aiCreatePropertyStore();
	aiSetImportPropertyInteger(props, "PP_PTV_NORMALIZE", 1);

	// Triangulates, generates normals
	const aiScene *scene = (aiScene *)aiImportFileExWithProperties(path.c_str(),
																   aiProcess_Triangulate
																	   | aiProcess_ForceGenNormals
																	   | aiProcess_GenSmoothNormals,
																   nullptr, props);

	if (!scene) {
	  std::cerr << "Assimp failed to load: " << path << std::endl;
	  return std::nullopt;
	}

	return load_node(scene);
  }
 private:

  struct ToProcessEntry {
	aiNode *to_process = nullptr;
	int parent_index = -1;
  };

  static Model load_node(const aiScene *scene) {
	Model model{};
	std::stack<ToProcessEntry> nodes_to_process;
	nodes_to_process.push({scene->mRootNode, -1});

	while (!nodes_to_process.empty()) {
	  const auto &next_node = nodes_to_process.top();
	  nodes_to_process.pop();

	  for (unsigned int i = 0; i < next_node.to_process->mNumMeshes; i++) {
		model.mesh_list.push_back(load_mesh(scene, scene->mMeshes[next_node.to_process->mMeshes[i]], model));
	  }

	  model.node_list.emplace_back(next_node.to_process->mName.data,
								   Conversions::convertAssimpMat4ToGLM(next_node.to_process->mTransformation),
								   next_node.parent_index);

	  // The current not is of course a parent to its children, meaning its ID will be the childrens' parent ID
	  int this_node_id = model.node_list.size() - 1;
	  for (unsigned int i = 0; i < next_node.to_process->mNumChildren; i++) {
		nodes_to_process.push({next_node.to_process->mChildren[i], this_node_id});
	  }
	}

	load_bones(scene->mAnimations[0], model);

	return model;
  }

  [[nodiscard]] static Mesh load_mesh(const aiScene *,
									  const aiMesh *mesh,
									  Model &model
  ) {
	std::vector<AnimatedVertex> all_vertices;
	std::vector<unsigned int> all_indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
	  AnimatedVertex vert{};
	  vert.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

	  all_vertices.push_back(vert);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
	  for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
		all_indices.push_back(mesh->mFaces[i].mIndices[j]);
	  }
	}

	Mesh result;
	result.vertices = all_vertices;
	result.indices = all_indices;

	create_mesh(result);
	load_bone_vertex_weights(mesh, result.vertices, model);

	return result;
  }

  static void load_bone_vertex_weights(const aiMesh *mesh, std::vector<AnimatedVertex> &vertices, Model &model) {
	// NOTE: bone_index refers to a "relative index", that is, it is able to index into mesh->mBones.
	// Which is NOT the same as bone_id, which is a globally unique id given to all bones.
	for (int bone_index = 0; bone_index < (int)mesh->mNumBones; bone_index++) {
	  int bone_id = -1;

	  auto bone_name = mesh->mBones[bone_index]->mName.C_Str();
	  auto absolute_index = model.bone_name_to_index.find(bone_name);
	  if (absolute_index == model.bone_name_to_index.end()) {
		// bone_index does not already exist in offset_matrix, so we create it
		model.bone_offset_matrix[model.next_bone_id] =
			Conversions::convertAssimpMat4ToGLM(mesh->mBones[bone_index]->mOffsetMatrix);
		model.bone_name_to_index[bone_name] = model.next_bone_id;
		model.next_bone_id += 1;
	  } else {
		// Otherwise, the bone already exists in the map
		bone_id = absolute_index->second;
	  }

	  // Configure the vertex data
	  auto weights = mesh->mBones[bone_index]->mWeights;
	  auto weight_count = mesh->mBones[bone_index]->mWeights;
	  for (int weight_index = 0; weight_index < (int)mesh->mBones[bone_index]->mNumWeights; weight_index++) {
		// vertex_id is the index of the vertex that is influenced by the bone with bone_id
		auto vertex_id = weights[weight_index].mVertexId;
		vertices[vertex_id].set_weight_to_first_unset(bone_id, weights[weight_index].mWeight);
	  }
	}
  }

  // The animation consists of an array of channels.
  // Each channel consists of keyframes, the channels also have a name that corresponds to the name of a node
  // in the scene.
  static void load_bones(const aiAnimation *animation, Model &model) {
	for (unsigned int channel_index = 0; channel_index < animation->mNumChannels; channel_index++) {
	  auto channel = animation->mChannels[channel_index];
	  auto bone = model.bone_name_to_index.find(channel->mNodeName.data);

	  if (bone == model.bone_name_to_index.end()) {
		model.bone_name_to_index[channel->mNodeName.data] = model.next_bone_id;
		model.next_bone_id += 1;
	  }

	  model.bone_list.emplace_back(model.bone_name_to_index[channel->mNodeName.data], channel);
	}
  }

  static void create_mesh(Mesh &mesh) {
	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ebo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(AnimatedVertex), &mesh.vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 mesh.indices.size() * sizeof(unsigned int),
				 &mesh.indices[0],
				 GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), nullptr);
  }

  [[nodiscard]] static std::string get_base_path(const std::string &path) {
	size_t pos = path.find_last_of("\\/");
	return (std::string::npos == pos) ? "" : path.substr(0, pos + 1);
  }
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_LOADERS_MODELLOADER_H_
