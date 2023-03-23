//
// Created by tor on 3/23/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

static const int MAX_BONE_PER_VERTEX = 4;

struct Vertex {
  glm::vec3 pos;    // m_position
//  glm::vec3 normal; // vertex normal
//  glm::vec2 tex;    // 2D texture coordinates, these are given in obj files.
};

// TODO: add vertex weights as well
// 		bone_ids is the ID of the bones that affect this vertex's position
struct AnimatedVertex {
  AnimatedVertex() {
	for (int &bone_id : bone_ids) {
	  bone_id = -1;
	}
  }

  glm::vec3 pos{};
  int bone_ids[MAX_BONE_PER_VERTEX]{};
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
