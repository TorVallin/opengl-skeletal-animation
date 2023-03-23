//
// Created by tor on 3/23/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

struct Vertex {
  glm::vec3 pos;    // m_position
//  glm::vec3 normal; // vertex normal
//  glm::vec2 tex;    // 2D texture coordinates, these are given in obj files.
};

struct Mesh {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  unsigned int vao, vbo, ebo;
};

class Model {
 public:
  std::vector<Mesh> mesh_list;
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_
