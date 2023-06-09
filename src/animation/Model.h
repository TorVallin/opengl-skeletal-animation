//
// Created by tor on 3/23/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_

#include <optional>
#include <utility>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/scene.h>
#include "Conversions.h"
#include "Bone.h"

static const int MAX_BONE_PER_VERTEX = 4;
static const int MAX_BONES_PER_MODEL = 128;

// This means that bone_ids[i] impacts this vertex with weight bone_weights[i]
// NOTE: bone_weights is not actually unused, it gets sent to the vertex shader buffers
struct AnimatedVertex {
  glm::vec3 pos{};
  glm::vec2 tex_coords{};
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

struct Node {
  std::string node_name{};
  glm::mat4 transformation;

  // A bone_index < 0 indicates that this node is not a bone
  // A bone_index >= 0 implies that it is a bone, and the index can be used as an index into the bone_list.
  int bone_index = -1;
  int parent_index = -1;
  Node(std::string node_name, const glm::mat4 &transformation, int parent_index)
	  : node_name(std::move(node_name)), transformation(transformation), parent_index(parent_index) {}
};

class Model {
 public:
  Model();

  std::optional<unsigned int> texture_id = std::nullopt;

  std::vector<Mesh> mesh_list{};
  std::vector<Node> node_list{};
  std::vector<Bone> bone_list{};

  // Maps bone ids to their offset matrix (i.e. the matrix called mOffset in assimp).
  // mOffset is the bone's inverse bind pose matrix (it transforms the bone from bind pose back to bone space)
  std::vector<glm::mat4> bone_offset_matrix{};
  std::unordered_map<std::string, int> bone_name_to_index{}; // just used during initial parsing (for convenience)
  int next_bone_id = 0;
  double current_animation_time = 0.0;
  double ticks_per_second = -1.0f;
  double animation_duration = 0.0f;

  // The matrices that transform vertex positions from their local space to their transformed and
  // animated position. This gets copied to the GPU (vertex shader) to transform the vertices
  std::vector<glm::mat4> skinning_matrices{};

  void update_skinning_matrix(double delta_time);

  void precompute_node_bone_indices() {
	for (auto &nodeData : node_list) {
	  auto bone = get_bone_by_name(nodeData.node_name);
	  if (bone) {
		nodeData.bone_index = bone->second;
	  }
	}
  }

 private:

  // Loops through all bones in the bone list for a bone with the given bone_name.
  //		If such a bone exists, the bone along with its index into the bone list is returned.
  //		Otherwise, if such a bone does not exist, nullopt is returned.
  [[nodiscard]]  std::optional<std::pair<Bone, int>> get_bone_by_name(const std::string &bone_name) const;
  double update_time(double delta_time);
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_
