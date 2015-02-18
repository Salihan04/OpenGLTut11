#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <glm/glm.hpp>
#include "Shader.h"

void initText();
void RenderText(Shader* s, string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
void renderScene();

#endif