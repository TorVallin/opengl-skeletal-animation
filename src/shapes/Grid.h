//
// Created by tor on 3/23/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_SHAPES_GRID_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_SHAPES_GRID_H_

#include <glad/glad.h>
#include <vector>
#include <glm/vec3.hpp>

class Grid {
 public:

  Grid() {
	setup_buffers();
  }

  ~Grid() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
  }

  void render() const {
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_LINES, 0, (int) m_vertex_count);
	glBindVertexArray(0);
  }

 private:
  unsigned int m_VAO{}, m_VBO{};
  unsigned int m_vertex_count{};
  // Adapted from OpenGL Shading Language Cookbook by David Wolff
  void setup_buffers() {
	std::vector<glm::vec3> vertices;
	float size = 10.0f;
	const int GRID_DIMENSIONS = 10;

	for (auto column = 0; column <= GRID_DIMENSIONS * 2; column++) {
	  float x = (float)column - size;
	  vertices.emplace_back(x, 0, -size);
	  vertices.emplace_back(x, 0, size);
	}
	for (auto row = 0; row <= GRID_DIMENSIONS * 2; row++) {
	  float z = (float)row - size;
	  vertices.emplace_back(-size, 0, z);
	  vertices.emplace_back(size, 0, z);
	}

	m_vertex_count = vertices.size();

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
  }

};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_SHAPES_GRID_H_
