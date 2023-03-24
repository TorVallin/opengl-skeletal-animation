//
// Created by tor on 3/23/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <unordered_map>

static const int MAX_BONE_PER_VERTEX = 4;

struct Vertex {
  glm::vec3 pos;

  // TODO: add these later
  //  glm::vec3 normal; // vertex normal
  //  glm::vec2 tex;    // 2D texture coordinates, these are given in obj files.
};

// TODO: add vertex weights as well
// 		bone_ids is the ID of the bones that affect this vertex's position
struct AnimatedVertex {
  glm::vec3 pos{};
  // This means that bone_ids[i] impacts this vertex with weight bone_weights[i]
  int bone_ids[MAX_BONE_PER_VERTEX]{};
  float bone_weights[MAX_BONE_PER_VERTEX]{};

  AnimatedVertex() {
	for (int &bone_id : bone_ids) {
	  bone_id = -1;
	}
  }

  // The function sets bone_id as a bone that influences this vertex with a weight of bone_weight
  void set_weight_to_first_unset(int bone_id, float bone_weight) {
	// Iterate through the list of bones to find the first available spot.
	// bone_ids[i] < 0 means that this index is uninitialized and free.
	for (int i = 0; i < MAX_BONE_PER_VERTEX; i++) {
	  if (bone_ids[i] < 0) {
		bone_ids[i] = bone_id;
		bone_weights[i] = bone_weight;
		break;
	  }
	}
  }
};

struct Mesh {
  std::vector<AnimatedVertex> vertices;
  std::vector<unsigned int> indices;
  unsigned int vao, vbo, ebo;
};

class Model {
 public:
  std::vector<Mesh> mesh_list{};

  // Maps bone ids to their offset matrix (i.e. the matrix called mOffset in assimp).
  // mOffset transforms a bone from t
  std::unordered_map<int, glm::mat4> bone_offset_matrix{};
  std::unordered_map<std::string, int> bone_name_to_index{};
  int next_bone_id = 0;
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_
