/*
Title: Pong Game
File Name: main.cpp
Copyright � 2015
Original authors: Srinivasan Thiagarajan
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
This example builds on the previous example "Circle-AABB 2D collision detection" to develop 
a small pong game. 

We use collision aresolution and decoupling algorithm for collisions between the ball and the paddle. 
For the boundaries, we use 3 giant sized blocks on the left, right and top. The collision with the 
blocks simply reverse the component perpendicular to the face. This is what happens when a object collides with an
immovable object. The same result can be obtained using the conventional formula when the object's 
mass is susbtituted with infinity i.e. inverse mass = 0.

The game has a difficulty mechanic which ups the speed after every 4 collisions.
The change in difficulty is reflected by a change in the colors. If the Ball falls
through, the difficulty is reset. 

Use mouse to move the paddle left or right.

References:
2D Game Collision Detection by Thomas Schwarzl
*/


#include "GLIncludes.h"

#define playArea 2.25f									// defines the play area. Increase the value to 2.5 for a bigger play area.

#pragma region program specific data
// We change this variable upon detecting collision
float blue = 0.5f;							// this variable is used to change color when difficulty is amped up.			
float timestep = 0.012f;					// Since we aren't using a accumulator to keep track of time, we are using a constant time step of 0.012 sec per frame. This value is provided by the checktime funciton. 
float maxVel = 0.01f;						// Velocity of the ball. (max velocity)
int difficultyCounter = 0;					// To keep track of the number of collision before the velocity is increased.
int attemtNo = 0;
#pragma endregion

// Global data members
#pragma region Base_data
// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniMVP;

// Reference to the window object being created by GLFW.
GLFWwindow* window;
#pragma endregion Base_data								  

//This struct consists of the basic stuff needed for getting the shape on the screen.
struct stuff_for_drawing{
	
	//This stores the address the buffer/memory in the GPU. It acts as a handle to access the buffer memory in GPU.
	GLuint vbo;

	//This will be used to tell the GPU, how many vertices will be needed to draw during drawcall.
	int numberOfVertices;

	//This function gets the number of vertices and all the vertex values and stores them in the buffer.
	void initBuffer(int numVertices, VertexFormat* vertices)
	{
		numberOfVertices = numVertices;

		// This generates buffer object names
		// The first parameter is the number of buffer objects, and the second parameter is a pointer to an array of buffer objects (yes, before this call, vbo was an empty variable)
		glGenBuffers(1, &vbo);
		
		//// Binds a named buffer object to the specified buffer binding point. Give it a target (GL_ARRAY_BUFFER) to determine where to bind the buffer.
		//// There are several different target parameters, GL_ARRAY_BUFFER is for vertex attributes, feel free to Google the others to find out what else there is.
		//// The second paramter is the buffer object reference. If no buffer object with the given name exists, it will create one.
		//// Buffer object names are unsigned integers (like vbo). Zero is a reserved value, and there is no default buffer for each target (targets, like GL_ARRAY_BUFFER).
		//// Passing in zero as the buffer name (second parameter) will result in unbinding any buffer bound to that target, and frees up the memory.
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		//// Creates and initializes a buffer object's data.
		//// First parameter is the target, second parameter is the size of the buffer, third parameter is a pointer to the data that will copied into the buffer, and fourth parameter is the 
		//// expected usage pattern of the data. Possible usage patterns: GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, 
		//// GL_DYNAMIC_READ, or GL_DYNAMIC_COPY
		//// Stream means that the data will be modified once, and used only a few times at most. Static means that the data will be modified once, and used a lot. Dynamic means that the data 
		//// will be modified repeatedly, and used a lot. Draw means that the data is modified by the application, and used as a source for GL drawing. Read means the data is modified by 
		//// reading data from GL, and used to return that data when queried by the application. Copy means that the data is modified by reading from the GL, and used as a source for drawing.
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * numVertices, vertices, GL_STATIC_DRAW);

		//// By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
		//// When enabled, the values in a generic vertex attribute array will be accessed and used for rendering when calls are made to vertex array commands (like glDrawArrays/glDrawElements)
		//// A GL_INVALID_VALUE will be generated if the index parameter is greater than or equal to GL_MAX_VERTEX_ATTRIBS
		glEnableVertexAttribArray(0);

		//// Defines an array of generic vertex attribute data. Takes an index, a size specifying the number of components (in this case, floats)(has a max of 4)
		//// The third parameter, type, can be GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, or GL_FLOAT
		//// The fourth parameter specifies whether to normalize fixed-point data values, the fifth parameter is the stride which is the offset (in bytes) between generic vertex attributes
		//// The fifth parameter is a pointer to the first component of the first generic vertex attribute in the array. If a named buffer object is bound to GL_ARRAY_BUFFER (and it is, in this case) 
		//// then the pointer parameter is treated as a byte offset into the buffer object's data.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
		//// You'll note sizeof(VertexFormat) is our stride, because each vertex contains data that adds up to that size.
		//// You'll also notice we offset this parameter by 16 bytes, this is because the vec3 position attribute is after the vec4 color attribute. A vec4 has 4 floats, each being 4 bytes 
		//// so we offset by 4*4=16 to make sure that our first attribute is actually the position. The reason we put position after color in the struct has to do with padding.
		//// For more info on padding, Google it.

		//// This is our color attribute, so the offset is 0, and the size is 4 since there are 4 floats for color.
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	}
};

// The basic structure for a Circle. We need a center, a radius, and VBO and total number of vertices.
struct Circle{
	glm::vec2 origin;
	float radius;
	glm::vec2 velocity;
	stuff_for_drawing base;
	int blue;
}circle,p1;

// the basic structure for a rectangle. We need a center, a length, a breadth and total number of vertices (6, 2 triangles).
struct Rectangle{
	glm::vec2 origin;
	float length;
	float breadth;
	stuff_for_drawing base;
}box1,box2,box3;

// This function return the value between min and mx with the least distance value to x. This is called clamping.
float clamp_on_range(float x, float min, float max)
{
	if (x < min)
		return min;
	if (x > max)
		return max;
	
	return x;
}

// This function clamps a vector on a rectangle. It basically finds the closest point on the rectangle to the point p.
glm::vec2 clamp_on_rectangle(glm::vec2 p, Rectangle r)
{
	glm::vec2 closest_point;
	closest_point.x = clamp_on_range(p.x,
		r.origin.x - (r.breadth / 2.0f),
		r.origin.x + (r.breadth / 2.0f));
	
	closest_point.y = clamp_on_range(p.y,
		r.origin.y - (r.length / 2.0f),
		r.origin.y + (r.length / 2.0f));

	return closest_point;
}

//This function checks if the circle is colliding with the rectangle.
bool is_colliding(Circle c, Rectangle r)
{
	//Gets the closest point on the box to the circle's center.
	glm::vec2 closest_point = clamp_on_rectangle(c.origin, r);

	//gets the distance between the circle's center and the point on box.
	float distance = glm::distance(closest_point, c.origin);
	
	// if the point lies on/inside the circle; return true . else false.
	if (distance <= c.radius)
		return true;
	else
		return false;
}

//This function checks for collisions between the circles (paddle and ball). If collision is detected, then it is resolved.
void checkAndResolvePaddleCollision(Circle c)
{
	if (glm::distance(c.origin, circle.origin) > c.radius + circle.radius)
	{
		//No collision. bail out.
		return;
	}

	//increase the counter as a collision has been detected.
	difficultyCounter++;

	//Collision normal. Since both the objects are circles, the line joining their centers is the collision normal.
	glm::vec2 n = circle.origin - c.origin;
	
	n = glm::normalize(n);

	//decoupling the two obnjects, since the paddle is supposed to be immovable, we just have to make sure that the ball is at a distance (of Sum of their radiuses) from the center of the paddle.
	circle.origin = c.origin + (n * (circle.radius + c.radius));

	//Since the paddle is of infinite mass, only the component of velocity which is along the collision normal will be reversed. 
	n *= glm::dot(n, circle.velocity);
	circle.velocity -= (n*2.0f);					// This reverses the component of the velocity along n.
}

//This function sets up the two shapes we need for this example.
void setup()
{
	int divisions = 20;
	//circle creation
	circle.origin = glm::vec2(0.0f, 0.0f);
	circle.radius = 0.15f;
	circle.velocity = glm::vec2(0.0f, maxVel);
	std::vector<VertexFormat> vertices1,vertices2;

	VertexFormat center(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	float theta = 360.0f / 20.0f;

	//Circle vertex generation
	//In this example we are not implementing the proper the code for indices. We are just going to produce redundant information in the buffer.
	//since we are only having a small number of objects on the screen currently, this redundancy should not matter.
	for (int i = 0; i < divisions; i++)
	{
		//In every iteration, the center, the point at angle theta and at angle (theta+delta) are fed into the buffer.
		vertices1.push_back(center);
		vertices1.push_back(VertexFormat(glm::vec3(circle.radius * cos(glm::radians(i*theta)), circle.radius * sin(glm::radians(i*theta)), 0.0f), glm::vec4(0.7f, 0.20f, 0.0f, 1.0f)));
		vertices1.push_back(VertexFormat(glm::vec3(circle.radius * cos(glm::radians((i + 1)*theta)), circle.radius * sin(glm::radians((i + 1)*theta)), 0.0f), glm::vec4(0.7f, 0.20f, 0.0f, 1.0f)));
	}

	//Push the data to the CPU
	circle.base.initBuffer(divisions * 3, &vertices1[0]);

	//Player1 generation
	p1.origin = glm::vec2(0.0f, -1.0f);
	p1.radius = 0.2f;

	vertices1.clear();

	for (int i = 0; i < divisions; i++)
	{
		//In every iteration, the center, the point at angle theta and at angle (theta+delta) are fed into the buffer.
		vertices1.push_back(center);
		vertices1.push_back(VertexFormat(glm::vec3(p1.radius * cos(glm::radians(i*theta)), p1.radius * sin(glm::radians(i*theta)), 0.0f), glm::vec4(0.2f, 0.40f, 0.7f, 1.0f)));
		vertices1.push_back(VertexFormat(glm::vec3(p1.radius * cos(glm::radians((i + 1)*theta)), p1.radius * sin(glm::radians((i + 1)*theta)), 0.0f), glm::vec4(0.2f, 0.40f, 0.7f, 1.0f)));
	}
	
	//Push the data to CPU
	p1.base.initBuffer(divisions * 3, &vertices1[0]);

	// Rectangle creation
	box1.origin = glm::vec2(0.0f, 0.0f);
	box2.origin = glm::vec2(playArea, 0.0f);
	box3.origin = glm::vec2(0.0f, playArea);
	box1.breadth = 3.0f;
	box1.length = 3.0f;
	box2.breadth = box1.breadth;
	box2.length = box1.length;
	box3.breadth = box1.breadth;
	box3.length = box1.length;
	//Rectangle Vertex generation
	VertexFormat A = VertexFormat(
		glm::vec3(box1.origin.x - (box1.breadth / 2.0f),
		box1.origin.y - (box1.length / 2.0f),
		0.0f),
		glm::vec4(0.7f, 0.20f, 0.0f, 1.0f));


	VertexFormat B = VertexFormat(
		glm::vec3(box1.origin.x + (box1.breadth / 2.0f),
		box1.origin.y - (box1.length / 2.0f),
		0.0f),
		glm::vec4(0.7f, 0.20f, 0.0f, 1.0f));

	VertexFormat C = VertexFormat(
		glm::vec3(box1.origin.x + (box1.breadth / 2.0f),
		box1.origin.y + (box1.length / 2.0f),
		0.0f),
		glm::vec4(0.7f, 0.20f, 0.0f, 1.0f));

	VertexFormat D = VertexFormat(
		glm::vec3(box1.origin.x - (box1.breadth / 2.0f),
		box1.origin.y + (box1.length / 2.0f),
		0.0f),
		glm::vec4(0.7f, 0.20f, 0.0f, 1.0f));

	/*
		D----------------------------C
		|                            |
		|                            |
		|                            |
		|                            |
		A----------------------------B
	*/


	vertices2.push_back(A);
	vertices2.push_back(B);
	vertices2.push_back(C);

	vertices2.push_back(A);
	vertices2.push_back(C);
	vertices2.push_back(D);

	//Push the data to the CPU
	box1.base.initBuffer(6, &vertices2[0]);
	box2.base.initBuffer(6, &vertices2[0]);
	box3.base.initBuffer(6, &vertices2[0]);
	box1.origin = glm::vec2(-playArea, 0.0f);

}

glm::vec2 EulerIntegrator(glm::vec2 pos, float h, glm::vec2 &velocity)
{
	glm::vec2 P;

	//Calculate the displacement in that time step with the current velocity.
	P = pos + (h * velocity);

	//return the position P
	return P;
}

// Functions called only once every time the program is executed.
#pragma region Helper_functions
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	// We choose ifstream and std::ios::in because we are opening the file for input into our program.
	// If we were writing to the file, we would use ofstream and std::ios::out.
	std::ifstream file(fileName, std::ios::in);

	// This checks to make sure that we didn't encounter any errors when getting the file.
	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;

		// Return so we don't error out.
		return "";
	}

	// ifstream keeps an internal "get" position determining the location of the element to be read next
	// seekg allows you to modify this location, and tellg allows you to get this location
	// This location is stored as a streampos member type, and the parameters passed in must be of this type as well
	// seekg parameters are (offset, direction) or you can just use an absolute (position).
	// The offset parameter is of the type streamoff, and the direction is of the type seekdir (an enum which can be ios::beg, ios::cur, or ios::end referring to the beginning, 
	// current position, or end of the stream).
	file.seekg(0, std::ios::end);					// Moves the "get" position to the end of the file.
	shaderCode.resize((unsigned int)file.tellg());	// Resizes the shaderCode string to the size of the file being read, given that tellg will give the current "get" which is at the end of the file.
	file.seekg(0, std::ios::beg);					// Moves the "get" position to the start of the file.

	// File streams contain two member functions for reading and writing binary data (read, write). The read function belongs to ifstream, and the write function belongs to ofstream.
	// The parameters are (memoryBlock, size) where memoryBlock is of type char* and represents the address of an array of bytes are to be read from/written to.
	// The size parameter is an integer that determines the number of characters to be read/written from/to the memory block.
	file.read(&shaderCode[0], shaderCode.size());	// Reads from the file (starting at the "get" position which is currently at the start of the file) and writes that data to the beginning
	// of the shaderCode variable, up until the full size of shaderCode. This is done with binary data, which is why we must ensure that the sizes are all correct.

	file.close(); // Now that we're done, close the file and return the shaderCode.

	return shaderCode;
}

// This method will consolidate some of the shader code we've written to return a GLuint to the compiled shader.
// It only requires the shader source code and the shader type.
GLuint createShader(std::string sourceCode, GLenum shaderType)
{
	// glCreateShader, creates a shader given a type (such as GL_VERTEX_SHADER) and returns a GLuint reference to that shader.
	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = sourceCode.c_str(); // We establish a pointer to our shader code string
	const int shader_code_size = sourceCode.size();   // And we get the size of that string.

	// glShaderSource replaces the source code in a shader object
	// It takes the reference to the shader (a GLuint), a count of the number of elements in the string array (in case you're passing in multiple strings), a pointer to the string array 
	// that contains your source code, and a size variable determining the length of the array.
	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader); // This just compiles the shader, given the source code.

	GLint isCompiled = 0;

	// Check the compile status to see if the shader compiled correctly.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		char infolog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infolog);

		// Print the compile error.
		std::cout << "The shader failed to compile with the error:" << std::endl << infolog << std::endl;

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.

		// NOTE: I almost always put a break point here, so that instead of the program continuing with a deleted/failed shader, it stops and gives me a chance to look at what may 
		// have gone wrong. You can check the console output to see what the error was, and usually that will point you in the right direction.
	}

	return shader;
}

// Initialization code
void init()
{
	// Initializes the glew library
	glewInit();

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Read in the shader code from a file.
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");

	// createShader consolidates all of the shader compilation code
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	// A shader is a program that runs on your GPU instead of your CPU. In this sense, OpenGL refers to your groups of shaders as "programs".
	// Using glCreateProgram creates a shader program and returns a GLuint reference to it.
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);
	// End of shader and program creation

	// This gets us a reference to the uniform variable in the vertex shader, which is called "color".
	// We're using this variable to change color during runtime, without changing the buffer values.
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	uniMVP = glGetUniformLocation(program, "colorAndPos");

	// This is not necessary, but I prefer to handle my vertices in the clockwise order. glFrontFace defines which face of the triangles you're drawing is the front.
	// Essentially, if you draw your vertices in counter-clockwise order, by default (in OpenGL) the front face will be facing you/the screen. If you draw them clockwise, the front face 
	// will face away from you. By passing in GL_CW to this function, we are saying the opposite, and now the front face will face you if you draw in the clockwise order.
	// If you don't use this, just reverse the order of the vertices in your array when you define them so that you draw the points in a counter-clockwise order.
	glFrontFace(GL_CCW);

	// This is also not necessary, but more efficient and is generally good practice. By default, OpenGL will render both sides of a triangle that you draw. By enabling GL_CULL_FACE, 
	// we are telling OpenGL to only render the front face. This means that if you rotated the triangle over the X-axis, you wouldn't see the other side of the triangle as it rotated.
	glEnable(GL_CULL_FACE);

	// Determines the interpretation of polygons for rasterization. The first parameter, face, determines which polygons the mode applies to.
	// The face can be either GL_FRONT, GL_BACK, or GL_FRONT_AND_BACK
	// The mode determines how the polygons will be rasterized. GL_POINT will draw points at each vertex, GL_LINE will draw lines between the vertices, and 
	// GL_FILL will fill the area inside those lines.
	glPolygonMode(GL_FRONT, GL_FILL);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions
// This runs once every physics timestep.
void update()
{
	if (difficultyCounter%4 == 0)
	{
		difficultyCounter += 1;
		//Amp up the difficulty. 
		blue *= -1.0f;		// change the color
		circle.velocity *= 1.25f;	
	}

	if (circle.origin.y < -2.0f)
	{
		//If the ball falls through i.e. its y-coordinate goes below -2. 
		std::cout << "\n Try " << 1 + attemtNo++ <<" : "<< difficultyCounter;
		difficultyCounter = 0;	// reset the difficulty counter
		//reset the velocity and position of the ball.
		circle.origin = glm::vec2(0);
		circle.velocity = glm::vec2(0, maxVel);
	}

	//Check for collision with two boxes on the left and right.
	if (is_colliding(circle, box1) || is_colliding(circle, box2))
	{
		//If collision is detected, then reverse the horizontal component of the velocity
		circle.velocity.x *= -1.0f;
	}
	
	//Check for collision with the box on the top.
	if (is_colliding(circle, box3))
	{
		//If collision is detected, then reverse the vertical component of the velocity.
		circle.velocity.y *= -1.0f;
	}
	
	//Check for collision with paddle.
	checkAndResolvePaddleCollision(p1);

	//Integrate the position of the ball. Since the ball motion is mostly linear, an euler integrator is sufficient. 
	circle.origin = EulerIntegrator(circle.origin, timestep, circle.velocity);

	// Get the cursor position with respect ot the window.
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	//Since the cursor posiiton is represented in pixel values, 
	//dividing it by the window specs will reduce the position to 0-1 range.
	//multipling it with 2 and then subtracting it with 1 wil bring the 
	//mouse position to -1 ot 1.

	p1.origin.x = ((x / 800.0f)*2.0f) - 1.0f;
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(blue, blue, blue, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	// Set the uniform matrix in our shader to our MVP matrix for the first object.
	glm::vec3 colorAndPos = glm::vec3(0.0f, 0.0f, 0.0f);
	
	/*
	we are using a vec3 to store 2 variables. The first 2 are used to store the x and y transformation.
	the last float is used to store the blue  color component. We are doing this so, there are less calls to be made to the GPU
	*/

	// Draw the Circle
	glUniform3f(uniMVP, circle.origin.x, circle.origin.y, blue);
	glBindBuffer(GL_ARRAY_BUFFER, circle.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, circle.base.numberOfVertices);

	//render the paddle.
	glUniform3f(uniMVP, p1.origin.x, p1.origin.y, blue);
	glBindBuffer(GL_ARRAY_BUFFER, p1.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, p1.base.numberOfVertices);

	
	//Draw the rectangle1
	glUniform3f(uniMVP, box1.origin.x, box1.origin.y,blue);
	glBindBuffer(GL_ARRAY_BUFFER, box1.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, box1.base.numberOfVertices);

	//Draw the rectangle2
	glUniform3f(uniMVP, box2.origin.x, box2.origin.y,blue);
	glBindBuffer(GL_ARRAY_BUFFER, box2.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, box2.base.numberOfVertices);

	//Draw the rectangle3
	glUniform3f(uniMVP, box3.origin.x, box3.origin.y, blue);
	glBindBuffer(GL_ARRAY_BUFFER, box3.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, box3.base.numberOfVertices);

}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	float moverate = 0.1f;

	//This set of controls are used to move the position of the circle.
	if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
		p1.origin.x -= moverate;
	if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
		p1.origin.x += moverate;

}

#pragma endregion


void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "Pong", nullptr, nullptr);

	std::cout << "This builds upon the previous examples to create a pong game.\n\n\n\n";
	std::cout << "Use mouse to control the paddle position";

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS. Otherwise we'd be consistently getting 60FPS or lower, 
	// since it would match our FPS to the screen refresh rate.
	// Set to 1 to enable VSync.
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);

	setup();

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
		update();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		// Remember, you're rendering to the back buffer, then once rendering is complete, you're moving the back buffer to the front so it can be displayed.
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.


	// Frees up GLFW memory
	glfwTerminate();
}