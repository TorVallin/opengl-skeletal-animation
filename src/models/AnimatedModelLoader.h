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

static const aiScene *setup_scene(const std::string &path) {
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
	return nullptr;
  }
  return scene;
}

class AnimatedModelLoader {
 public:
  [[nodiscard]] static std::optional<Model> load_model(const std::string &model_path,
													   const std::string &animation_path);
 private:
  static void load_node(Model &model, const aiScene *scene, const aiNode *node);
  static void load_node_animations(const aiScene *scene,
								   const aiNode *node,
								   Model &model,
								   int parent_index);

  [[nodiscard]] static Mesh load_mesh(const aiScene *,
									  const aiMesh *mesh,
									  Model &model
  );
  static void load_vertex_bone_weights(const aiMesh *mesh, std::vector<AnimatedVertex> &vertices, Model &model);
  static void load_bones(Model &model, const aiAnimation *animation);

  static void create_mesh(Mesh &mesh);
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_LOADERS_MODELLOADER_H_
