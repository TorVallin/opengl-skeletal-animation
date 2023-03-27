//
// Created by tor on 3/23/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_PROGRAM_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_PROGRAM_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader/Shader.h"
#include "shapes/Grid.h"
#include "animation/AnimatedModelLoader.h"
#include "renderer/Renderer.h"

class Program {
 public:
  [[nodiscard]] GLFWwindow *init_glfw() const;

  void run() const;

 private:
  const int SCR_WIDTH = 800;
  const int SCR_HEIGHT = 800;

  static void configure_opengl() {
	glEnable(GL_DEPTH_TEST);
  }
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_PROGRAM_H_
