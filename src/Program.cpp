//
// Created by tor on 3/23/23.
//

#include "Program.h"
#include "TextureLoader.h"

static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
								   GLsizei length, const char *message, const void *userParam);

void Program::run() const {
  GLFWwindow *glfw_window = this->init_glfw();

  if (glfw_window == nullptr) {
	std::cerr << "Could not create window, exiting\n";
	return;
  }

  configure_opengl();

  // Creates a basic camera
  glm::mat4 view_matrix = glm::lookAt(glm::vec3{3.0f, 6.0f, 5.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
  glm::mat4
	  projection_matrix = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

  // Setup the grid shader
  Shader shader = Shader("../shaders/basic.vert.glsl", "../shaders/basic.frag.glsl");
  shader.use();
  shader.setMat4("projection", projection_matrix);
  shader.setMat4("view", view_matrix);
  shader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)));

  // Setup the skeletal animation shader
  Shader skeletal_animation_shader = Shader("../shaders/skeletal_animation.vert.glsl", "../shaders/textured.frag.glsl");
  skeletal_animation_shader.use();
  glm::mat4 animation_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
  animation_model_matrix = glm::scale(animation_model_matrix, glm::vec3(0.01f, 0.01f, 0.01f));
  skeletal_animation_shader.setMat4("projection", projection_matrix);
  skeletal_animation_shader.setMat4("view", view_matrix);
  skeletal_animation_shader.setMat4("model", animation_model_matrix);

  Grid grid{};

  std::optional<Model>
	  character_model_opt = AnimatedModelLoader::load_model("../assets/character.fbx", "../assets/run.fbx");
  if (!character_model_opt) {
	std::cerr << "Could not load character model, exiting\n";
	return;
  }
  unsigned int texture_id;
  auto res = TextureLoader::load_texture("../assets/skin.png", texture_id);
  if (!res) {
	std::cerr << "Could not load character texture, exiting\n";
	return;
  }
  auto character_model = *character_model_opt;

  double delta_time;
  double last_frame = glfwGetTime();

  while (!glfwWindowShouldClose(glfw_window)) {
	double current_frame = glfwGetTime();
	delta_time = current_frame - last_frame;
	last_frame = current_frame;

	character_model.update_skinning_matrix(delta_time);

	// --- Render current frame
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();
	grid.render();

	skeletal_animation_shader.use();
	// transfer the skinning matrices to the GPU
	const auto transforms = character_model.skinning_matrices;
	for (unsigned int i = 0; i < transforms.size(); i++) {
	  skeletal_animation_shader.setMat4("skinning_matrices[" + std::to_string(i) + "]", transforms[i]);
	}

	Renderer::render_model(character_model);

	glfwSwapBuffers(glfw_window);
	glfwPollEvents();
  }
}

GLFWwindow *Program::init_glfw() const {
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
