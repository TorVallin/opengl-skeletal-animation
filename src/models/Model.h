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

static const int MAX_BONE_PER_VERTEX = 4;
static const int MAX_BONES_PER_MODEL = 128;

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

struct Node {
  std::string node_name;
  glm::mat4 transformation;
  int parent_index = -1;
  Node(std::string node_name, const glm::mat4 &transformation, int parent_index)
	  : node_name(std::move(node_name)), transformation(transformation), parent_index(parent_index) {}
};

struct Vec3KeyFrame {
  glm::vec3 vec;
  double timestamp;
  Vec3KeyFrame(const glm::vec3 &vec, double timestamp) : vec(vec), timestamp(timestamp) {}
};

struct QuatKeyFrame {
  glm::quat quat;
  double timestamp;
  QuatKeyFrame(const glm::quat &quat, double timestamp) : quat(quat), timestamp(timestamp) {}
};

struct Bone {
  int bone_id = -1;
  std::string bone_name{};
  std::vector<QuatKeyFrame> rotation_keyframes{};
  std::vector<Vec3KeyFrame> position_keyframes{};
  glm::mat4 local_transformation = glm::mat4{1.0f};

  Bone(int bone_id, aiNodeAnim *channel) {
	this->bone_id = bone_id;
	this->bone_name = channel->mNodeName.data;

	// Convert all channel keyframes to our format

	for (unsigned int pos_index = 0; pos_index < channel->mNumPositionKeys; pos_index++) {
	  auto keyframe = channel->mPositionKeys[pos_index];
	  position_keyframes.emplace_back(Conversions::convertAssimpVecToGLM(keyframe.mValue), keyframe.mTime);
	}

	for (unsigned int rot_index = 0; rot_index < channel->mNumRotationKeys; rot_index++) {
	  auto keyframe = channel->mRotationKeys[rot_index];
	  rotation_keyframes.emplace_back(Conversions::convertAssimpQuatToGLM(keyframe.mValue), keyframe.mTime);
	}
  }

  // animation_timestamp is the current time of the animation,
  // it is NOT a delta time.
  // TODO: only translations and rotations are handled at the moment
  void update_local_transformation(double animation_timestamp) {
	// TODO: find the correct keyframe later
	auto current_pos = position_keyframes[0].vec;
	auto current_rot = rotation_keyframes[0].quat;

	local_transformation = glm::mat4(1.0f);
	local_transformation = glm::translate(local_transformation, current_pos);
	local_transformation *= glm::toMat4(current_rot);
  }
};

class Model {
 public:
  Model() {
	skinning_matrices.reserve(MAX_BONES_PER_MODEL);
	for (int i = 0; i < MAX_BONES_PER_MODEL; i++) {
	  skinning_matrices.emplace_back(1.0f);
	}
  }

  std::vector<Mesh> mesh_list{};
  std::vector<Node> node_list{};
  std::vector<Bone> bone_list{};

  // Maps bone ids to their offset matrix (i.e. the matrix called mOffset in assimp).
  // mOffset transforms a bone from t
  std::unordered_map<int, glm::mat4> bone_offset_matrix{};
  std::unordered_map<std::string, int> bone_name_to_index{};
  int next_bone_id = 0;
  double current_animation_time = 0.0;

  // The matrices that transform vertex positions from their local space to their transformed and
  // animated position. This gets copied to the GPU (vertex shader) to transform the vertices
  std::vector<glm::mat4> skinning_matrices{};

  void update_skinning_matrix(double delta_time);

 private:
  std::optional<Bone> get_bone_by_name(const std::string &bone_name);
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_MODELS_MODEL_H_
