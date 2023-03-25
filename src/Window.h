//
// Created by tor on 3/23/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_WINDOW_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_WINDOW_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader/Shader.h"
#include "shapes/Grid.h"
#include "models/AnimatedModelLoader.h"
#include "renderer/Renderer.h"

static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
								   GLsizei length, const char *message, const void *userParam);
class Window {

 public:
  [[nodiscard]] GLFWwindow *init_glfw() const {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	// glfw window creation
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Skeletal Animation", nullptr, nullptr);
	if (window == nullptr) {
	  std::cerr << "Failed to create GLFW window" << std::endl;
	  glfwTerminate();
	  return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
	  std::cerr << "Failed to initialize GLAD" << std::endl;
	  return nullptr;
	}

	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
	  // initialize debug output
	  glEnable(GL_DEBUG_OUTPUT);
	  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	  glDebugMessageCallback(glDebugOutput, nullptr);
	  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	return window;
  }

  void run() {
	GLFWwindow *glfw_window = this->init_glfw();

	if (glfw_window == nullptr) {
	  std::cerr << "Could not create window, exiting" << std::endl;
	}

	configure_opengl();

	// Creates a basic camera
	glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
	glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), VEC_UP);
	glm::mat4
		projection_matrix = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	Shader shader = Shader("../shaders/basic.vert", "../shaders/basic.frag");
	shader.use();
	shader.setMat4("projection", projection_matrix);
	shader.setMat4("model", model_matrix);
	shader.setMat4("view", view_matrix);

	Shader skel_shader = Shader("../shaders/skeletal_animation.vert", "../shaders/basic.frag");
	skel_shader.use();
	skel_shader.setMat4("projection", projection_matrix);
	skel_shader.setMat4("model", model_matrix);
	skel_shader.setMat4("view", view_matrix);

	Grid grid{};

	std::optional<Model> character_model_opt = AnimatedModelLoader::load_model("../assets/Pushing.fbx");
	if (!character_model_opt) {
	  std::cerr << "Could not load character model, exiting";
	  return;
	}
	auto character_model = *character_model_opt;

	double delta_time = 0.0;
	double last_frame = glfwGetTime();
	while (!glfwWindowShouldClose(glfw_window)) {
	  double current_frame = glfwGetTime();
	  delta_time = current_frame - last_frame;

	  character_model.update_skinning_matrix(delta_time);

	  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	  // Render current frame
	  shader.use();
	  grid.render();

	  skel_shader.use();
	  skel_shader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)));
	  // transfer the skinning matrices to the GPU
	  unsigned int i = 0;
	  for (const auto &skinning_matrices : character_model.skinning_matrices) {
		skel_shader.setMat4("skinning_matrices[" + std::to_string(i) + "]", skinning_matrices);
		i++;
	  }

	  Renderer::render_model(character_model);

	  glfwSwapBuffers(glfw_window);
	  glfwPollEvents();
	}
  }

 private:
  const int SCR_WIDTH = 800;
  const int SCR_HEIGHT = 800;
  const glm::vec3 VEC_UP{0.0f, 1.0f, 0.0f};

  static void configure_opengl() {
	glEnable(GL_DEPTH_TEST);
  }
};

void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}

// Debug output courtesy of LearnOpenGL
static void APIENTRY glDebugOutput(GLenum source,
								   GLenum type,
								   unsigned int id,
								   GLenum severity,
								   GLsizei, // length
								   const char *message,
								   const void *) // userparam
{
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " << message << std::endl;

  switch (source) {
	case GL_DEBUG_SOURCE_API: std::cout << "Source: API";
	  break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "Source: Window System";
	  break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler";
	  break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Source: Third Party";
	  break;
	case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Source: Application";
	  break;
	case GL_DEBUG_SOURCE_OTHER: std::cout << "Source: Other";
	  break;
  }
  std::cout << std::endl;

  switch (type) {
	case GL_DEBUG_TYPE_ERROR: std::cout << "Type: Error";
	  break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour";
	  break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "Type: Undefined Behaviour";
	  break;
	case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Type: Portability";
	  break;
	case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Type: Performance";
	  break;
	case GL_DEBUG_TYPE_MARKER: std::cout << "Type: Marker";
	  break;
	case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Type: Push Group";
	  break;
	case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Type: Pop Group";
	  break;
	case GL_DEBUG_TYPE_OTHER: std::cout << "Type: Other";
	  break;
  }
  std::cout << std::endl;

  switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: std::cout << "Severity: high";
	  break;
	case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Severity: medium";
	  break;
	case GL_DEBUG_SEVERITY_LOW: std::cout << "Severity: low";
	  break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification";
	  break;
  }
  std::cout << std::endl;
  std::cout << std::endl;
}

#endif //OPENGL_SKELETAL_ANIMATION_SRC_WINDOW_H_
