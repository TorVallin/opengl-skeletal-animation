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
  [[nodiscard]] static std::optional<Model> load_model(const std::string &model_path,
													   const std::string &animation_path);
 private:
  static void load_node(Model &model, const aiScene *scene, const aiNode *node);

  /**
   * Populates the given model's node_list with the nodes found in the scene.
   * @param scene contains the animation to load.
   * @param node The node to traverse next (use the scene's root node).
   * @param model
   * @param parent_index
   */
  static void load_node_animations(const aiScene *scene,
								   const aiNode *node,
								   Model &model,
								   int parent_index);

  /**
   * Loads vertices properties (position, texture coordinates) from the assimp scene & generates a new mesh
   * object.
   * @param mesh
   * @param model
   * @return
   */
  [[nodiscard]] static Mesh load_mesh(const aiScene *,
									  const aiMesh *mesh,
									  Model &model
  );
  static void load_vertex_bone_weights(const aiMesh *mesh, std::vector<AnimatedVertex> &vertices, Model &model);

  static void load_bones(Model &model, const aiAnimation *animation);

  /**
   * Initializes the OpenGL buffers & sends the mesh data to the GPU.
   */
  static void create_mesh(Mesh &mesh);
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_LOADERS_MODELLOADER_H_
