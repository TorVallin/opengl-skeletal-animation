//
// Created by tor on 3/26/23.
//

#ifndef OPENGL_SKELETAL_ANIMATION_SRC_MODELS_TEXTURELOADER_H_
#define OPENGL_SKELETAL_ANIMATION_SRC_MODELS_TEXTURELOADER_H_


#include <glad/glad.h>
#include <string>
#include <iostream>

class TextureLoader {
 public:
  [[nodiscard]] static bool load_texture(const std::string &filePath, unsigned int &textureId);
};

#endif //OPENGL_SKELETAL_ANIMATION_SRC_MODELS_TEXTURELOADER_H_
