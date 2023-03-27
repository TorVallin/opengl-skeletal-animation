//
// Created by tor on 3/24/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_MODELS_CONVERSIONS_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_MODELS_CONVERSIONS_H_

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>

class Conversions {
 public:
  [[nodiscard]] static glm::mat4 convertAssimpMat4ToGLM(const aiMatrix4x4 &src) {
	float results[16] = {
		src.a1, src.b1, src.c1, src.d1,
		src.a2, src.b2, src.c2, src.d2,
		src.a3, src.b3, src.c3, src.d3,
		src.a4, src.b4, src.c4, src.d4
	};

	return glm::make_mat4(results);
  }

  [[nodiscard]] static glm::vec3 convertAssimpVecToGLM(const aiVector3D &src) {
	return glm::vec3{
		src.x, src.y, src.z
	};
  }

  [[nodiscard]] static glm::quat convertAssimpQuatToGLM(const aiQuaternion &src) {
	return glm::quat{
		src.w, src.x, src.y, src.z
	};
  }
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_MODELS_CONVERSIONS_H_
