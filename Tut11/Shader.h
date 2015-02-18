#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

#include <GL/glew.h>

class Shader
{
public:
	GLuint Program;
	
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	void Use();
	~Shader();
};

#endif