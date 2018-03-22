#include <iostream>
#include <windows.h>
#include <stdexcept>
#include <algorithm> //required for max() function
#include <string>
#define GLEW_STATIC
#include <GL/glew.h> // window management library
#include <GL/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //
#include <SOIL/SOIL.h> // read image file
#include <time.h>      //to seed the random number generator
using namespace std;
// Macro for printing exceptions
#define PrintException(exception_object)\
	std::cerr << exception_object.what() << std::endl
const float PI = 3.14159265;
#include "Player.h"

class Bullet{
	const int lifetime = 100;
	glm::vec2 position;
	int timer;
	int speed;
	float myOrientation;
public:
	void setOrientation(float);
	float getOrientation();
	void setPosition(float, float);
	glm::vec2 getPosition();
}bullet;

class Target {
	glm::vec2 position;
	int timer;
public:
	void setPosition(float, float);
	glm::vec2 getPosition();
}target;


void Bullet::setOrientation(float angle)
{
	myOrientation = angle;
}
float Bullet::getOrientation()
{
	return myOrientation;
}
void Bullet::setPosition(float x, float y)
{
	position = glm::vec2(x, y);
}
glm::vec2 Bullet::getPosition()
{
	return position;
}

void Target::setPosition(float x, float y)
{
	position = glm::vec2(x, y);
}
glm::vec2 Target::getPosition()
{
	return position;
}

// Globals that define the OpenGL window and viewport
const std::string window_title_g = "Single Sprite Demo";
const unsigned int window_width_g = 800;
const unsigned int window_height_g = 600;
const glm::vec3 viewport_background_color_g(0.0, 0.0, 0.2);

// Global texture info
GLuint tex[14]; //this is the number of textures, right?

// Source code of vertex shader
const char *source_vp = "#version 130\n\
\n\
// Vertex buffer\n\
in vec2 vertex;\n\
in vec3 color;\n\
in vec2 uv;\n\
out vec2 uv_interp;\n\
\n\
// Uniform (global) buffer\n\
uniform mat4 x;\n\
\n\
// Attributes forwarded to the fragment shader\n\
out vec4 color_interp;\n\
\n\
\n\
void main()\n\
{\n\
	vec4 t;\n\
	t = vec4(vertex, 0.0, 1.0);\n\
    gl_Position = x*t;\n\
	\n\
    color_interp = vec4(color, 1.0);\n\
	uv_interp = uv;\n\
}";

//source code of the vertex shader used for particles
const char *source_vpart = "#version 131\n\
\n\
// Vertex buffer\n\
in vec2 vertex;\n\
in vec2 dir;\n\
in float t;\n\
in vec2 uv;\n\
out vec2 uv_interp;\n\
\n\
// Uniform (global) buffer\n\
uniform mat4 x;\n\
uniform float time;\n\
\n\
// Attributes forwarded to the fragment shader\n\
out vec4 color_interp;\n\
\n\
\n\
void main()\n\
{\n\
	vec4 ppos;\n\
	float acttime;\n\
	float speed = 14.0;\n\
	float gravity = -9.8;\n\
	acttime = mod(time + t*10, 10.0);\n\
//	acttime = mod(time,10);\n\
    ppos = vec4(vertex.x+dir.x*acttime*speed , vertex.y+dir.y*acttime*speed+ 0.5*gravity*acttime*acttime, 0.0, 1.0); \n\
	gl_Position = x * ppos; \n\
	\n\
	color_interp = vec4(uv, 0.5, 1.0); \n\
	uv_interp = uv; \n\
}";


// Source code of fragment shader
const char *source_fp = "\n\
#version 130\n\
\n\
// Attributes passed from the vertex shader\n\
in vec4 color_interp;\n\
in vec2 uv_interp;\n\
\n\
uniform sampler2D onetex;\n\
\n\
\n\
void main()\n\
{\n\
	vec4 color = texture2D(onetex, uv_interp) ;\n\
	gl_FragColor = vec4(color.r,color.g,color.b,color.a);\n\
    if(gl_FragColor.a < 0.9)\n\
	{\n\
		//discard; \n\
		gl_FragDepth = 10000;\n\
//		gl_FragColor = vec4(color.r,color.g,color.b,0.5);\n\
	} \n\
//	 gl_FragColor = color_interp;\n\
}";

GLuint SetupParticleShaders() // returns ID of newly created program
{

	// Set up shaders

	// Create a shader from vertex program source code
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &source_vpart, NULL);
	glCompileShader(vs);

	// Check if shader compiled successfully
	GLint status;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		glGetShaderInfoLog(vs, 512, NULL, buffer);
		throw(std::ios_base::failure(std::string("Error compiling vertex shader:") + std::string(buffer)));
	}

	// Create a shader from the fragment program source code
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &source_fp, NULL);
	glCompileShader(fs);

	// Check if shader compiled successfully
	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		glGetShaderInfoLog(fs, 512, NULL, buffer);
		throw(std::ios_base::failure(std::string("Error compiling fragment shader: ") + std::string(buffer)));
	}

	// Create a shader program linking both vertex and fragment shaders
	// together
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	// Check if shaders were linked successfully
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		glGetShaderInfoLog(program, 512, NULL, buffer);
		throw(std::ios_base::failure(std::string("Error linking shaders: ") +
			std::string(buffer)));
	}

	// Delete memory used by shaders, since they were already compiled
	// and linked
	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;

}

void AttributeBinding(GLuint program)
{

	// Set attributes for shaders
	// Should be consistent with how we created the buffers for the particle elements
		GLint vertex_att = glGetAttribLocation(program, "vertex");
	glVertexAttribPointer(vertex_att, 2, GL_FLOAT, GL_FALSE, 7 *
		sizeof(GLfloat), 0);
	glEnableVertexAttribArray(vertex_att);

	GLint dir_att = glGetAttribLocation(program, "dir");
	glVertexAttribPointer(dir_att, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat),
		(void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(dir_att);

	GLint time_att = glGetAttribLocation(program, "t");
	glVertexAttribPointer(time_att, 1, GL_FLOAT, GL_FALSE, 7 *
		sizeof(GLfloat), (void *)(4 * sizeof(GLfloat)));
	glEnableVertexAttribArray(time_att);

	GLint tex_att = glGetAttribLocation(program, "uv");
	glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat),
		(void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(tex_att);

}
// Callback for when a key is pressed
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){

    // Quit the program when pressing 'q'
  //  if (key == GLFW_KEY_Q && action == GLFW_PRESS){
 //       glfwSetWindowShouldClose(window, true);
 //   }
}


// Callback for when the window is resized
void ResizeCallback(GLFWwindow* window, int width, int height){

    // Set OpenGL viewport based on framebuffer width and height
    glViewport(0, 0, width, height);

}

int CreateParticleArray(void) {

	// Each particle is a square with four vertices and two triangles

	// Number of attributes for vertices and faces
	const int vertex_attr = 7;  // 7 attributes per vertex: 2D (or 3D) position(2), direction(2), 2D texture coordinates(2), time(1)
		//   const int face_att = 3; // Vertex indices (3)

		GLfloat vertex[] = {
		//  square (two triangles)
		//  Position      Color             Texcoords
		-0.5f, 0.5f,    1.0f, 0.0f, 0.0f,	0.0f, 0.0f, // Top-left
		0.5f, 0.5f,   	0.0f, 1.0f, 0.0f,	1.0f, 0.0f, // Top-right
		0.5f, -0.5f,    0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Bottom-right
		-0.5f, -0.5f,   1.0f, 1.0f, 1.0f,	0.0f, 1.0f  // Bottom-left
	};

	GLfloat particleatt[1000 * vertex_attr];
	float theta, r, tmod;

	for (int i = 0; i < 1000; i++)
	{
		if (i % 4 == 0)
		{
			theta = (2 * (rand() % 10000) / 10000.0f - 1.0f)*0.13f;
			r = 0.7f + 0.3*(rand() % 10000) / 10000.0f;
			tmod = (rand() % 10000) / 10000.0f;
		}

		particleatt[i*vertex_attr + 0] = vertex[(i % 4) * 7 + 0];
		particleatt[i*vertex_attr + 1] = vertex[(i % 4) * 7 + 1];

		particleatt[i*vertex_attr + 2] = sin(theta)*r;
		particleatt[i*vertex_attr + 3] = cos(theta)*r;


		particleatt[i*vertex_attr + 4] = tmod;

		particleatt[i*vertex_attr + 5] = vertex[(i % 4) * 7 + 5];
		particleatt[i*vertex_attr + 6] = vertex[(i % 4) * 7 + 6];


	}


	GLuint face[] = {
		0, 1, 2, // t1
		2, 3, 0  //t2
	};

	GLuint manyface[1000 * 6];

	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 6; j++)
			manyface[i * 6 + j] = face[j] + i * 4;
	}

	GLuint vbo, ebo;

	// Create buffer for vertices
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleatt), particleatt,
		GL_STATIC_DRAW);

	// Create buffer for faces (index buffer)
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(manyface), manyface,
		GL_STATIC_DRAW);

	// Return number of elements in array buffer
	return sizeof(manyface);

}


void drawParticles(GLuint particleprogram, int particlesize)
{

	// Select proper shader program to use
	glUseProgram(particleprogram);

	//set displacement
	int matrixLocation = glGetUniformLocation(particleprogram, "x");
	int timeLocation = glGetUniformLocation(particleprogram, "time");

	glm::mat4 rot = glm::mat4();
	glm::mat4 world = glm::mat4();

	float k = glfwGetTime();
	//rot = glm::rotate(rot, -k * 360 / 6.283f, glm::vec3(0, 0, 1));
	rot = glm::translate(rot, glm::vec3(0.5, 0, 0));
	rot = glm::scale(rot, glm::vec3(0.1, 0.1, 0.1));
	// get ready to draw, load matrix
	glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &rot[0][0]);
	glUniform1f(timeLocation, k);
	glBindTexture(GL_TEXTURE_2D, tex[1]);

	// Draw 
	glDrawElements(GL_TRIANGLES, 6 * particlesize, GL_UNSIGNED_INT, 0);

}

// Create the geometry for a square (with two triangles)
// Return the number of array elements that form the square
int CreateSquare(void) {

	// The face of the square is defined by four vertices and two triangles

	// Number of attributes for vertices and faces
//	const int vertex_att = 7;  // 7 attributes per vertex: 2D (or 3D) position (2), RGB color (3), 2D texture coordinates (2)
//	const int face_att = 3; // Vertex indices (3)

	GLfloat vertex[]  = {
		//  square (two triangles)
		   //  Position      Color             Texcoords
		-0.5f, 0.5f,	 1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // Top-left
		0.5f, 0.5f,		 0.0f, 1.0f, 0.0f,		1.0f, 0.0f, // Top-right
		0.5f, -0.5f,	 0.0f, 0.0f, 1.0f,		1.0f, 1.0f, // Bottom-right
		-0.5f, -0.5f,	 1.0f, 1.0f, 1.0f,		0.0f, 1.0f  // Bottom-left
	};


	GLuint face[] = {
		0, 1, 2, // t1
		2, 3, 0  //t2
	};

	GLuint vbo, ebo;

	// Create buffer for vertices
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// Create buffer for faces (index buffer)
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face), face, GL_STATIC_DRAW);

	// Return number of elements in array buffer
	return sizeof(face);

}


void setthisTexture(GLuint w, char *fname)
{
	glBindTexture(GL_TEXTURE_2D, w);

	int width, height;
	unsigned char* image = SOIL_load_image(fname, &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


}

void setallTexture(void)
{
//	tex = new GLuint[3];
	glGenTextures(14, tex);
	
	setthisTexture(tex[0], "orb.png");
	setthisTexture(tex[1], "road.png");
	setthisTexture(tex[2],  "blueFalcon_left_4.png");
	setthisTexture(tex[3],  "blueFalcon_left_3.png");
	setthisTexture(tex[4],  "blueFalcon_left_2.png");
	setthisTexture(tex[5],  "blueFalcon_left_1.png");
	setthisTexture(tex[6],  "blueFalcon_centered.png");
	setthisTexture(tex[7],  "blueFalcon_right_1.png");
	setthisTexture(tex[8],  "blueFalcon_right_2.png");
	setthisTexture(tex[9],  "blueFalcon_right_3.png");
	setthisTexture(tex[10], "blueFalcon_right_4.png");
	setthisTexture(tex[11], "Engine_particle_1.png");
	setthisTexture(tex[12], "Engine_particle_2.png");
	setthisTexture(tex[13], "Engine_particle_3.png");
	glBindTexture(GL_TEXTURE_2D, tex[0]);

}


// Main function that builds and runs the game
int main(void){
	srand(time(NULL)); //seed the random number generator

	Player *me = new Player(300, 0.1, 0.001f, 1, 1.5);
	
	try {
        // Initialize the window management library (GLFW)
        if (!glfwInit()){
            throw(std::runtime_error(std::string("Could not initialize the GLFW library")));
        }

        // Create a window and its OpenGL context
        GLFWwindow* window;
        window = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), NULL, NULL);
        if (!window){
            glfwTerminate();
            throw(std::runtime_error(std::string("Could not create window")));
        }

        /* Make the window's OpenGL context the current one */
        glfwMakeContextCurrent(window);

        // Initialize the GLEW library to access OpenGL extensions
        // Need to do it after initializing an OpenGL context
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK){
            throw(std::runtime_error(std::string("Could not initialize the GLEW library: ")+std::string((const char *) glewGetErrorString(err))));
        }

        // Set up z-buffer for rendering
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

		// Enable Alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Create geometry of the square
		int size = CreateSquare();

        // Set up shaders

        // Create a shader from vertex program source code
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &source_vp, NULL);
        glCompileShader(vs);

        // Check if shader compiled successfully
        GLint status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE){
            char buffer[512];
            glGetShaderInfoLog(vs, 512, NULL, buffer);
            throw(std::ios_base::failure(std::string("Error compiling vertex shader: ")+std::string(buffer)));
        }

        // Create a shader from the fragment program source code
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &source_fp, NULL);
        glCompileShader(fs);

        // Check if shader compiled successfully
        glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE){
            char buffer[512];
            glGetShaderInfoLog(fs, 512, NULL, buffer);
            throw(std::ios_base::failure(std::string("Error compiling fragment shader: ")+std::string(buffer)));
        }

        // Create a shader program linking both vertex and fragment shaders
        // together
        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        // Check if shaders were linked successfully
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE){
            char buffer[512];
            glGetShaderInfoLog(program, 512, NULL, buffer);
            throw(std::ios_base::failure(std::string("Error linking shaders: ")+std::string(buffer)));
        }

        // Delete memory used by shaders, since they were already compiled
        // and linked
        glDeleteShader(vs);
        glDeleteShader(fs);

        // Set attributes for shaders
        // Should be consistent with how we created the buffers for the square
        GLint vertex_att = glGetAttribLocation(program, "vertex");
        glVertexAttribPointer(vertex_att, 2, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(vertex_att);

        GLint color_att = glGetAttribLocation(program, "color");
        glVertexAttribPointer(color_att, 3, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (void *) (2 *sizeof(GLfloat)));
        glEnableVertexAttribArray(color_att);

        GLint tex_att = glGetAttribLocation(program, "uv");
        glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (void *) (5 *sizeof(GLfloat)));
        glEnableVertexAttribArray(tex_att);

		setallTexture();
		
        // Set event callbacks
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetFramebufferSizeCallback(window, ResizeCallback);
		glm::vec2 position = glm::vec2(0, 0); //changing this to be the player's position
		float lateralSpeed=0;
		int lateralDirection=1;
		float velocity = 0;
		float orientation = 90;


		double targetTimer = 0; //will increment to count game time, and be used to spawn targets.
		double deltaTime = 0;
		double cooldown = 10.0;


		Target *RoadMarkersL[20];
		Target *RoadMarkersR[20];

		
	
        // Run the main loop
        bool animating = 1;
		deltaTime = glfwGetTime();
		while (!glfwWindowShouldClose(window)) {
			// Clear background
			glClearColor(viewport_background_color_g[0],
				viewport_background_color_g[1],
				viewport_background_color_g[2], 0.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// Draw the square


			// Select proper shader program to use
			glUseProgram(program);

			//set displacement
			int matrixLocation = glGetUniformLocation(program, "x");
			//glUniform2f(w, 0.0, 0.6*sin(glfwGetTime()));
			glm::mat4 rot = glm::mat4();

			glm::mat4 roadVecL[20];
			glm::mat4 roadVecR[20];

			glm::mat4 trackVec = glm::mat4();


			rot = glm::scale(rot, glm::vec3(0.4, 0.4, 0.4));
			trackVec = glm::scale(trackVec,glm::vec3(1,1,1));
			//		rot = glm::translate(rot, glm::vec3(3, 0, 0));
			//		rot = glm::rotate(rot, 50.0f*(float)glfwGetTime(), glm::vec3(0, 0, 1));


					// rot = glm::translate(rot, glm::vec3(3*(float)sin(glfwGetTime()), 0, 0)); //moves the ship


			if (GetAsyncKeyState('P') & 0x8000)
			{
				//pause the game
			}

			if (GetAsyncKeyState('R') & 0x8000)
			{
				me->changePosition(glm::vec2(0, -2));
			}

			if (GetAsyncKeyState('A') & 0x8000)
			{
				
				lateralSpeed = min(lateralSpeed+0.005,0.1);
				lateralDirection = -1; //left
				me->turnLeft(); ///for now all this does is update the sprite
			}
			else if (GetAsyncKeyState('D') & 0x8000)
			{
			
				lateralSpeed = min(lateralSpeed+0.005,0.1);
				lateralDirection = 1;//right
				me->turnRight();
			}
			else if (lateralSpeed != 0)
			{
				if (lateralSpeed > 0)
				{
					lateralSpeed -= 0.05;
					if (lateralSpeed < 0)
					{
						lateralSpeed= 0;
						lateralDirection = 0;
					}
				}
				//lateralMovement = 0;
			}

			if (GetAsyncKeyState('W') & 0x8000)
			 {
				//zvelocity = zvelocity +  0.001f; //when holding 'w', the player will slowly accelerate
				me->startEngine();

				if (GetAsyncKeyState(VK_SPACE) & 0x8000)
				{
				//	zvelocity = zvelocity + 0.004f; //while also holding down space, the player will Boost
					me->boost();
					if (me->getSprite() > 1 && me->getSprite() < 6)
					{
						me->changeSpriteIndex(me->getSprite() + 1);
					}
					if (me->getSprite() < 11 && me->getSprite() > 6)
					{
						me->changeSpriteIndex(me->getSprite() - 1);
					}
				}
				else
				{
					me->holdBoost();
				}
			 }

			 else if (GetAsyncKeyState('S') & 0x8000 && true)
			 {
				 cout << "Holding down brake\n ";
				 me->brake();
			 } 


			 else if (velocity > 0)
			 {
				 
				//velocity = velocity-0.0005f; //if the player isn't holding down 'w' but also isn't actively braking, their speed will decrease but much more slowly
				if (velocity < 0)//if our speed would become negative, instead set it to 0
				{
					velocity = 0;
				}
			 }
			 else
			 {
				 me->stopEngine();

			 }

			 if (position.x >= 1.5)  
			 {
				lateralDirection = -1;//
				lateralSpeed = 0.1;
			 }
			 else if (position.x <= -1.5)
			 {
				 lateralDirection = 1;//
				 lateralSpeed = 0.1;
			 }
			 if (position.y >=1.5 || position.y <= -1.5)
			 {
				 //position.y = 0;
			 }
			 
			 position += glm::vec2(lateralSpeed * lateralDirection, velocity);  //we add our velocity to the current position
			
			 me->changePosition(glm::vec2(me->getPosition().x + lateralSpeed* lateralDirection, me->getPosition().y + velocity* sin(me->getDirection())));

			 me->movementPhysics();

			 system("CLS");
			 cout << "Forward speed: "<<velocity <<"\n";
			 cout << "Energy: " << me->getEnergy() << "\n";

			 rot = glm::translate(rot, glm::vec3(me ->getPosition().x, me->getPosition().y, 0)); //moves the ship
			 //rot = glm::rotate(rot, orientation, glm::vec3(0, 0, 1)); //rotates the ship

			glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &rot[0][0]);
			
			


			
			glBindTexture(GL_TEXTURE_2D, tex[me->getSprite()]);

			// Draw 
			glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);

			//glUniform2f(w, 0.6*sin(glfwGetTime()),0.0);
			rot = glm::rotate(rot, 50.0f*(float)glfwGetTime(), glm::vec3(0, 0, 1));

			glBindTexture(GL_TEXTURE_2D, tex[1]);  //making the racetrack

			trackVec = glm::translate(trackVec, glm::vec3(0, 0, 0));


			glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);

			

			
				//	glDrawArrays(GL_TRIANGLES, 0, 6); // if glDrawArrays be used, glDrawElements will be ignored 		
		

		/*	glBindTexture(GL_TEXTURE_2D, tex[1]);  //making the road markers

			for (int j = 0; j < 20; j++)
			{
				roadVecL[j] = glm::scale(roadVecL[j], glm::vec3(0.2, 0.2, 0.2));
				roadVecR[j] = glm::scale(roadVecL[j], glm::vec3(0.2, 0.2, 0.2));
				RoadMarkersL[j] = new Target;
				RoadMarkersR[j] = new Target;
				RoadMarkersL[j]->setPosition(-1, (j / 20) - 10);
				RoadMarkersR[j]->setPosition(1, (j / 20) - 10);
				roadVecL[j] = glm::translate(roadVecL[j], glm::vec3(RoadMarkersL[j]->getPosition().x, RoadMarkersL[j]->getPosition().y, 0));
				roadVecR[j] = glm::translate(roadVecR[j], glm::vec3(RoadMarkersR[j]->getPosition().x, RoadMarkersR[j]->getPosition().y, 0));
			}
			glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
			*/
		//	glDrawArrays(GL_TRIANGLES, 0, 6); // if glDrawArrays be used, glDrawElements will be ignored 

            // Update other events like input handling
            glfwPollEvents();

            // Push buffer drawn in the background onto the display
            glfwSwapBuffers(window);
			cooldown = cooldown - 1;// ((int)glfwGetTime() - (int)deltaTime);


			if (cooldown < 0) cooldown = 0;
        }
    }
    catch (std::exception &e){
        PrintException(e);
    }

    return 0;
}


 