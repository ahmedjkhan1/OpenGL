// Ahmed Jawad Khan
// 014-262-117
// Final Project 

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "..\SOIL\src\SOIL.h"
#include <vector>
#include <iostream>


//Used to represent a vehicle
struct vehicle {
	float x_pos, y_pos;	//Position of a vehicle
	bool isAlive;		//If the vehicle is alive and must be rendered
	bool isBullet;
};
float wheel_rotation = 0.0f;
float vehicle_velocity = 3.0f;

//We use a container to keep the data corresponding to vehicles
std::vector<vehicle> collection;

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer };
enum Attrib_IDs { vPosition = 0 };

const GLint NumBuffers = 2;
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint location;
GLuint cam_mat_location;
GLuint proj_mat_location;
GLuint texture[2];	//Array of pointers to textrure data in VRAM. We use two textures in this example.

glm::vec2 shoot_dir;

const GLuint NumVertices = 28;

//Height of camera (player) from the level
float height = 0.8f;

//Player motion speed for movement and pitch/yaw
float travel_speed = 30.0f;		//Motion speed
float mouse_sensitivity = 0.01f;	//Pitch/Yaw speed

//Used for tracking mouse cursor position on screen
int x0 = 0;
int y_0 = 0;

//Transformation matrices and camera vectors
glm::mat4 model_view;
glm::vec3 unit_z_vector = glm::vec3(0, 0, 1);	//Assigning a meaningful name to (0,0,1) :-)
glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, height);
glm::vec3 forward_vector = glm::vec3(1, 1, 0);	//Forward vector is parallel to the level at all times (No pitch)

//The direction which the camera is looking, at any instance
glm::vec3 looking_dir_vector = glm::vec3(1, 1, 0);
glm::vec3 up_vector = unit_z_vector;
glm::vec3 side_vector = glm::cross(up_vector, forward_vector);

//Used to measure time between two frames
float oldTimeSinceStart = 0;
float deltaTime;

//Creating and rendering bunch of objects on the scene to interact with
const int Num_Obstacles = 10;
float obstacle_data[Num_Obstacles][3];
int killCount = 0;
void initialize_vehicles();
void drawVehicle(float scale, glm::vec2 direction);
void drawBullet(float scale, glm::vec2 direction);

bool isColliding(vehicle v1, vehicle v2) {
	bool result = false;
	if (v1.isBullet && v2.isBullet) {
		// does nothing
	}
	else if (v1.isAlive && v2.isAlive) {
		float x_dist = abs(v1.x_pos - v2.x_pos);
		float y_dist = abs(v1.y_pos - v2.y_pos);

		if (x_dist <= 0.9 && y_dist <= 0.9) {
			result = true;
			if (v1.isBullet) {
				killCount = killCount + 1;
			}
			else if (v2.isBullet) {
				killCount = killCount + 1;
			}
		}
	}
	
	return result;
}

// checks to see if in the vector collection of vehicles and bullets if there are any collisions happening 
void checkCollision() {
	for (int i = 0; i < collection.size(); i++) {
		vehicle v = collection[i];
		for (int j = 0; j < collection.size(); j++) {
			vehicle temp = collection[j];
			if (i != j) {
				if (isColliding(v, temp)) {
					v.isAlive = false;
					temp.isAlive = false;
					collection[i] = v;
					collection[j] = temp;

				}
			}
		}
	}
}


// This function will check if the player gets 10 kills which means he wins the game so exit with 0 status
void gameOver() {
	if (killCount == 10) {
		exit(0);
	}
}
//This function runs on every frame
//The function iterates through the collection of vehicles
//It updates the position of the vehices and render them, respectively.
void renderVehicles() {

	// check to see if user wins the game
	gameOver();

	// check to see if any object is colliding both bullet or vehicles colliding between each other
	checkCollision();

	for (int i = 0; i < collection.size(); i++) {
		vehicle v = collection[i];

		if (v.isAlive) {	//We render the vehicle ONLY if it is supposed to be rendered
			glm::vec2 moving_dir = glm::vec2((0 - v.x_pos), (0 - v.y_pos));

			moving_dir = glm::normalize(moving_dir);

			if (v.isBullet) {

				moving_dir = shoot_dir;

				v.x_pos += deltaTime * vehicle_velocity * moving_dir.x;
				v.y_pos += deltaTime * vehicle_velocity * moving_dir.y;

				model_view = glm::translate(model_view, glm::vec3(v.x_pos, v.y_pos, 0));
				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

				drawBullet(1.0, moving_dir);
			}
			else {
				v.x_pos += deltaTime * vehicle_velocity * moving_dir.x;
				v.y_pos += deltaTime * vehicle_velocity * moving_dir.y;

				model_view = glm::translate(model_view, glm::vec3(v.x_pos, v.y_pos, 0));
				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

				drawVehicle(1.0, moving_dir);
			}

			model_view = glm::mat4(1.0);
			glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
			
			// checks to see if user hits the vehicle
			if (abs(cam_pos.x - v.x_pos) < (0.1 / 2.0 + 0.9 / 2.0) && abs(cam_pos.y - v.y_pos) < (0.1 / 2.0 + 0.9 / 2.0)) {
				v.isAlive = false;

				cam_pos = glm::rotate(forward_vector, 45.0f, glm::vec3(0, 1, 0));
			}

			//Check to see if the vehicle has reached to 'approximately' zero
			if (abs(v.x_pos) < 0.1 && abs(v.y_pos) < 0.1) v.isAlive = false;

			//Updating vehicle info in the collection vector<vehicle>
			collection[i] = v;
		}
	}
}

//Helper function to generate a random float number within a range
float randomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

void shoot() {

	vehicle bullet;

	bullet.x_pos = cam_pos.x + forward_vector.x;
	bullet.y_pos = cam_pos.y + forward_vector.y;
	bullet.isAlive = TRUE;
	bullet.isBullet = TRUE;

	collection.push_back(bullet);

}

// inititializing buffers, coordinates, setting up pipeline, etc.
void init(void) {
	glEnable(GL_DEPTH_TEST);

	//Normalizing all vectors
	up_vector = glm::normalize(up_vector);
	forward_vector = glm::normalize(forward_vector);
	looking_dir_vector = glm::normalize(looking_dir_vector);
	side_vector = glm::normalize(side_vector);

	//Randomizing the position and scale of obstacles
	for (int i = 0; i < Num_Obstacles; i++) {
		obstacle_data[i][0] = randomFloat(-50, 50); //X
		obstacle_data[i][1] = randomFloat(-50, 50); //Y
		obstacle_data[i][2] = randomFloat(0.1, 10.0); //Scale
	}

	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up

	GLfloat vertices[NumVertices][3] = {

		{ -100.0, -100.0, 0.0 }, //Plane to walk on and a sky
		{ 100.0, -100.0, 0.0 },
		{ 100.0, 100.0, 0.0 },
		{ -100.0, 100.0, 0.0 },

		{ -0.45, -0.45 ,-0.45 }, // bottom face
		{ 0.45, -0.45 ,-0.45 },
		{ 0.45, 0.45 ,-0.45 },
		{ -0.45, 0.45 ,-0.45 },

		{ -0.45, -0.45 ,0.45 }, //top face
		{ 0.45, -0.45 ,0.45 },
		{ 0.45, 0.45 ,0.45 },
		{ -0.45, 0.45 ,0.45 },

		{ 0.45, -0.45 , -0.45 }, //left face
		{ 0.45, 0.45 , -0.45 },
		{ 0.45, 0.45 ,0.45 },
		{ 0.45, -0.45 ,0.45 },

		{ -0.45, -0.45, -0.45 }, //right face
		{ -0.45, 0.45 , -0.45 },
		{ -0.45, 0.45 ,0.45 },
		{ -0.45, -0.45 ,0.45 },

		{ -0.45, 0.45 , -0.45 }, //front face
		{ 0.45, 0.45 , -0.45 },
		{ 0.45, 0.45 ,0.45 },
		{ -0.45, 0.45 ,0.45 },

		{ -0.45, -0.45 , -0.45 }, //back face
		{ 0.45, -0.45 , -0.45 },
		{ 0.45, -0.45 ,0.45 },
		{ -0.45, -0.45 ,0.45 },
	};

	//These are the texture coordinates for the second texture
	GLfloat textureCoordinates[28][2] = {
		0.0f, 0.0f,
		200.0f, 0.0f,
		200.0f, 200.0f,
		0.0f, 200.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
	};

	//Creating our texture:
	//This texture is loaded from file. To do this, we use the SOIL (Simple OpenGL Imaging Library) library.
	//When using the SOIL_load_image() function, make sure the you are using correct patrameters, or else, your image will NOT be loaded properly, or will not be loaded at all.
	GLint width1, height1;
	unsigned char* textureData1 = SOIL_load_image("grass.png", &width1, &height1, 0, SOIL_LOAD_RGB);

	GLint width2, height2;
	unsigned char* textureData2 = SOIL_load_image("apple.png", &width2, &height2, 0, SOIL_LOAD_RGB);

	glGenBuffers(2, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindAttribLocation(program, 0, "vPosition");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
	glBindAttribLocation(program, 1, "vTexCoord");
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	location = glGetUniformLocation(program, "model_matrix");
	cam_mat_location = glGetUniformLocation(program, "camera_matrix");
	proj_mat_location = glGetUniformLocation(program, "projection_matrix");

	///////////////////////TEXTURE SET UP////////////////////////

	//Allocating two buffers in VRAM
	glGenTextures(2, texture);

	//First Texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData1);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//And now, second texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData2);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//////////////////////////////////////////////////////////////
}

// draw a basic cube to act as bullet but smaller scale and higher off the ground to make seem more like a bullet
void drawBullet(float scale, glm::vec2 direction) {
	//Select the second texture (apple.png) when drawing the second geometry (cube)
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	model_view = glm::translate(model_view, glm::vec3(0.0f, 0.0f, 0.6f));	//To level the vehicles on the ground
	model_view = glm::rotate(model_view, atan(direction.y / direction.x), unit_z_vector);	//Rotate the vehicle such that its front-face moves perpendecular to its forward direction
	model_view = glm::scale(model_view, glm::vec3(scale/4.0, scale/4.0, scale/4.0));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glDrawArrays(GL_QUADS, 4, 24);

}
//Helper function to draw a cube
void drawVehicle(float scale, glm::vec2 direction)
{
	//Select the second texture (apple.png) when drawing the second geometry (cube)
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	model_view = glm::translate(model_view, glm::vec3(0.0f, 0.0f, 0.45f));	//To level the vehicles on the ground
	model_view = glm::rotate(model_view, atan(direction.y / direction.x), unit_z_vector);	//Rotate the vehicle such that its front-face moves perpendecular to its forward direction
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glDrawArrays(GL_QUADS, 4, 24);


	//Drawing wheels
	glm::mat4 tmp_mv = model_view;

	model_view = glm::translate(model_view, glm::vec3(scale / 2.0, scale / 2.0, -0.45f));
	model_view = glm::rotate(model_view, wheel_rotation, glm::vec3(0, 1, 0));
	model_view = glm::scale(model_view, glm::vec3(scale / 4.0, scale / 4.0, scale / 4.0));

	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glDrawArrays(GL_QUADS, 4, 24);

	model_view = tmp_mv;

	//Drawing wheels
	// First Wheel 
	glm::mat4 tmp_mv1 = model_view;

	model_view = glm::translate(model_view, glm::vec3(scale / 2.0, scale / 2.0, -0.45f));
	model_view = glm::rotate(model_view, wheel_rotation, glm::vec3(0, 1, 0));
	model_view = glm::scale(model_view, glm::vec3(scale / 4.0, scale / 4.0, scale / 4.0));

	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glDrawArrays(GL_QUADS, 4, 24);

	model_view = tmp_mv1;

	// Second Wheel 
	glm::mat4 tmp_mv2 = model_view;

	model_view = glm::translate(model_view, glm::vec3(scale / -2.0, scale / 2.0, -0.45f));
	model_view = glm::rotate(model_view, wheel_rotation, glm::vec3(0, 1, 0));
	model_view = glm::scale(model_view, glm::vec3(scale / 4.0, scale / 4.0, scale / 4.0));

	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glDrawArrays(GL_QUADS, 4, 24);

	model_view = tmp_mv2;

	// Third Wheel 
	glm::mat4 tmp_mv3 = model_view;

	model_view = glm::translate(model_view, glm::vec3(scale / 2.0, scale / -2.0, -0.45f));
	model_view = glm::rotate(model_view, wheel_rotation, glm::vec3(0, 1, 0));
	model_view = glm::scale(model_view, glm::vec3(scale / 4.0, scale / 4.0, scale / 4.0));

	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glDrawArrays(GL_QUADS, 4, 24);

	model_view = tmp_mv3;

	// Fourth Wheel 
	glm::mat4 tmp_mv4 = model_view;

	model_view = glm::translate(model_view, glm::vec3(scale / -2.0, scale / -2.0, -0.45f));
	model_view = glm::rotate(model_view, wheel_rotation, glm::vec3(0, 1, 0));
	model_view = glm::scale(model_view, glm::vec3(scale / 4.0, scale / 4.0, scale / 4.0));

	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glDrawArrays(GL_QUADS, 4, 24);

	model_view = tmp_mv4;
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
}

//Initialize 
void initialize_vehicles() {
	//Rendering obstacles obstacles
	for (int i = 0; i < Num_Obstacles; i++) {
		vehicle v;
		v.x_pos = obstacle_data[i][0];
		v.y_pos = obstacle_data[i][1];
		v.isAlive = TRUE;
		v.isBullet = false;
		collection.push_back(v);
	}
	//cout << "All vehicles initialized!!!" << endl;
}

//---------------------------------------------------------------------
//
// display
//
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	model_view = glm::mat4(1.0);
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//The 3D point in space that the camera is looking
	glm::vec3 look_at = cam_pos + looking_dir_vector;

	glm::mat4 camera_matrix = glm::lookAt(cam_pos, look_at, up_vector);
	glUniformMatrix4fv(cam_mat_location, 1, GL_FALSE, &camera_matrix[0][0]);

	glm::mat4 proj_matrix = glm::frustum(-0.01f, +0.01f, -0.01f, +0.01f, 0.01f, 100.0f);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &proj_matrix[0][0]);

	//Select the first texture (grass.png) when drawing the first geometry (floor)
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glDrawArrays(GL_QUADS, 0, 4);

	//Draw a column in the middle of the scene (0, 0, 0)
	//*****************************************************
	model_view = glm::scale(model_view, glm::vec3(0.1, 0.1, 10));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glDrawArrays(GL_QUADS, 4, 24);
	model_view = glm::mat4(1.0);
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	//*****************************************************

	renderVehicles();

	glFlush();
}


void keyboard(unsigned char key, int x, int y)
{
	if (key == 'a') {
		//Moving camera along opposit direction of side vector
		cam_pos += side_vector * travel_speed * deltaTime;
	}
	if (key == 'd') {
		//Moving camera along side vector
		cam_pos -= side_vector * travel_speed * deltaTime;
	}
	if (key == 'w') {
		//Moving camera along forward vector. To be more realistic, we use X=V.T equation in physics
		cam_pos += forward_vector * travel_speed * deltaTime;
	}
	if (key == 's') {
		//Moving camera along backward (negative forward) vector. To be more realistic, we use X=V.T equation in physics
		cam_pos -= forward_vector * travel_speed * deltaTime;
	}
	if (key == 'f') {
		shoot_dir = glm::vec2(forward_vector.x, forward_vector.y);
		shoot();
	}
}

//Controlling Pitch with vertical mouse movement
void mouse(int x, int y) {
	//Controlling Yaw with horizontal mouse movement
	int delta_x = x - x0;

	//The following vectors must get updated during a yaw movement
	forward_vector = glm::rotate(forward_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	looking_dir_vector = glm::rotate(looking_dir_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	side_vector = glm::rotate(side_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	up_vector = glm::rotate(up_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	x0 = x;

	//The following vectors must get updated during a pitch movement
	int delta_y = y - y_0;
	glm::vec3 tmp_up_vec = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
	glm::vec3 tmp_looking_dir = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);

	//The dot product is used to prevent the user from over-pitch (pitching 360 degrees)
	//The dot product is equal to cos(theta), where theta is the angle between looking_dir and forward vector
	GLfloat dot_product = glm::dot(tmp_looking_dir, forward_vector);

	//If the angle between looking_dir and forward vector is between (-90 and 90) degress 
	if (dot_product > 0) {
		up_vector = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
		looking_dir_vector = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);
	}
	y_0 = y;
}

void idle() {
	//Calculating the delta time between two frames
	//We will use this delta time when moving forward (in keyboard function)
	wheel_rotation += 0.01f;
	float timeSinceStart = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	deltaTime = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;
	//cout << timeSinceStart << " " << oldTimeSinceStart << " " << deltaTime << endl;
	glutPostRedisplay();
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("Camera and Projection");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();
	initialize_vehicles();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);

	glutIdleFunc(idle);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();



}
