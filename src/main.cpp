/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> sphere;
shared_ptr<Shape> cube;
shared_ptr<Shape> planet;
shared_ptr<Shape> teapot;
shared_ptr<Shape> bunny;
shared_ptr<Shape> ship;
shared_ptr<Shape> t800;
shared_ptr<Shape> earth;

#define RING_COUNT 200
#define TRIANGLE_COUNT 40

#ifndef M_PI
#define M_PI 3.14159f
#endif


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		static float speed = 0;
		if (w == 1)
		{
			speed += 1 * ftime;
		}
		else if (s == 1)
		{
		    if (speed >= -4 * ftime) {
                speed -= 1 * ftime;
            }
		}
		else if (speed >= 0) {
		    speed -= 0.5f * ftime;
		}
		else if (speed <= 0) {
		    speed += 0.5f * ftime;
		}

		// Fixes the jitter at low speeds
		if (w == 0 && s ==0 && abs(speed) <= 1 * ftime) {
		    speed = 0;
		}

		float yangle=0;
		if (a == 1)
			yangle = -2*ftime;
		else if(d==1)
			yangle = 2*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 0.5f, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, psky, pShip, pPlanet, globeprog, pEarth;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox, InstanceBuffer, NormalBufferCyl, VertexBufferCyl, NormalBufferIDCyl, VertexBufferIDCyl, IndexBufferCyl, VertexArrayIDCyl;

	//texture data
	GLuint Texture;
	GLuint Texture2;
	GLuint PlanetTex0;
	GLuint PlanetTex1;
	GLuint PlanetTex2;
	GLuint PlanetTex3;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{

	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	/* Billboards and Skybox */
	glm::vec4 *ringPositions = new glm::vec4[RING_COUNT];
	int *ringX = new int[RING_COUNT];
	int *ringZ = new int[RING_COUNT];

	void initGeom()
	{

		string resourceDirectory = "../resources";
		// Initialize mesh.
		sphere = make_shared<Shape>();
		sphere->loadMesh(resourceDirectory + "/sphere.obj");
		sphere->resize();
		sphere->init();

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0,  1.0,//LD
			1.0, -1.0,  1.0,//RD
			1.0,  1.0,  1.0,//RU
			-1.0,  1.0,  1.0,//LU
		};
		//make it a bit smaller
		for (int i = 0; i < 12; i++)
			cube_vertices[i] *= 0.5;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		GLfloat cube_norm[] = {
			// front colors
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,

		};
		glGenBuffers(1, &VertexNormDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		glm::vec2 cube_tex[] = {
			// front colors
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(0.0, 0.0),

		};
		glGenBuffers(1, &VertexTexBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {

			// front
			0, 1, 2,
			2, 3, 0,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);


		//generate vertex buffer to hand off to OGL ###########################
		glGenBuffers(1, &InstanceBuffer);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer);
		for (int i = 0; i < RING_COUNT; i++) {
			ringX[i] = (rand() % 10000) - 5000;
			ringZ[i] = (rand() % 10000) - 5000;
			ringPositions[i] = glm::vec4(ringX[i], 0, ringZ[i], 0);
		}
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, RING_COUNT * sizeof(glm::vec4), ringPositions, GL_STATIC_DRAW);
		int position_loc = glGetAttribLocation(prog->pid, "InstancePos");
		for (int i = 0; i < RING_COUNT; i++)
		{
			// Set up the vertex attribute
			glVertexAttribPointer(position_loc + i,              // Location
				4, GL_FLOAT, GL_FALSE,       // vec4
				sizeof(vec4),                // Stride
				(void *)(sizeof(vec4) * i)); // Start offset
											 // Enable it
			glEnableVertexAttribArray(position_loc + i);
			// Make it instanced
			glVertexAttribDivisor(position_loc + i, 1);
		}

		glBindVertexArray(0);

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/Ring.png";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/starfield.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

	}

	void initCylinder() {
		// **** Cylinder ******
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayIDCyl);
		glBindVertexArray(VertexArrayIDCyl);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDCyl);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDCyl);

		// Slap the extra 3 on for 0,0,0
		static GLfloat g_vertex_buffer_data[(TRIANGLE_COUNT * 3 * 2) + 6];

		// Fill vertex buffer
		//Fill Top
		for (int i = 0; i < TRIANGLE_COUNT / 2; i++) {
			g_vertex_buffer_data[i * 3] = 0.95f * cosf(2 * (float)M_PI * ((float)i / (float)(TRIANGLE_COUNT / 2)));
			g_vertex_buffer_data[(i * 3) + 1] = 0.95f * sinf(2 * (float)M_PI * ((float)i / (float)(TRIANGLE_COUNT / 2)));
			g_vertex_buffer_data[(i * 3) + 2] = 1.0f; // Every other vertex scale note
		}


		// Bottom plate verticies
		for (int i = (TRIANGLE_COUNT / 2); i < TRIANGLE_COUNT; i++) {
			g_vertex_buffer_data[i * 3] = 0.95f * cosf(2 * (float)M_PI * ((float)i / (float)(TRIANGLE_COUNT / 2)));
			g_vertex_buffer_data[(i * 3) + 1] = 0.95f * sinf(2 * (float)M_PI * ((float)i / (float)(TRIANGLE_COUNT / 2)));
			g_vertex_buffer_data[(i * 3) + 2] = -1.0f; // Every other vertex scale note
		}

		// Put in 0,0,1 in the vertex buffer
		g_vertex_buffer_data[TRIANGLE_COUNT * 3] = 0.0f;
		g_vertex_buffer_data[(TRIANGLE_COUNT * 3) + 1] = 0.0f;
		g_vertex_buffer_data[(TRIANGLE_COUNT * 3) + 2] = 1.0f;

		// Put in 0,0,-1 in the vertex buffer
		g_vertex_buffer_data[(TRIANGLE_COUNT * 3) + 3] = 0.0f;
		g_vertex_buffer_data[(TRIANGLE_COUNT * 3) + 4] = 0.0f;
		g_vertex_buffer_data[(TRIANGLE_COUNT * 3) + 5] = -1.0f;

		for (int i = TRIANGLE_COUNT + 2; i < (TRIANGLE_COUNT + 2) + (TRIANGLE_COUNT / 2); i++) {
			g_vertex_buffer_data[i * 3] = 0.95f * cosf(2 * (float)M_PI * ((float)i / (float)(TRIANGLE_COUNT / 2)));
			g_vertex_buffer_data[(i * 3) + 1] = 0.95f * sinf(2 * (float)M_PI * ((float)i / (float)(TRIANGLE_COUNT / 2)));
			g_vertex_buffer_data[(i * 3) + 2] = 1.0f; // Every other vertex scale note
		}

		for (int i = (TRIANGLE_COUNT + 2) + (TRIANGLE_COUNT / 2); i < (TRIANGLE_COUNT + 2) + TRIANGLE_COUNT; i++) {
			g_vertex_buffer_data[i * 3] = 0.95f * cosf(2 * (float)M_PI * ((float)i / (float)(TRIANGLE_COUNT / 2)));
			g_vertex_buffer_data[(i * 3) + 1] = 0.95f * sinf(2 * (float)M_PI * ((float)i / (float)(TRIANGLE_COUNT / 2)));
			g_vertex_buffer_data[(i * 3) + 2] = -1.0f; // Every other vertex scale note
		}

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);

		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);



		//****************** Index for sun ********************
		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &IndexBufferCyl);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);

		static GLuint g_index_buffer_data[(TRIANGLE_COUNT * 3) * 2];

		// Connect top plate
		for (GLuint i = 0; i < TRIANGLE_COUNT / 2; i++) {
			g_index_buffer_data[i * 3] = i;

			if (i + 1 == TRIANGLE_COUNT / 2) {
				g_index_buffer_data[(i * 3) + 1] = 0;
			}
			else {
				g_index_buffer_data[(i * 3) + 1] = i + 1;
			}
			g_index_buffer_data[(i * 3) + 2] = TRIANGLE_COUNT;
		}

		// Connect bottom plate
		for (GLuint i = TRIANGLE_COUNT / 2; i < TRIANGLE_COUNT; i++) {
			g_index_buffer_data[i * 3] = i;

			if (i + 1 == TRIANGLE_COUNT) {
				g_index_buffer_data[(i * 3) + 1] = TRIANGLE_COUNT / 2;
			}
			else {
				g_index_buffer_data[(i * 3) + 1] = i + 1;
			}
			g_index_buffer_data[(i * 3) + 2] = TRIANGLE_COUNT + 1;
		}

		for (GLuint i = TRIANGLE_COUNT; i < TRIANGLE_COUNT + (TRIANGLE_COUNT / 2); i++) {
			g_index_buffer_data[i * 3] = i + 2;

			if (i + 1 == TRIANGLE_COUNT + (TRIANGLE_COUNT / 2)) {
				g_index_buffer_data[(i * 3) + 1] = 2 + TRIANGLE_COUNT;
			}
			else {
				g_index_buffer_data[(i * 3) + 1] = i + 3;
			}

			g_index_buffer_data[(i * 3) + 2] = i + 2 + (TRIANGLE_COUNT / 2);

		}

		for (GLuint i = TRIANGLE_COUNT + (TRIANGLE_COUNT / 2); i < TRIANGLE_COUNT * 2; i++) {
			g_index_buffer_data[i * 3] = i + 2;

			if (i + 1 == TRIANGLE_COUNT * 2) {
				g_index_buffer_data[(i * 3) + 1] = 2 + TRIANGLE_COUNT + (TRIANGLE_COUNT / 2);
				g_index_buffer_data[(i * 3) + 2] = TRIANGLE_COUNT + 2;

			}
			else {
				g_index_buffer_data[(i * 3) + 1] = i + 3;
				g_index_buffer_data[(i * 3) + 2] = i + 2 - ((TRIANGLE_COUNT / 2) - 1);
			}

		}

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_index_buffer_data), g_index_buffer_data, GL_DYNAMIC_DRAW);

		//****************** Normals for Cylinder

		static GLfloat g_vertex_normal_data[(TRIANGLE_COUNT * 3 * 2) + 6];

		// Top Plate
		for (unsigned i = 0; i < TRIANGLE_COUNT / 2; i++) {
			g_vertex_normal_data[(i * 3)] = 0;
			g_vertex_normal_data[(i * 3) + 1] = 0;
			g_vertex_normal_data[(i * 3) + 2] = 1.0f;
		}

		// Bottom Plate
		for (unsigned i = TRIANGLE_COUNT / 2; i < TRIANGLE_COUNT; i++) {
			g_vertex_normal_data[(i * 3)] = 0;
			g_vertex_normal_data[(i * 3) + 1] = 0;
			g_vertex_normal_data[(i * 3) + 2] = -1.0f;
		}

		// Top plate
		g_vertex_normal_data[TRIANGLE_COUNT * 3] = 0;
		g_vertex_normal_data[TRIANGLE_COUNT * 3 + 1] = 0;
		g_vertex_normal_data[TRIANGLE_COUNT * 3 + 2] = 1.0f;

		// Bottom plate
		g_vertex_normal_data[TRIANGLE_COUNT * 3 + 3] = 0;
		g_vertex_normal_data[TRIANGLE_COUNT * 3 + 4] = 0;
		g_vertex_normal_data[TRIANGLE_COUNT * 3 + 5] = -1.0f;

		// Edges
		for (unsigned i = TRIANGLE_COUNT; i < TRIANGLE_COUNT + TRIANGLE_COUNT / 2; i++) {
			vec3 vertexA, vertexB, vertexC, v, u, normal;

			vertexA.x = g_vertex_buffer_data[(g_index_buffer_data[i * 3]) * 3];
			vertexA.y = g_vertex_buffer_data[(g_index_buffer_data[i * 3]) * 3 + 1];
			vertexA.z = g_vertex_buffer_data[(g_index_buffer_data[i * 3]) * 3 + 2];

			vertexB.x = g_vertex_buffer_data[(g_index_buffer_data[i * 3 + 1]) * 3];
			vertexB.y = g_vertex_buffer_data[(g_index_buffer_data[i * 3 + 1]) * 3 + 1];
			vertexB.z = g_vertex_buffer_data[(g_index_buffer_data[i * 3 + 1]) * 3 + 2];

			vertexC.x = g_vertex_buffer_data[(g_index_buffer_data[i * 3 + 2]) * 3];
			vertexC.y = g_vertex_buffer_data[(g_index_buffer_data[i * 3 + 2]) * 3 + 1];
			vertexC.z = g_vertex_buffer_data[(g_index_buffer_data[i * 3 + 2]) * 3 + 2];

			v = vertexB - vertexA;
			u = vertexC - vertexA;

			normal = cross(u, v);

			g_vertex_normal_data[(i + 2) * 3] = normal.x;
			g_vertex_normal_data[(i + 2) * 3 + 1] = normal.y;
			g_vertex_normal_data[(i + 2) * 3 + 2] = normal.z;
		}

		glGenBuffers(1, &NormalBufferCyl);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, NormalBufferCyl);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_normal_data), g_vertex_normal_data, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		prog->addAttribute("InstancePos");

		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

        pShip = std::make_shared<Program>();
        pShip->setVerbose(true);
        pShip->setShaderNames(resourceDirectory + "/shipvertex.glsl", resourceDirectory + "/shipfrag.glsl");
		if (!pShip->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
        pShip->addUniform("P");
        pShip->addUniform("V");
        pShip->addUniform("M");
		pShip->addUniform("campos");
        pShip->addAttribute("vertPos");
        pShip->addAttribute("vertNor");
        pShip->addAttribute("vertTex");

        pPlanet = std::make_shared<Program>();
        pPlanet->setVerbose(true);
        pPlanet->setShaderNames(resourceDirectory + "/normal_vertex.glsl", resourceDirectory + "/normal_fragment.glsl");
        if (!pPlanet->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        pPlanet->addUniform("P");
        pPlanet->addUniform("V");
        pPlanet->addUniform("M");
        pPlanet->addUniform("campos");
        pPlanet->addAttribute("vertPos");
        pPlanet->addAttribute("vertNor");
        pPlanet->addAttribute("vertTex");

		// Initialize the GLSL program.
		globeprog = std::make_shared<Program>();
		globeprog->setVerbose(true);
		globeprog->setShaderNames(resourceDirectory + "/globe_vertex.glsl", resourceDirectory + "/globe_fragment.glsl");
		if (!globeprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
		}
		globeprog->addUniform("P");
		globeprog->addUniform("V");
		globeprog->addUniform("M");
		globeprog->addAttribute("vertPos");
		globeprog->addAttribute("vertNor");
		globeprog->addAttribute("vertTex");
		
		// Initialize the GLSL program.
		pEarth = std::make_shared<Program>();
		pEarth->setVerbose(true);
		pEarth->setShaderNames(resourceDirectory + "/earth_vertex.glsl", resourceDirectory + "/earth_fragment.glsl");
		if (!pEarth->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
		}
		pEarth->addUniform("P");
		pEarth->addUniform("V");
		pEarth->addUniform("M");
		pEarth->addUniform("campos");
		pEarth->addAttribute("vertPos");
		pEarth->addAttribute("vertNor");
		pEarth->addAttribute("vertTex");
	}

	/**
	 * Planets
	 */
	void initPlanet() {
		string resourceDirectory = "../resources" ;

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/2k_earth_clouds.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &PlanetTex0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, PlanetTex0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/2k_earth_daymap.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &PlanetTex1);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, PlanetTex1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		// Specular texture
		str = resourceDirectory + "/2k_earth_specular_map.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &PlanetTex2);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, PlanetTex2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		// Night Texture
		str = resourceDirectory + "/2k_earth_nightmap.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &PlanetTex3);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, PlanetTex3);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[FOURTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex6Location = glGetUniformLocation(pEarth->pid, "tex6");//tex, tex2... sampler in the fragment shader
		GLuint Tex7Location = glGetUniformLocation(pEarth->pid, "tex7");
		GLuint Tex8Location = glGetUniformLocation(pEarth->pid, "tex8");
		GLuint Tex9Location = glGetUniformLocation(pEarth->pid, "tex9");
		// Then bind the uniform samplers to texture units:
		glUseProgram(pEarth->pid);
		glUniform1i(Tex6Location, 2);
		glUniform1i(Tex7Location, 3);
		glUniform1i(Tex8Location, 4);
		glUniform1i(Tex9Location, 5);
	}

	void initFont() {

	}

#define INTENSE_COUNT 15
	glm::vec3 *bunnyPositions = new glm::vec3[RING_COUNT];
	glm::vec3 *teapotPositions = new glm::vec3[RING_COUNT];
	glm::vec3 *benderPositions = new glm::vec3[INTENSE_COUNT];
	glm::vec3 *midtermPositions = new glm::vec3[INTENSE_COUNT];

	void initPositions() {
		for (int i = 0; i < INTENSE_COUNT; i++) {
			int randx = (rand() % 1000) - 500;
			int randz = (rand() % 1000) - 500;
			midtermPositions[i] = glm::vec3(randx, 0, randz);
		}
		
		for (int i = 0; i < INTENSE_COUNT; i++) {
			int randx = (rand() % 1000) - 500;
			int randz = (rand() % 1000) - 500;
			benderPositions[i] = glm::vec3(randx, 0, randz);
		}

		for (int i = 0; i < RING_COUNT; i++) {
			int randx = (rand() % 1000) - 500;
			int randz = (rand() % 1000) - 500;
			bunnyPositions[i] = glm::vec3(randx, 0, randz);
		}

		for (int i = 0; i < RING_COUNT; i++) {
			int randx = (rand() % 5000) - 2500;
			int randz = (rand() % 5000) - 2500;
			teapotPositions[i] = glm::vec3(randx, 0, randz);

			printf("RING %d %d %d\n", ringX[i], 0, ringZ[i]);
		}
	}

	void genericInit(shared_ptr<Shape> *shape, string mesh) {
        string resourceDirectory = "../resources" ;
        // Initialize mesh.
        (*shape) = make_shared<Shape>();
        (*shape)->loadMesh(resourceDirectory + "/" + mesh);
        (*shape)->resize();
        (*shape)->init();
	}

	void initShip() {
		string resourceDirectory = "../resources" ;
		string pathmtl = resourceDirectory + "/ship/";
		// Initialize mesh.
		ship = make_shared<Shape>();
		ship->loadMesh(resourceDirectory + "/ship/enterprise1701d.obj", &pathmtl, stbi_load);
		ship->resize();
		ship->init();
	}

	void drawBunny(vec3 pos, vec3 rotation, vec3 scale, double frametime, glm::mat4 V, glm::mat4 M, glm::mat4 P) {
        M = glm::scale(glm::mat4(1.0f), scale) * glm::translate(glm::mat4(1.0f), pos);
        pPlanet->bind();
        //send the matrices to the shaders
        glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(pPlanet->getUniform("campos"), 1, &mycam.pos[0]);

        bunny->draw(pPlanet, false, false);
        pPlanet->unbind();
	}

    // TODO: Scaling is messed up now
    void drawMidtermArm(vec3 pos, vec3 rotation, vec3 scale, double frametime, glm::mat4 V, glm::mat4 M, glm::mat4 P) {
        //animation with the model matrix:
        static float w = 0.0;
        w += 0.01;//rotation angle
        glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)); //glm::rotate(glm::mat4(1.0f), 0.25f, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 bodyScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f,0.3f,0.3f));
        M = TransZ * bodyScale * RotateX;

        // Draw the box using GLSL.
        pPlanet->bind();

        //send the matrices to the shaders
        glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);

        cube->draw(pPlanet, false, false);

        float rotFactor;

        rotFactor = sinf(w);

        // Limb 1
        glm::mat4 armTransZ1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0));
        glm::mat4 Scale1 = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f,0.5f,0.1f));
        glm::mat4 RotateX1 = glm::rotate(glm::mat4(1.0f), rotFactor, glm::vec3(0.0f, 0.0f, 1.0f));

        M = (TransZ * RotateX) * RotateX1 * armTransZ1 * Scale1;

        //send the matrices to the shaders
        glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);

        cube->draw(pPlanet, false, false);


        rotFactor = sinf(w * 3);

        // Limb 2
        glm::mat4 armTransZ2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0));
        glm::mat4 RotateX2 = glm::rotate(glm::mat4(1.0f), rotFactor, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 armTransX2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0));


        M = (TransZ * RotateX   * RotateX1 * armTransZ1) * armTransZ2 * RotateX2 * armTransX2 * Scale1 ;

        //send the matrices to the shaders
        glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);

        cube->draw(pPlanet, false, false);

        rotFactor = sinf(w*6);

        // Limb 3
        glm::mat4 armTransZ3 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.9f, 0));
        glm::mat4 RotateX3 = glm::rotate(glm::mat4(1.0f), rotFactor, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 armTransX3 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0));


        M = (TransZ * RotateX  * RotateX1 * armTransZ1   * armTransZ2 * RotateX2) * armTransZ3  * RotateX3 * armTransX3 * Scale1;

        //send the matrices to the shaders
        glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);

        cube->draw(pPlanet, false, false);


        glBindVertexArray(0);

        pPlanet->unbind();
    }

    void drawTeapot(vec3 pos, vec3 rotation, vec3 scale, double frametime, glm::mat4 V, glm::mat4 M, glm::mat4 P) {
        M = glm::translate(glm::mat4(1.0f), pos) *  glm::rotate(glm::mat4(1.0f), 1.2f, vec3(-M_PI, 0, 0)) *  glm::scale(glm::mat4(1.0f), scale);
        pPlanet->bind();
        //send the matrices to the shaders
        glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(pPlanet->getUniform("campos"), 1, &mycam.pos[0]);

        teapot->draw(pPlanet, false, false);
        pPlanet->unbind();
    }

	void drawBender(vec3 pos, vec3 rotation, vec3 scale, double frametime, glm::mat4 V, glm::mat4 M, glm::mat4 P) {
		static float w = 0;
		static float m = 0;
		static unsigned score = 0;

		m += 0.02f;
		float animFactorL = cos(m) + 0.7f;
		float animFactorR = sin(m) + 0.7f;
		float animFactor2 = cos(m * 2) + 0.7f;

		mat4 globalTrans = glm::translate(glm::mat4(1.0f), pos);

		mat4 globRotate = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));


		// ********* Draw Base

		pPlanet->bind();
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		mat4 baseTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.6f, 0.0f));
		mat4 baseRotate = glm::rotate(glm::mat4(1), M_PI / 2, glm::vec3(0.5f, 0.0f, 0.0f));
		mat4 baseScale = glm::scale(glm::mat4(1), glm::vec3(4.0f, 4.0f, 0.4f));

		M = globalTrans * globRotate * baseTranslate * baseRotate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);

		// Head tube
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		baseTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -1.0f));
		baseRotate = glm::rotate(glm::mat4(1), M_PI / 2, glm::vec3(0.5f, 0.0f, 0.0f));
		baseScale = glm::scale(glm::mat4(1), glm::vec3(0.6f, 0.6f, 1.0f));

		M = globalTrans * globRotate * baseRotate * baseTranslate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);

		// Antenna
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		baseTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -2.5f));
		baseRotate = glm::rotate(glm::mat4(1), M_PI / 2, glm::vec3(0.5f, 0.0f, 0.0f));
		baseScale = glm::scale(glm::mat4(1), glm::vec3(0.05f, 0.05f, 0.3f));

		M = globalTrans * globRotate * baseRotate * baseTranslate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);

		// Body
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		baseTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 1.0f));
		baseRotate = glm::rotate(glm::mat4(1), M_PI / 2, glm::vec3(0.5f, 0.0f, 0.0f));
		baseScale = glm::scale(glm::mat4(1), glm::vec3(1.2f, 1.2f, 1.5f));

		M = globalTrans * globRotate * baseRotate * baseTranslate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);

		// Leg 1
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		baseTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.6f, 3.2f));
		baseRotate = glm::rotate(glm::mat4(1), M_PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
		baseScale = glm::scale(glm::mat4(1), glm::vec3(0.3f, 0.3f, 1.0f));

		M = globalTrans * globRotate * baseRotate * baseTranslate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);

		// Leg 2
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		baseTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.6f, 3.2f));
		baseRotate = glm::rotate(glm::mat4(1), M_PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));

		M = globalTrans * globRotate * baseRotate * baseTranslate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);

		// Arm L 1
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		baseScale = glm::scale(glm::mat4(1), glm::vec3(0.2f, 0.2f, 0.7f));
		baseTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.3f, 0.3f));
		baseRotate = glm::rotate(glm::mat4(1), M_PI / 2, glm::vec3(0.5f, 0.0f, 0.0f));
		mat4 animRot = glm::rotate(glm::mat4(1), animFactorL, glm::vec3(0, 0.5f, 0));

		M = globalTrans * globRotate * baseRotate * animRot * baseTranslate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);

		// Arm L2
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		mat4 base2Translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.7f));
		mat4 baseA2Translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.6f));
		mat4 base2Rotate = glm::rotate(glm::mat4(1), animFactor2, glm::vec3(0, 0.5f, 0.0f));

		M = globalTrans * globRotate * baseRotate * animRot * baseTranslate * baseA2Translate * base2Rotate * base2Translate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);

		// Arm R 1
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		baseScale = glm::scale(glm::mat4(1), glm::vec3(0.2f, 0.2f, 0.7f));
		baseTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1.3f, 0.3f));
		baseRotate = glm::rotate(glm::mat4(1), M_PI / 2, glm::vec3(0.5f, 0.0f, 0.0f));
		animRot = glm::rotate(glm::mat4(1), animFactorR, glm::vec3(0, 0.5f, 0));

		M = globalTrans * globRotate * baseRotate * animRot * baseTranslate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);

		// Arm R2
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		base2Translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.7f));
		baseA2Translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.6f));
		base2Rotate = glm::rotate(glm::mat4(1), animFactor2, glm::vec3(0, 0.5f, 0.0f));

		M = globalTrans * globRotate * baseRotate * animRot * baseTranslate * baseA2Translate * base2Rotate * base2Translate * baseScale;
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDCyl);
		//Bind the index buffer for the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferCyl);
		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, TRIANGLE_COUNT * 3 * 2, GL_UNSIGNED_INT, (void *)0);

		glBindVertexArray(0);
		pPlanet->unbind();

		// L foot cap
		pPlanet->bind();

		mat4 scaleFoot = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
		mat4 transFoot = glm::translate(glm::mat4(1.0f), glm::vec3(0, -4.3f, 0.6f));

		M = globalTrans * globRotate * transFoot * scaleFoot;
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sphere->draw(pPlanet, false, false);
		pPlanet->unbind();

		// R foot cap
		pPlanet->bind();

		scaleFoot = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
		transFoot = glm::translate(glm::mat4(1.0f), glm::vec3(0, -4.3f, -0.6f));

		M = globalTrans * globRotate * transFoot * scaleFoot;
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sphere->draw(pPlanet, false, false);
		pPlanet->unbind();

		// Head Cap
		pPlanet->bind();

		mat4 scaleHead = glm::scale(glm::mat4(1.0f), glm::vec3(0.569f, 0.6f, 0.569f));
		mat4 transHead = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1.8f, 0));

		M = globalTrans * globRotate * transHead * scaleHead;
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sphere->draw(pPlanet, false, false);
		pPlanet->unbind();

		// Antenna topper
		pPlanet->bind();

		mat4 scaleAntTop = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
		mat4 transAntTop = glm::translate(glm::mat4(1.0f), glm::vec3(0, 2.8f, 0));

		M = globalTrans * globRotate * transAntTop * scaleAntTop;
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sphere->draw(pPlanet, false, false);
		pPlanet->unbind();

		// Eye 1
		pPlanet->bind();

		scaleHead = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
		transHead = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 1.7f, 0.15f));

		M = globalTrans * globRotate * transHead * scaleHead;
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sphere->draw(pPlanet, false, false);
		pPlanet->unbind();

		// Eye 2
		pPlanet->bind();

		scaleHead = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
		transHead = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 1.7f, -0.15f));

		M = globalTrans * globRotate * transHead * scaleHead;
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sphere->draw(pPlanet, false, false);
		pPlanet->unbind();

		//Mouth
		pPlanet->bind();

		scaleHead = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.5f));
		transHead = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 1.1f, 0));

		M = globalTrans * globRotate * transHead * scaleHead;
		glUniformMatrix4fv(pPlanet->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pPlanet->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sphere->draw(pPlanet, false, false);
		pPlanet->unbind();

		// **** Draw Globe
		globeprog->bind();

		M = globalTrans * glm::scale(glm::mat4(1.0f), glm::vec3(5, 5, 5));
		glUniformMatrix4fv(globeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(globeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(globeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sphere->draw(globeprog, false, false);
		globeprog->unbind();
	}

	void drawEarth(vec3 pos, vec3 rotation, vec3 scale, double frametime, glm::mat4 V, glm::mat4 M, glm::mat4 P) {
		//animation with the model matrix:
		static float w = 0.0;
		w += 0.3 * frametime;//rotation angle
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

		M = TransZ * RotateY * RotateX * S;
		// Draw the box using GLSL.
		pEarth->bind();
		//send the matrices to the shaders
		glUniformMatrix4fv(pEarth->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pEarth->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pEarth->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pEarth->getUniform("campos"), 1, &mycam.pos[0]);
		sphere->draw(pEarth, false, false);
		pEarth->unbind();
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();

		static unsigned score = 0;

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now

		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;

		for (int i = 0; i < RING_COUNT; i++) {
			if (glm::distance(vec3(mycam.pos.x, 0, mycam.pos.z), vec3(ringX[i], 0, ringZ[i])) <= 100 &&
				glm::distance(vec3(mycam.pos.x, 0, mycam.pos.z), vec3(ringX[i], 0, ringZ[i])) >= -100) {
				score++;

				printf("HIT HIT HIT\n");
				printf("%d %d %d\n", ringX[i], 0, ringZ[i]);
				printf("%d %d %d\n", mycam.pos.x, mycam.pos.y, mycam.pos.z);
			}
		}

		//printf("%d %d %d\n", mycam.pos.x, mycam.pos.y, mycam.pos.z);

		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * RotateXSky * SSky;

		// Draw the box using GLSL.
		psky->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		sphere->draw(psky, false, false);
		glEnable(GL_DEPTH_TEST);

		psky->unbind();

		//animation with the model matrix:
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle
		float trans = 0;// sin(t) * 2;
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3 + trans));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransZ * RotateY * RotateX * S;

		// Draw the Target billboards
		prog->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);

		glBindVertexArray(VertexArrayID);
		//actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		mat4 Vi = glm::transpose(V);
		Vi[0][3] = 0;
		Vi[1][3] = 0;
		Vi[2][3] = 0;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);

		M = S * Vi;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, RING_COUNT);
		glBindVertexArray(0);
		prog->unbind();

		for (int i = 0; i < RING_COUNT; i++) {
			drawBunny(bunnyPositions[i], vec3(), vec3(20, 20, 20), frametime, V, M, P);
			drawTeapot(teapotPositions[i], vec3(), vec3(30, 30, 30), frametime, V, M, P);
		}

		for (int i = 0; i < INTENSE_COUNT; i++) {
			drawMidtermArm(midtermPositions[i], vec3(), vec3(), frametime, V, M, P);
		}

		drawBender(benderPositions[0], vec3(), vec3(), frametime, V, M, P);
		drawEarth(vec3(20, 0, 20), vec3(), vec3(10,10,10), frametime, V, M, P);

        // Tip rate calculation
        static float tiprate = 0;

        if (mycam.a && tiprate >= -0.75f) {
            tiprate -= .005;
        }
        else if (mycam.d && tiprate <= 0.75) {
            tiprate += .005;
        }
        else if (tiprate <= 0) {
            tiprate += .009;
        }
        else if (tiprate >= 0) {
            tiprate -= .009;
        }

        if (!mycam.a && !mycam.d && abs(tiprate) <= .01) {
            tiprate = 0;
        }

		M = glm::translate(glm::mat4(1.0f), camp) * glm::rotate(glm::mat4(1), mycam.rot.y, glm::vec3(0, -1, 0)) *
		        glm::translate(glm::mat4(1.0f), vec3(0,-0.20f,-1.8f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)) *
		        glm::rotate(glm::mat4(1), 3.14159f, glm::vec3(tiprate, 1, 0));
		pShip->bind();
		//send the matrices to the shaders
		glUniformMatrix4fv(pShip->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pShip->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pShip->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pShip->getUniform("campos"), 1, &mycam.pos[0]);
		ship->draw(pShip, false, false);
        pShip->unbind();
	}
};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();
	application->initCylinder();
	application->genericInit(&bunny, "bunny.obj");
	application->genericInit(&cube, "cube.obj");
	application->genericInit(&teapot, "teapot.obj");
	application->initShip();
	application->initPlanet();
	application->initPositions();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
