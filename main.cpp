#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glew.h>
#include <GL/freeglut.h>

#include "math_func.h"
#include "stb_image.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "teapot.h"
#include "skybox.h"

#include <windows.h>
#include <mmsystem.h>
#include<glm.hpp>
#include "loader.h"

int width = 1000;
int height = 1000;
int specularStrength = 0;
float specularStrengthCook = 0.0;

GLuint ObjectShaderProgramID;
GLuint ObjectShaderReflection;
GLuint ObjectShaderRefraction;
GLuint ObjectShaderFresnal;
GLuint ObjectShaderCD;
GLuint textureID;

GLuint skyboxVAO;
GLuint skyboxVBO;
GLuint objectVAO = 0;
GLuint objectVBO = 0;
GLuint objectNormalVBO = 0;
GLuint objectloc1;
GLuint objectloc2;
GLuint skyboxloc1;

GLfloat rotatez = 0.0f;


//LoadObj obj1("OBJFiles/cube.obj");


std::string readShaderSource(const std::string& fileName)
{
	std::ifstream file(fileName.c_str());
	if (file.fail()) {
		std::cerr << "Error loading shader called " << fileName << std::endl;
		exit(EXIT_FAILURE);
	}

	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	return stream.str();
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (ShaderObj == 0) {
		std::cerr << "Error creating shader type " << ShaderType << std::endl;
		exit(EXIT_FAILURE);
	}

	/* bind shader source code to shader object */
	std::string outShader = readShaderSource(pShaderText);
	const char* pShaderSource = outShader.c_str();
	glShaderSource(ShaderObj, 1, (const GLchar * *)& pShaderSource, NULL);

	/* compile the shader and check for errors */
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling shader type " << ShaderType << ": " << InfoLog << std::endl;
		exit(EXIT_FAILURE);
	}
	glAttachShader(ShaderProgram, ShaderObj); /* attach compiled shader to shader programme */
}

GLuint CompileShaders(const char* pVShaderText, const char* pFShaderText)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint ShaderProgramID = glCreateProgram();
	if (ShaderProgramID == 0) {
		std::cerr << "Error creating shader program" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(ShaderProgramID, pVShaderText, GL_VERTEX_SHADER);
	AddShader(ShaderProgramID, pFShaderText, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(ShaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		exit(EXIT_FAILURE);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(ShaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(ShaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		exit(EXIT_FAILURE);
	}
	return ShaderProgramID;
}


bool loadcubemapside(
	GLuint texture, GLenum side_target, const char* file_name) {
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	int x, y, n;
	int force_channels = 4;
	stbi_set_flip_vertically_on_load(false);
	unsigned char*  image_data = stbi_load(
		file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// non-power-of-2 dimensions check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(stderr,
			"WARNING: image %s is not power-of-2 dimensions\n",
			file_name);
	}

	// copy image data into 'target' side of cube map
	glTexImage2D(
		side_target,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data);
	free(image_data);
	return true;
}

void create_cube_map(
	const char* front,
	const char* back,
	const char* top,
	const char* bottom,
	const char* left,
	const char* right,
	GLuint* tex_cube) {
	// generate a cube-map texture to hold all the sides
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, tex_cube);

	// load each image and copy into a side of the cube-map texture
	loadcubemapside(*tex_cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front);
	loadcubemapside(*tex_cube, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back);
	loadcubemapside(*tex_cube, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top);
	loadcubemapside(*tex_cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom);
	loadcubemapside(*tex_cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left);
	loadcubemapside(*tex_cube, GL_TEXTURE_CUBE_MAP_POSITIVE_X, right);
	// format cube map texture
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void generateObjectBufferTeapot(GLuint TempObjectShaderProgramID)
{
	GLuint vp_vbo = 0;

	objectloc1 = glGetAttribLocation(TempObjectShaderProgramID, "vertex_position");
	objectloc2 = glGetAttribLocation(TempObjectShaderProgramID, "vertex_normals");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	//glBufferData(GL_ARRAY_BUFFER, 3 * obj1.getNumVertices() * sizeof(float), obj1.getVertices(), GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof(float), teapot_vertex_points, GL_STATIC_DRAW);
	GLuint vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	//glBufferData(GL_ARRAY_BUFFER, 3 * obj1.getNumVertices() * sizeof(float), obj1.getNormals(), GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof(float), teapot_normals, GL_STATIC_DRAW);

	glGenVertexArrays(1, &objectVAO);
	glBindVertexArray(objectVAO);

	glEnableVertexAttribArray(objectloc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(objectloc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(objectloc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(objectloc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}



void init(void) {

	GLuint vbo;
	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), skybox_points, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	create_cube_map("SkyboxImages/negz.jpg", "SkyboxImages/posz.jpg", "SkyboxImages/posy.jpg",
		"SkyboxImages/negy.jpg", "SkyboxImages/negx.jpg", "SkyboxImages/posx.jpg", &textureID);

	//Skybox
	ObjectShaderProgramID = CompileShaders("skybox_vertex.glsl","skybox_fragment.glsl");

	// Reflection
	ObjectShaderReflection = CompileShaders("reflection_vertex.glsl", "reflection_fragment.glsl");
	generateObjectBufferTeapot(ObjectShaderReflection);
	//ObjectShaderRefraction = CompileShaders("1v.glsl", "1.glsl");
	//generateObjectBufferTeapot(ObjectShaderReflection);

	//// Refraction
	ObjectShaderRefraction = CompileShaders("refraction_vertex.glsl", "refraction_fragment.glsl");
	generateObjectBufferTeapot(ObjectShaderRefraction);

	//// Fresnal
	ObjectShaderFresnal = CompileShaders("fresnal_vertex.glsl", "fresnal_fragment.glsl");
	generateObjectBufferTeapot(ObjectShaderFresnal);

	////Chromatic Dispersion
	ObjectShaderCD = CompileShaders("cromatic_vertex.glsl", "cromatic_fragment.glsl");
	generateObjectBufferTeapot(ObjectShaderCD);
}

void display() {

	// skybox
	//glEnable(GL_DEPTH);
	//glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(ObjectShaderProgramID);

	int matrix_location = glGetUniformLocation(ObjectShaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(ObjectShaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(ObjectShaderProgramID, "proj");
	mat4 view = look_at(vec3(0.0f, 0.0f, 1.0f), vec3(0, 0, -1), vec3(0.0, 1.0, 0.0));
	mat4 persp_proj = perspective(100, (float)width / (float)height, 0.01, 100.0);
	mat4 model = identity_mat4();

	glViewport(0, 0, width, height);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	//Reflection
	glUseProgram(ObjectShaderReflection);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

	matrix_location = glGetUniformLocation(ObjectShaderReflection, "model");
	view_mat_location = glGetUniformLocation(ObjectShaderReflection, "view");
	proj_mat_location = glGetUniformLocation(ObjectShaderReflection, "proj");
	
	mat4 view_reflect = translate(identity_mat4(), vec3(0.0, 0.0, -40.0));
	mat4 persp_proj_reflect = perspective(50, (float)width / (float)height, 0.01, 100.0);
	mat4 model_reflect = rotate_y_deg(identity_mat4(), rotatez);

	glViewport((width / 2 - 500), (height / 2), width / 2, (height / 2 - 50));
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj_reflect.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_reflect.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model_reflect.m);
	glBindVertexArray(objectVAO);
	//glDrawArrays(GL_TRIANGLES, 0, obj1.getNumVertices());
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	glBindVertexArray(0);


	// Refraction
	glUseProgram(ObjectShaderRefraction);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

	matrix_location = glGetUniformLocation(ObjectShaderRefraction, "model");
	view_mat_location = glGetUniformLocation(ObjectShaderRefraction, "view");
	proj_mat_location = glGetUniformLocation(ObjectShaderRefraction, "proj");

	mat4 view_refract = translate(identity_mat4(), vec3(0.0, 0.0, -40.0));
	mat4 persp_proj_refract = perspective(50, (float)width / (float)height, 0.01, 100.0);
	mat4 model_refract = rotate_y_deg(identity_mat4(), rotatez);

	glViewport((width / 2), (height / 2), width / 2, (height / 2 - 50));
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj_refract.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_refract.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model_refract.m);
	glBindVertexArray(objectVAO);
	//glDrawArrays(GL_TRIANGLES, 0, obj1.getNumVertices());
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	glBindVertexArray(0);

	//Fresnal
	glUseProgram(ObjectShaderFresnal);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

	matrix_location = glGetUniformLocation(ObjectShaderFresnal, "model");
	view_mat_location = glGetUniformLocation(ObjectShaderFresnal, "view");
	proj_mat_location = glGetUniformLocation(ObjectShaderFresnal, "proj");

	mat4 view_f = translate(identity_mat4(), vec3(0.0, 0.0, -40.0));
	mat4 persp_proj_f = perspective(50, (float)width / (float)height, 0.01, 100.0);
	mat4 model_f = rotate_y_deg(identity_mat4(), rotatez);

	glViewport((width / 2-500), (height / 2 - 500), width / 2, (height / 2 - 50));
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj_f.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_f.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model_f.m);
	glBindVertexArray(objectVAO);
	//glDrawArrays(GL_TRIANGLES, 0, obj1.getNumVertices());
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	glBindVertexArray(0);

	//Chromatic Dispersion
	glUseProgram(ObjectShaderCD);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

	matrix_location = glGetUniformLocation(ObjectShaderCD, "model");
	view_mat_location = glGetUniformLocation(ObjectShaderCD, "view");
	proj_mat_location = glGetUniformLocation(ObjectShaderCD, "proj");

	mat4 view_cd = translate(identity_mat4(), vec3(0.0, 0.0, -40.0));
	mat4 persp_proj_cd = perspective(50, (float)width / (float)height, 0.01, 100.0);
	mat4 model_cd = rotate_y_deg(identity_mat4(), rotatez);

	glViewport((width / 2), (height / 2-500), width / 2, (height / 2 - 50));
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj_cd.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_cd.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model_cd.m);
	glBindVertexArray(objectVAO);
	//glDrawArrays(GL_TRIANGLES, 0, obj1.getNumVertices());
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	glBindVertexArray(0);

	glutSwapBuffers();
}

void keyPress(unsigned char key, int xmouse, int ymouse) {
	std::cout << specularStrength << "Keypress: " << key << std::endl;
	switch (key) {
	case('a'):
		specularStrength += 5;
		break;
	case('q'):
		if (specularStrength > 0)
			specularStrength -= 5;
		break;
	case ('w'):
		specularStrengthCook += 0.1;
		break;
	case ('s'):
		if (specularStrengthCook > 0.1)
			specularStrengthCook -= 0.1;
		break;
	}
};


void updateScene() {

	rotatez += 0.5f;
	// Draw the next frame
	glutPostRedisplay();
}

int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow(argv[1]);

	glutDisplayFunc(display);

	//glutIdleFunc(updateScene);
	glutKeyboardFunc(keyPress);
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(res) << std::endl;
		return EXIT_FAILURE;
	}

	init();
	glutMainLoop();
	return 0;
}