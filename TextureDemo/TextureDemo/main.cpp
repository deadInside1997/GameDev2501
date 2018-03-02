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
GLuint tex[3];

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
	glGenTextures(3, tex);
	setthisTexture(tex[0], "blueships1.png");
	setthisTexture(tex[1], "orb.png");
	setthisTexture(tex[2], "saw.png");

	glBindTexture(GL_TEXTURE_2D, tex[0]);

}


// Main function that builds and runs the game
int main(void){
	srand(time(NULL)); //seed the random number generator

	Player *me = new Player(100, 0.1, 0.001f, 1, 1.5);
	
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
		float xvelocity = 0;
		float zvelocity = 0;
		float orientation = 90;


		double targetTimer = 0; //will increment to count game time, and be used to spawn targets.
		double deltaTime = 0;
		double cooldown = 10.0;

		Bullet *bullets[10];

		Target *RoadMarkersL[20];
		Target *RoadMarkersR[20];

		int bulletCount = 0; 
		
	
        // Run the main loop
        bool animating = 1;
		deltaTime = glfwGetTime();
        while (!glfwWindowShouldClose(window)){
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

			glm::mat4 bullVec [10]; //a state vector for each bullet and target
			glm::mat4 roadVecL[20];
			glm::mat4 roadVecR[20];


			rot = glm::scale(rot, glm::vec3(0.2, 0.2, 0.2));
	//		rot = glm::translate(rot, glm::vec3(3, 0, 0));
	//		rot = glm::rotate(rot, 50.0f*(float)glfwGetTime(), glm::vec3(0, 0, 1));
			

			// rot = glm::translate(rot, glm::vec3(3*(float)sin(glfwGetTime()), 0, 0)); //moves the ship

			if (GetAsyncKeyState('R') & 0x8000)
			{
				me->changePosition(glm::vec2(0, -4));
			}
		
			if (GetAsyncKeyState('A') & 0x8000)
			{
				xvelocity = -0.1;
			}
			else if (GetAsyncKeyState('D') & 0x8000)
			{
				xvelocity = 0.1;
			}
			else if (xvelocity != 0)
			{
					xvelocity = 0;
			}

			if (GetAsyncKeyState('W') & 0x8000)
			 {
			//	zvelocity = zvelocity +  0.001f; //when holding 'w', the player will slowly accelerate
				me->startEngine();
				if (GetAsyncKeyState(VK_SPACE) & 0x8000)
				{
				//	zvelocity = zvelocity + 0.004f; //when holding 'b', the player will Boost
					me->boost();
				}
				else
				{
					me->holdBoost();
				}
			 }

			 else if (GetAsyncKeyState('S') & 0x8000 /*&& zvelocity > 0*/)
			 {
				 me->brake();
			 } 


			 else if (zvelocity > 0)
			 {
				 
				zvelocity = zvelocity-0.0005f; //if the player isn't holding down 'w' but also isn't actively braking, their speed will decrease but much more slowly
				if (zvelocity < 0)//if our speed would become negative, instead set it to 0
				{
					zvelocity = 0;
				}
			 }
			 else
			 {
				 me->stopEngine();

			 }

			/* if (GetAsyncKeyState(VK_SPACE) & 0x8000)
			 {
				 if (bulletCount < 10 && cooldown == 0)
				 {
					 cooldown = 20; //each time the main loop is run through, this is decrememented by  the amount of time that has passed
					 bullets[bulletCount] = new Bullet; //spawn a new bullet at our position
					 bullets[bulletCount]->setOrientation(orientation);//orientation in this case refers to the player's orientation
					 bullets[bulletCount]->setPosition(position.x,position.y);//position refers to the player's position
					 bulletCount++;
				 }
				 else if( bulletCount >= 10)
				 {
					 bulletCount = 0;

				 }
			 }*/

			 if (position.x >= 4.5 || position.x <= -4.5) 
			 {
				 xvelocity = -xvelocity;
			 }
			 if (position.y >=4.5 || position.y <= -4.5)
			 {
				 position.y = -4;
			 }

			//theta = orientation
            //magnitude = velocity.y


			 
			 position += glm::vec2(xvelocity, zvelocity);  //we add our velocity to the current position
			 me->changePosition(glm::vec2(me->getPosition().x + xvelocity, me->getPosition().y + zvelocity));

			 me->movementPhysics();

			 system("CLS");
			 cout << "Forward speed: "<<zvelocity <<"\n";
			 cout << "Energy: " << me->getEnergy() << "\n";

			 rot = glm::translate(rot, glm::vec3(me ->getPosition().x, me->getPosition().y, 0)); //moves the ship
			 rot = glm::rotate(rot, orientation, glm::vec3(0, 0, 1)); //rotates the ship

			glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &rot[0][0]);
			
			
			
			
			glBindTexture(GL_TEXTURE_2D, tex[0]);

			// Draw 
			glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);

			//glUniform2f(w, 0.6*sin(glfwGetTime()),0.0);
			rot = glm::rotate(rot, 50.0f*(float)glfwGetTime(), glm::vec3(0, 0, 1));

		
			

			glBindTexture(GL_TEXTURE_2D, tex[2]); //change to drawing the saw image
			int distance_to_target;
			for (int i = 0; i < bulletCount; i++)
			{
				
				bullVec[i] = glm::scale(bullVec[i], glm::vec3(0.2, 0.2, 0.2));
				//float radius = 2 + (float)sin(glfwGetTime());
				bullets[i]->setPosition(bullets[i]->getPosition().x + 0.1*cos(bullets[i]->getOrientation()*(PI / 180)), bullets[i]->getPosition().y + 0.1* sin(bullets[i]->getOrientation()*(PI / 180)));
				bullVec[i] = glm::translate(bullVec[i], glm::vec3(bullets[i]->getPosition().x, bullets[i]->getPosition().y, 0));  
				bullVec[i] = glm::rotate(bullVec[i], 100.0f*(float)glfwGetTime()+bullets[i]->getOrientation(), glm::vec3(0, 0, 1));
				glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &bullVec[i][0][0]);
				
				//known bug: sometimes hitting a target with a saw crashes the game with an arrayoutofbounds exception. Something is wrong with my logic and I'm not sure what. 

			

				// Draw 
				glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);

				bullVec[i] = glm::translate(bullVec[i], glm::vec3(-0, 0, 0));
			}
			

			glBindTexture(GL_TEXTURE_2D, tex[1]);  //making the road markers

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


			
			/*for (int i = 0; i < 12; i++) //makes the rotating bullets
			{
				rot = glm::rotate(rot, 30.0f, glm::vec3(0, 0, 1));
				float radius = 2 +(float)sin(glfwGetTime());
				rot = glm::translate(rot, glm::vec3(radius, 0, 0));
				glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &rot[0][0]);

				// Draw 
				glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
				
				rot = glm::translate(rot, glm::vec3(-radius, 0, 0));
			}*/
			
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


 