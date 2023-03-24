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

#include "Model.h"
class ModelLoader {
 public:

  [[nodiscard]] static std::optional<Model> load_model(const std::string &path) {

	aiPropertyStore *props = aiCreatePropertyStore();
	aiSetImportPropertyInteger(props, "PP_PTV_NORMALIZE", 1);

	// Triangulates, generates normals
	const aiScene *scene = (aiScene *)aiImportFileExWithProperties(path.c_str(),
																   aiProcess_Triangulate
																	   | aiProcess_PreTransformVertices
																	   | aiProcess_ForceGenNormals
																	   | aiProcess_GenSmoothNormals,
																   nullptr, props);
	if (!scene) {
	  std::cerr << "Assimp failed to load: " << path << std::endl;
	  return std::nullopt;
	}

	return {load_node(scene)};
  }
 private:

  static Model load_node(const aiScene *scene) {
	Model model{};
	std::stack<aiNode *> nodes_to_process;
	nodes_to_process.push(scene->mRootNode);

	while (!nodes_to_process.empty()) {
	  auto next_node = nodes_to_process.top();
	  nodes_to_process.pop();

	  for (unsigned int i = 0; i < next_node->mNumMeshes; i++) {
		model.mesh_list.push_back(load_mesh(scene, scene->mMeshes[next_node->mMeshes[i]]));
	  }

	  for (unsigned int i = 0; i < next_node->mNumChildren; i++) {
		nodes_to_process.push(next_node->mChildren[i]);
	  }
	}

	return model;
  }

  [[nodiscard]] static Mesh load_mesh(const aiScene *, const aiMesh *mesh) {
	std::vector<Vertex> all_vertices;
	std::vector<unsigned int> all_indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
	  Vertex vert{};
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

	return result;
  }

  static void create_mesh(Mesh &mesh) {
	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ebo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 mesh.indices.size() * sizeof(unsigned int),
				 &mesh.indices[0],
				 GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  }

  [[nodiscard]] static std::string get_base_path(const std::string &path) {
	size_t pos = path.find_last_of("\\/");
	return (std::string::npos == pos) ? "" : path.substr(0, pos + 1);
  }
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_LOADERS_MODELLOADER_H_
