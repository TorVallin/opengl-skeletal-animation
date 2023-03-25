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
	// Loading the model is split into 2 steps,
	// 		1. Loads all meshes and vertices into the model object
	//		2. Loads all animation data, bone information into the model object
	Assimp::Importer importer;
	aiPropertyStore *props = aiCreatePropertyStore();
	aiSetImportPropertyInteger(props, "PP_PTV_NORMALIZE", 1);
	const aiScene *scene = (aiScene *)aiImportFileExWithProperties(path.c_str(),
																   aiProcess_Triangulate
																	   | aiProcess_GenSmoothNormals
																	   | aiProcess_ForceGenNormals,
																   nullptr, props);

	if (!scene || !scene->mRootNode) {
	  std::cerr << "AnimatedModel::Error - Failed to load model " <<
				importer.GetErrorString() << std::endl;
	  return std::nullopt;
	}

	Model model{};
	load_node(model, scene, scene->mRootNode);
	load_bones(model, scene->mAnimations[0]);

	return model;
  }
 private:

  struct NodeToProcess {
	aiNode *to_process = nullptr;
	int parent_index = -1;
  };

  static void load_node(Model &model, const aiScene *scene, const aiNode *node, const int parent_index = -1) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
	  // the node object only contains indices to index the actual objects in the scene.
	  // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
	  aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
	  model.mesh_list.push_back(load_mesh(scene, mesh, model));
	}

	model.node_list.emplace_back(node->mName.data,
								 Conversions::convertAssimpMat4ToGLM(node->mTransformation),
								 parent_index);

	// The current not is of course a parent to its children, meaning its ID will be the childrens' parent ID
	auto this_node_id = (int)(model.node_list.size() - 1);
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
	  load_node(model, scene, node->mChildren[i], this_node_id);
	}
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
  static void load_bones(Model &model, const aiAnimation *animation) {
	for (unsigned int channel_index = 0; channel_index < animation->mNumChannels; channel_index++) {
	  auto channel = animation->mChannels[channel_index];
	  auto bone = model.bone_name_to_index.find(channel->mNodeName.data);

	  if (bone == model.bone_name_to_index.end()) {
		model.bone_name_to_index[channel->mNodeName.data] = model.next_bone_id;
		model.next_bone_id += 1;
		assert("This should not happen, something is wrong with the bone parsing");
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

	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 4, GL_INT, sizeof(AnimatedVertex),
						   (void *)offsetof(AnimatedVertex, bone_ids));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex),
						  (void *)offsetof(AnimatedVertex, bone_weights));
  }

  [[nodiscard]] static std::string get_base_path(const std::string &path) {
	size_t pos = path.find_last_of("\\/");
	return (std::string::npos == pos) ? "" : path.substr(0, pos + 1);
  }
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_LOADERS_MODELLOADER_H_
