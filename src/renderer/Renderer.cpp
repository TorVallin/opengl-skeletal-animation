//
// Created by tor on 3/23/23.
//

#include "Renderer.h"

void Renderer::render_model(const Model &model) {
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
