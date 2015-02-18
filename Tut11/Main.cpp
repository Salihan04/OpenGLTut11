#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "Main.h"
#include "Shader.h"

using namespace std;
using namespace glm;

//Properties
GLuint screenWidth = 700, screenHeight = 500;

struct Character {
	GLuint textureID;	//ID handle of the glyph texture
	ivec2 size;			//Size of glyph
	ivec2 bearing;		//Offset from baseline to left/top of glyph
	GLuint advance;		//Offset to advance to next glyph
};

map<GLchar, Character> characters;

//FreeType
FT_Library ft;
FT_Face face;

//Pointers to Shader objects;
Shader *shaderPtr;

GLuint vbo;

int main(int argc, char **argv)
{
	//initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialise GLFW\n");
		return -1;
	}

	//Creating window
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, 0);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "OpenGLTut11 - Text Rendering", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetWindowPos(window, 400, 200);

	//initialize GLEW
	glewExperimental = true;		// Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	//Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	//Set OpenGL options
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		cout << "ERROR::FREETYPE: Could not init FreeType Library" << endl;

	//Load font as face
	if (FT_New_Face(ft, "arial.ttf", 0, &face))
		cout << "ERROR::FREETYPE: Failed to load font" << endl;

	//Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);

	//Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	initText();

	//Setup and compile our shaders
	//need to be after glewInit(), otherwise got error
	Shader shader("VertexShader.vs", "FragmentShader.fs");
	shaderPtr = &shader;

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);

		renderScene();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

void initText()
{
	for (GLubyte c = 0; c < 128; c++)
	{
		//Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			cout << "ERROR::FREETYTPE: Failed to load Glyph" << endl;
			continue;
		}

		//Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
			);

		//Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Now store character for later use
		Character character = {
			texture,
			ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		characters.insert(pair<GLchar, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	//Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//Use GL_DYNAMIC_DRAW because we will update vbo's memory quite often
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
}

void RenderText(Shader* s, string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color)
{
	//Activate corresponding render state	
	s->Use();
	glUniform3f(glGetUniformLocation(s->Program, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);

	//Iterate through all characters
	string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = characters[*c];

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;

		//Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos, ypos, 0.0, 1.0 },
			{ xpos + w, ypos, 1.0, 1.0 },

			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos + w, ypos, 1.0, 1.0 },
			{ xpos + w, ypos + h, 1.0, 0.0 }
		};

		//Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		//Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);

		//Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6) * scale;		//Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 projection = ortho(0.0f, 800.0f, 0.0f, 600.0f);

	//Use the shader
	shaderPtr->Use();

	glUniformMatrix4fv(glGetUniformLocation(shaderPtr->Program, "projection"), 1, GL_FALSE, value_ptr(projection));

	//Configure vbo for texture quads
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	RenderText(shaderPtr, "This is sample text", 25.0f, 25.0f, 1.0f, vec3(0.5, 0.8f, 0.2f));
	RenderText(shaderPtr, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, vec3(0.3, 0.7f, 0.9f));
}