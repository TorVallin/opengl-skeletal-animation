//
// Created by tor on 3/26/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_MODELS_TEXTURELOADER_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_MODELS_TEXTURELOADER_H_

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.hpp>
#include <glad/glad.h>
#include <string>
#include <iostream>

static bool load_texture(std::string filePath, unsigned int &textureId) {
  stbi_set_flip_vertically_on_load(true);
  int x, y, n;

  unsigned char *data = stbi_load(filePath.c_str(), &x, &y, &n, 0);

  if (!data) {
	std::cout << "Error loading filename: " << filePath << std::endl;
  } else {
	glGenTextures(1, &textureId);
	GLenum texFormat = GL_RGB;
	glBindTexture(GL_TEXTURE_2D, textureId);
	if (n == 1)
	  texFormat = GL_RED;
	else if (n == 3)
	  texFormat = GL_RGB;
	else if (n == 4)
	  texFormat = GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, texFormat, x, y, 0, texFormat, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	stbi_image_free(data);
	return true;
  }

  return false;
}

#endif //OPENGL_SKELETAL_ANIMATION_SRC_MODELS_TEXTURELOADER_H_
