//
// Created by tor on 3/23/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_RENDERER_RENDERER_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_RENDERER_RENDERER_H_

#include <iostream>
#include "models/Model.h"
#include "glad/glad.h"
#include "shader/Shader.h"
class Renderer {
 public:

  static void render_model(const Model &model) {
	if (model.texture_id) {
	  glActiveTexture(GL_TEXTURE0);
	  glBindTexture(GL_TEXTURE_2D, *model.texture_id);
	}

	for (const auto &mesh : model.mesh_list) {
	  glBindVertexArray(mesh.vao);
	  glDrawElements(GL_TRIANGLES, (int)mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	glBindVertexArray(0);
  }
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_RENDERER_RENDERER_H_
