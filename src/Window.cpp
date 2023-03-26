//
// Created by tor on 3/23/23.
//

#include "Window.h"
#include "models/TextureLoader.h"

void Window::run() {
  GLFWwindow *glfw_window = this->init_glfw();

  if (glfw_window == nullptr) {
	std::cerr << "Could not create window, exiting" << std::endl;
  }

  configure_opengl();

  // Creates a basic camera
  glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
  glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), VEC_UP);
  glm::mat4
	  projection_matrix = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

  Shader shader = Shader("../shaders/basic.vert.glsl", "../shaders/basic.frag.glsl");
  shader.use();
  shader.setMat4("projection", projection_matrix);
  shader.setMat4("view", view_matrix);
  shader.setMat4("model", model_matrix);

  Shader skel_shader = Shader("../shaders/skeletal_animation.vert.glsl", "../shaders/textured.frag.glsl");
  skel_shader.use();
  glm::mat4 animation_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
  animation_model_matrix = glm::scale(animation_model_matrix, glm::vec3(0.01f, 0.01f, 0.01f));
  skel_shader.setMat4("projection", projection_matrix);
  skel_shader.setMat4("view", view_matrix);
  skel_shader.setMat4("model", animation_model_matrix);

  Grid grid{};

  std::optional<Model>
	  character_model_opt = AnimatedModelLoader::load_model("../assets/character.fbx", "../assets/run.fbx");
  if (!character_model_opt) {
	std::cerr << "Could not load character model, exiting";
	return;
  }
  unsigned int texture_id;
  auto res = TextureLoader::load_texture("../assets/skin.png", texture_id);
  assert(res);
  auto character_model = *character_model_opt;

  double delta_time;
  double last_frame = glfwGetTime();

  while (!glfwWindowShouldClose(glfw_window)) {
	double current_frame = glfwGetTime();
	delta_time = current_frame - last_frame;
	last_frame = current_frame;

	character_model.update_skinning_matrix(delta_time);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render current frame
	shader.use();
	grid.render();

	skel_shader.use();
	// transfer the skinning matrices to the GPU
	const auto transforms = character_model.skinning_matrices;
	for (unsigned int i = 0; i < transforms.size(); i++) {
	  skel_shader.setMat4("skinning_matrices[" + std::to_string(i) + "]",
						  transforms[i]);
	}

	Renderer::render_model(character_model);

	glfwSwapBuffers(glfw_window);
	glfwPollEvents();
  }
}

GLFWwindow *Window::init_glfw() const {
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
