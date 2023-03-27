#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader/Shader.h"

/**
 * Shader.cpp
 * Contains the implementation of a shader class. Loads vertex and fragment
 * shaders that can be used to render_lines objects.
 *
 * Author: Tor Vallin (c18tvn@cs.umu.se)
 * Credit: Much of the structure and ideas of this class were inspired by
 * the Shader class examples in LearnOpenGL, by Joey Devries. E.g. the
 * shader loading and idea of having setter abstractions in the shader.
 */

/**
 * @brief Creats a new shader object with the given shader files.
 * @param vertexPath Path to the vertex shader.
 * @param fragmentPath Path to the fragment shader.
 */
Shader::Shader(const char *vertexPath, const char *fragmentPath) {
  loadShader(std::string(vertexPath), std::string(fragmentPath));
}

void Shader::reload() {
  loadShader(m_vertexPath, m_fragmentPath);
}

void Shader::loadShader(const std::string &vertexPath, const std::string &fragmentPath) {
  m_vertexPath = vertexPath;
  m_fragmentPath = fragmentPath;
  std::ifstream vertexStream(vertexPath), fragStream(fragmentPath);

  std::string vertexCode((std::istreambuf_iterator<char>(vertexStream)),
						 (std::istreambuf_iterator<char>()));

  std::string fragmentCode((std::istreambuf_iterator<char>(fragStream)),
						   (std::istreambuf_iterator<char>()));

  if (vertexCode.empty()) {
	std::cerr << "Shader::loadShader - empty vertex shader\n";
	throw std::runtime_error("Empty vertex shader");
  }
  if (fragmentCode.empty()) {
	std::cerr << "Shader::loadShader - empty fragment shader\n";
	throw std::runtime_error("Empty fragment shader");
  }

  const char *vCode = vertexCode.c_str();
  const char *fCode = fragmentCode.c_str();
  unsigned int vertexID, fragmentID;

  vertexID = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexID, 1, &vCode, nullptr);
  glCompileShader(vertexID);
  checkCompileErrors(vertexID, "VERTEX");

  fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentID, 1, &fCode, nullptr);
  glCompileShader(fragmentID);
  checkCompileErrors(fragmentID, "FRAGMENT");

  shaderID = glCreateProgram();
  glAttachShader(shaderID, vertexID);
  glAttachShader(shaderID, fragmentID);

  glLinkProgram(shaderID);
  checkCompileErrors(shaderID, "PROGRAM");

  glDeleteShader(vertexID);
  glDeleteShader(fragmentID);
}

/**
 * @brief Sets the current shader as active.
 */
void Shader::use() const {
  glUseProgram(shaderID);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const {
  glUniform2fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string &name, float x, float y) const {
  glUniform2f(glGetUniformLocation(shaderID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const {
  glUniform3f(glGetUniformLocation(shaderID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const {
  glUniform4fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string &name, float x, float y, float z, float w) {
  glUniform4f(glGetUniformLocation(shaderID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const {
  glUniformMatrix2fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
  glUniformMatrix3fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
  glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setBool(const std::string &name, bool value) const {
  glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
}

/**
 * @brief Checks if there were any errors when compiling the shaders, if
 * errors were encountered, they are printed to the terminal.
 * @param shader ID of the shader.
 * @param type Type of shader (vertex, fragment or program).
 *
 * @note Credit to LearnOpenGL for this function.
 */
void Shader::checkCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
	  glGetShaderInfoLog(shader, 1024, NULL, infoLog);
	  std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
				<< infoLog << std::endl;
	}
  } else {
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
	  glGetProgramInfoLog(shader, 1024, NULL, infoLog);
	  std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
				<< infoLog << std::endl;
	}
  }
}