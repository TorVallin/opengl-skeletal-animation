#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * Shader.cpp
 * Contains the implementation of a shader class. Loads vertex and fragment
 * shaders that can be used to render_lines objects.
 *
 * Author: Tor Vallin (c18tvn@cs.umu.se)
 * Credit: Some of the structure and ideas of this class were inspired by
 * the Shader class examples in LearnOpenGL, by Joey Devries. E.g. the
 * shader loading and idea of having setter/use shader abstractions 
 * in the Shader class.
 */

class Shader {
 public:
  unsigned int shaderID;

  Shader();
  ~Shader() {
	cleanup();
  }

  void cleanup() {
	glDeleteProgram(shaderID);
  }

  /**
   * @brief Creats a new shader object with the given shader files.
   * @param vertexPath Path to the vertex shader.
   * @param fragmentPath Path to the fragment shader.
  */
  Shader(const char *vertexPath, const char *fragmentPath);

  /**
   * @brief Sets the current shader as active.
  */
  void use() const;

  // Loads a shader from the given path
  void loadShader(const std::string& vertexPath, const std::string& fragmentPath);
  // Reloads the most recent shader path from disk.
  void reload();

  void setBool(const std::string &name, bool value) const;

  void setInt(const std::string &name, int value) const;

  void setFloat(const std::string &name, float value) const;
  void setVec2(const std::string &name, const glm::vec2 &value) const;
  void setVec2(const std::string &name, float x, float y) const;

  void setVec3(const std::string &name, const glm::vec3 &value) const;
  void setVec3(const std::string &name, float x, float y, float z) const;

  void setVec4(const std::string &name, const glm::vec4 &value) const;
  void setVec4(const std::string &name, float x, float y, float z, float w);

  void setMat2(const std::string &name, const glm::mat2 &mat) const;

  void setMat3(const std::string &name, const glm::mat3 &mat) const;

  void setMat4(const std::string &name, const glm::mat4 &mat) const;

  [[nodiscard]] std::string getVertexPath() const {
	return m_vertexPath;
  }

  [[nodiscard]] std::string getFragmentPath() const {
	return m_fragmentPath;
  }

 private:
  std::string m_vertexPath, m_fragmentPath;

  /**
   * @brief Checks if there were any errors when compiling the shaders, if
   * errors were encountered, they are printed to the terminal.
   * @param shader ID of the shader.
   * @param type Type of shader (vertex, fragment or program).
   *
   * @note Credit to LearnOpenGL for this function.
  */
  static void checkCompileErrors(GLuint shader, std::string type);
};
#endif