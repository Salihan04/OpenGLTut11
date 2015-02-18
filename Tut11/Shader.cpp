#include "Shader.h"
#include <vector>
#include <algorithm>

//Constructor generates the shader on the fly
Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
{
	//Create the shaders
	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//Read the Vertex Shader code from the file
	string VertexShaderCode;
	ifstream VertexShaderStream(vertexPath, ios::in);
	if (VertexShaderStream.is_open())
	{
		string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	//Read the Fragment Shader code from the file
	string FragmentShaderCode;
	ifstream FragmentShaderStream(fragmentPath, ios::in);
	if (FragmentShaderStream.is_open()){
		string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	//Compile Vertex Shader
	printf("Compiling shader : %s\n", vertexPath);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShader, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShader);

	//Check Vertex Shader
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	//Compile Fragment Shader
	printf("Compiling shader : %s\n", fragmentPath);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShader, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShader);

	//Check Fragment Shader
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShader, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	//Link the program
	fprintf(stdout, "Linking program\n");
	this->Program = glCreateProgram();
	glAttachShader(this->Program, VertexShader);
	glAttachShader(this->Program, FragmentShader);
	glLinkProgram(this->Program);

	//Check the program
	glGetProgramiv(this->Program, GL_LINK_STATUS, &Result);
	glGetProgramiv(this->Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	vector<char> ProgramErrorMessage(max(InfoLogLength, int(1)));
	glGetProgramInfoLog(this->Program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
}

//Uses the current shader
void Shader::Use() 
{
	glUseProgram(this->Program);
}

Shader::~Shader()
{

}