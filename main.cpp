#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stb_image.h>

#include <fstream>
#include <sstream>
#include <string>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp>
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "FrameBuffer.h"

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);

// screen sizes
const unsigned int SCREEN_WIDTH = 1000;
const unsigned int SCREEN_HEIGHT = 1000;
// camera
Camera camera(glm::vec3(1.0f, 5.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), 70.0f, -25.0f);
// Second camera used for reflection
Camera cameraReflection(glm::vec3(1.0f, 5.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), 70.0f, -25.0f);

//coords
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool first_mouse_flag = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// waves
float waveSpeed = 0.3f;
float waterOffset = 0.5f;

// projection matrix
mat4 projectionMatrix;

// cursor state, change with space
bool cursorHidden = false;

// Ray State
bool ray_flag = false;
vec3 ray;

float INTERSECT_EPSILON = 0.5f;


//TODO
// Intersection Triangle Verticies
vec3 vertice1;
vec3 vertice2;
vec3 vertice3;
vec3 vertice4;
vec3 intersection = vec3(-15.0f, 0, 54.0f);

// Surface
struct Surface {
	GLfloat* coordinates; // vertex information.
	int size; // generated coordinates array size.
	int* indexBuffer; // triangle strips
	int indexCount;

	void print() { cout << "coordinates " << coordinates << " coordinate_size " << size; };
};

// number of vertices needed
int getVerticesCount(int hVertices, int vVertices) {
	return hVertices * vVertices * 5;
}

// number of indices needed
int getIndicesCount(int hVertices, int vVertices) {
	int numStripsRequired = vVertices - 1;
	int numDegensRequired = 2 * (numStripsRequired - 1);
	int verticesPerStrip = 2 * hVertices;

	return (verticesPerStrip * numStripsRequired) + numDegensRequired;
}

// Generate coordinates for vertices
GLfloat* generateVerticies(int hVertices, int vVertices, int size, int verticeCount) {
	GLfloat* surfaceVertices = new GLfloat[verticeCount];

	GLfloat cellSize = (float)(size) / (float)(hVertices);
	int verticeIndex = 0;

	// Looping over all points and generating coordinates
	for (int col = vVertices - 1; col >= 0; col--) {
		float ratioC = (float)col / (float)vVertices;
		for (int row = 0; row < hVertices; row++) {
			float ratioR = (float)row / (float)(hVertices - 1);
			surfaceVertices[verticeIndex] = (float)(ratioR * size);
			surfaceVertices[verticeIndex + 2] = (float)(ratioC * size);
			surfaceVertices[verticeIndex + 1] = 0.0f;
			surfaceVertices[verticeIndex + 3] = 0.0f + (float)(cellSize * row) / size;
			surfaceVertices[verticeIndex + 4] = 0.0f + (float)(cellSize * col) / size;

			if (row == 0 && col == vVertices - 1) {
				vertice1 = vec3((float)(ratioR * size), 0.0f, (float)(ratioC * size));
			}
			else if (row == hVertices - 1 && col == vVertices - 1) {
				vertice2 = vec3((float)(ratioR * size), 0.0f, (float)(ratioC * size));
			}
			else if (col == 0 && row == 0) {
				vertice3 = vec3((float)(ratioR * size), 0.0f, (float)(ratioC * size));
			}
			else if (col == 0 && row == hVertices - 1) {
				vertice3 = vec3((float)(ratioR * size), 0.0f, (float)(ratioC * size));
			}

			verticeIndex += 5;
		}
	}

	return surfaceVertices;
}

// sequence of indices
int* getIndices(int width, int height, int indicesCount) {
	int* indices = new int[indicesCount];
	int offset = 0;

	for (int y = 0; y < height - 1; y++) {
		if (y > 0) {
			indices[offset++] = (int)(y * height);
		}

		for (int x = 0; x < width; x++) {
			indices[offset++] = (int)((y * height) + x);
			indices[offset++] = (int)(((y + 1) * height) + x);
		}

		if (y < height - 2) {
			indices[offset++] = (int)(((y + 1) * height) + (width - 1));
		}
	}
	return indices;
}

Surface GenerateIndexedTriangleStripPlane(int hVertices, int vVertices, float size) {
	// indices and vertices
	int verticeCount = getVerticesCount(hVertices, vVertices);
	int indicesCount = getIndicesCount(hVertices, vVertices);
	// generate vecs
	GLfloat* surfaceVerticies = generateVerticies(hVertices, vVertices, size, verticeCount);
	int* surfaceIndices = getIndices(hVertices, vVertices, indicesCount);

	Surface surfaceData = {};

	surfaceData.coordinates = surfaceVerticies;
	surfaceData.size = verticeCount;
	surfaceData.indexBuffer = surfaceIndices;
	surfaceData.indexCount = indicesCount;

	return surfaceData;
};

bool IntersectTriangle(vec3 dir, vec3 v0, vec3 v1, vec3 v2, vec3 orig) {
	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;
	vec3 N = cross(v0v2, v0v1);
	float denom = dot(N, N);

	float NdotRayDirection = dot(N, dir);
	if (fabs(NdotRayDirection) < INTERSECT_EPSILON) {
		return false;
	}

	float d = dot(N, v0);

	float t = -((dot(N, orig) - d) / NdotRayDirection);
	if (t < 0) {
		return false;
	}

	intersection = orig + t * dir;

    //inside-outside test
	vec3 C;

	vec3 edge0 = v1 - v0;
	vec3 vp0 = intersection - v0;
	C = cross( vp0, edge0);

	vec3 edge1 = v2 - v1;
	vec3 vp1 = intersection - v1;
	C = cross(vp1, edge1);

	vec3 edge2 = v0 - v2;
	vec3 vp2 = intersection - v2;
	C = cross(vp2, edge2);

	return true;
}

int main() {
	// Generate Ground Surface
	Surface ground = GenerateIndexedTriangleStripPlane(500, 500, 10);
	//ground.print();
	// Generate Water Surface
	Surface water = GenerateIndexedTriangleStripPlane(500, 500, 10);
	//water.print();

	/* GLFW start*/
	GLFWwindow* window;
	if (!glfwInit()) {
		return -1;
	}

	window = glfwCreateWindow(1000, 1000, "window", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	/* GLFW end*/

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	//face cool done in while too
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//blend and set alpha for grass transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// water to the scene
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, water.size * sizeof(GLfloat), water.coordinates, GL_STATIC_DRAW);

	// position attribute
	GLintptr vertex_position_offset = 0 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)vertex_position_offset);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, water.indexCount * sizeof(int), water.indexBuffer, GL_STATIC_DRAW);

	// ground to the scene
	unsigned int VBO_G, VAO_G;
	glGenVertexArrays(1, &VAO_G);
	glGenBuffers(1, &VBO_G);
	glBindVertexArray(VAO_G);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_G);
	glBufferData(GL_ARRAY_BUFFER, ground.size * sizeof(GLfloat), ground.coordinates, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)vertex_position_offset);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	GLuint index_buffer_g;
	glGenBuffers(1, &index_buffer_g);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_g);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ground.indexCount * sizeof(int), ground.indexBuffer, GL_STATIC_DRAW);

	// get textures and bind
	Texture texture = Texture("./Media/WaterDiffuse.png");
	Texture texture_dudv = Texture("./Media/dudv_map.png");
	Texture texture_ground = Texture("./Media/TerrainDiffuse.png");
	Texture texture_ground_map = Texture("./Media/TerrainHeightMap.png");
	Texture grass_dist("./Media/GrassDistribution.png");
	Texture grass_textexture("./Media/GrassDiffuse.png");

	Shader ground_program("./ground_vertex.shader",
						  "./ground_fragment.shader", 
						  "./ground_geometry.shader");

	ground_program.use();
	ground_program.setInt("textureMainGround", 0);
	ground_program.setInt("heightMap", 1);
	ground_program.setVec3("viewPos", camera.Position);

	Shader water_program("./water_vertex.shader", 
						 "./water_fragment.shader", 
						 "./water_geometry.shader");

    water_program.use();
	water_program.setInt("textureMain", 2);
	water_program.setInt("DudvMap", 3);
	water_program.setInt("Reflection", 4);
	water_program.setVec3("viewPos", camera.Position);

	Shader grass_program("./ground_vertex.shader",
						"./grass_fragment.shader",
						"./grass_geometry.shader");
	//grass_program.use() in while

	FrameBuffer reflection = FrameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		cameraReflection.Position = vec3(camera.Position[0], -camera.Position[1], camera.Position[2]);
		cameraReflection.Pitch = -camera.Pitch;
		cameraReflection.updateCameraVectors();

		//both start from same place
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		glm::mat4 projectionReflection = projection;
		projectionMatrix = projection;
		water_program.setMat4("projection", projection);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 viewReflection = cameraReflection.GetViewMatrix();

		water_program.setMat4("view", view);
		water_program.setFloat("time", currentFrame);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		reflection.Bind(SCREEN_WIDTH, SCREEN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAO_G);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_g);
		ground_program.use();
		
		ground_program.setMat4("projection", projection);
		ground_program.setMat4("view", viewReflection);

		texture_ground.Bind(GL_TEXTURE0);
		texture_ground_map.Bind(GL_TEXTURE1);

		glm::mat4 model = glm::mat4(1.0f);
		ground_program.setMat4("model", model);

		//draw ground
		glDrawElements(GL_TRIANGLE_STRIP, ground.indexCount, GL_UNSIGNED_INT, nullptr);
		glDisable(GL_CULL_FACE);

		//disable cull face to get reflection from opposite side
		grass_program.use();
		grass_program.setMat4("projection", projection);
		grass_program.setMat4("view", viewReflection);
		grass_program.setMat4("model", model);

		texture_ground_map.Bind(GL_TEXTURE0);
		grass_dist.Bind(GL_TEXTURE1);
		grass_textexture.Bind(GL_TEXTURE2);

		glUniform1i(glGetUniformLocation(grass_program.ID, "heightMap"), 0);
		glUniform1i(glGetUniformLocation(grass_program.ID, "grassDist"), 1);
		glUniform1i(glGetUniformLocation(grass_program.ID, "grassText"), 2);
		glDrawElements(GL_TRIANGLE_STRIP, ground.indexCount, GL_UNSIGNED_INT, nullptr);

		reflection.Unbind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAO_G);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_g);
		ground_program.use();

		ground_program.setMat4("projection", projection);
		ground_program.setMat4("view", view);

		texture_ground.Bind(GL_TEXTURE0);
		texture_ground_map.Bind(GL_TEXTURE1);

		model = glm::mat4(1.0f);
		ground_program.setMat4("model", model);

		glDrawElements(GL_TRIANGLE_STRIP, ground.indexCount, GL_UNSIGNED_INT, nullptr);

		GLuint unifr_loc = glGetUniformLocation(water_program.ID, "rippleCenter");
		glUniform3f(unifr_loc, intersection[0], intersection[1], intersection[2]);

		if (ray_flag) {
			cout << "Checking Ray" << endl;
			bool performWave = false;
			if (IntersectTriangle(ray, vertice1, vertice2, vertice3, camera.Position)) {//1st tri
				performWave = true;
				cout << "HIT" << endl;
				glfwSetTime(0);
			}
			else if (IntersectTriangle(ray, vertice3, vertice2, vertice4, camera.Position)) { // else 2nd one
				performWave = true;
				cout << "HIT" << endl;
				glfwSetTime(0);
			}
			water_program.setBool("performWave", performWave);
			water_program.setVec3("rippleCenter", intersection);
			ray_flag = false;
		}

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		water_program.use();

		waterOffset = currentFrame * waveSpeed;
		GLint myUniformLocation = glGetUniformLocation(water_program.ID, "offset");
		glUniform1f(myUniformLocation, waterOffset);

		if (waterOffset > 1) {
			waterOffset = 0.0f;
		}

		processInput(window);
		
		texture.Bind(GL_TEXTURE2);
		texture_dudv.Bind(GL_TEXTURE3);
		reflection.BindTexture(GL_TEXTURE4);
		
		model = glm::mat4(1.0f);
		water_program.setMat4("model", model);
		water_program.setMat4("view", view);
		water_program.setFloat("time", currentFrame);
		water_program.setMat4("projection", projection);

		glDrawElements(GL_TRIANGLE_STRIP, water.indexCount, GL_UNSIGNED_INT, nullptr);

		grass_program.use();
		grass_program.setMat4("projection", projection);
		grass_program.setMat4("view", view);
		grass_program.setMat4("model", model);

		texture_ground_map.Bind(GL_TEXTURE0);
		grass_dist.Bind(GL_TEXTURE1);
		grass_textexture.Bind(GL_TEXTURE2);

		glUniform1i(glGetUniformLocation(grass_program.ID, "heightMap"), 0);
		glUniform1i(glGetUniformLocation(grass_program.ID, "grassDist"), 1);
		glUniform1i(glGetUniformLocation(grass_program.ID, "grassText"), 2);
		glDrawElements(GL_TRIANGLE_STRIP, ground.indexCount, GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

vec2 getNormalizedCoordinates(vec2 pixel, int screenWidth, int screenHeight) {
	float x = (2.0f * pixel[0]) / screenWidth - 1;
	float y = (2.0f * pixel[1]) / screenHeight - 1;

	return vec2(x, -y);
};

vec4 convertToEyeSpace(vec4 clipSpace, mat4 projection) {
	mat4 invertedProjection = glm::inverse(projection);
	vec4 eyeCoordinates = invertedProjection * clipSpace;

	return vec4(eyeCoordinates[0], eyeCoordinates[1], -1.0f, 0.0f);
}

vec3 convertToWorldCoordinates(vec4 eyeCoordinates, mat4 viewMatrix) {
	mat4 invertedViewMatrix = glm::inverse(viewMatrix);
	vec4 ray = invertedViewMatrix * eyeCoordinates;

	vec3 result = vec3(ray[0], ray[1], ray[2]);
	result = glm::normalize(result);

	return result;
}



vec3 ConstructRayFromPixel(float fov, vec2 pixel) {
	vec2 normalized = getNormalizedCoordinates(pixel, SCREEN_WIDTH, SCREEN_HEIGHT);
	float mouseX = normalized[0];
	float mouseY = normalized[1];

	vec4 clippedCoordinates = vec4(mouseX, mouseY, -1.0f, 1.0f);
	vec4 eyeCoordinates = convertToEyeSpace(clippedCoordinates, projectionMatrix);
	vec3 ray = convertToWorldCoordinates(eyeCoordinates, camera.GetViewMatrix());

	//cout << ray[0]<< " x " << ray[1] << " y " << ray[2] << endl;

	ray_flag = true;
	return ray;
};


void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (cursorHidden) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		cursorHidden = !cursorHidden;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
		cameraReflection.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		cameraReflection.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
		cameraReflection.ProcessKeyboard(LEFT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
		cameraReflection.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (first_mouse_flag) {
		lastX = xpos;
		lastY = ypos;
		first_mouse_flag = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
	cameraReflection.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		ray = ConstructRayFromPixel(12.0f, vec2(xpos, ypos));
	}
}