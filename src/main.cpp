#include <stdio.h>

#include "gl_util.hpp"


/*
  GLM
*/
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*
  TINY OBJ
*/
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

#ifndef M_PI
#define M_PI 3.14159
#endif


using std::string;
using std::vector;
using glm::vec3;

struct Mesh {
    std::vector<float> vertices;
    std::vector<float> normals;

    std::vector<GLuint> faces;

    GLuint indexVbo;
    GLuint vertexVbo;
    GLuint normalVbo;
} mesh;

/*
Global variables.
 */
GLuint vao;

const int WINDOW_WIDTH = 960;
const int WINDOW_HEIGHT = 650;

GLFWwindow* window;

float cameraYaw = 4.2f;
float cameraPitch = 0.5f;
float cameraZoom = 7.0;

glm::vec3 cameraPos;
glm::mat4 viewMatrix;


/*
  Update view matrix according pitch and yaw. Is called every frame.
 */
void updateViewMatrix() {

    glm::mat4 cameraTransform;

    cameraTransform = glm::rotate(cameraTransform, cameraYaw, glm::vec3(0.f, 1.f, 0.f)); // add yaw
    cameraTransform = glm::rotate(cameraTransform, cameraPitch, glm::vec3(0.f, 0.f, 1.f)); // add pitch

    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 center(0.0f, 0.0f, 0.0f);
    cameraPos = glm::vec3(cameraTransform * glm::vec4(cameraZoom, 0.0, 0.0, 1.0));

    viewMatrix = glm::lookAt(
	cameraPos,
	center,
	up
	);
}

void LoadModel(void) {

    //
    // First load the model.
    //

    std::string inputfile = "teapot.obj";
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    printf("Loading model: %s", inputfile.c_str() );
    bool ret = tinyobj::LoadObj(shapes, materials, err, inputfile.c_str());

    if (!err.empty()) {
	printf("%s\n", err.c_str() );
    }
    if (!ret) {
	exit(1);
    }

    mesh.vertices = shapes[0].mesh.positions;
    mesh.faces = shapes[0].mesh.indices;
    mesh.normals = shapes[0].mesh.normals;

    //
    // Then upload the model to OpenGL.
    //


    GL_C(glGenBuffers(1, &mesh.indexVbo));
    GL_C(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexVbo));
    GL_C(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* mesh.faces.size()*3, mesh.faces.data(), GL_STATIC_DRAW));


    // create

    GL_C(glGenBuffers(1, &mesh.vertexVbo));
    GL_C(glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexVbo));
    GL_C(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*mesh.vertices.size(), mesh.vertices.data() , GL_STATIC_DRAW));


    GL_C(glGenBuffers(1, &mesh.normalVbo));
    GL_C(glBindBuffer(GL_ARRAY_BUFFER, mesh.normalVbo));
    GL_C(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*mesh.normals.size(), mesh.normals.data() , GL_STATIC_DRAW));

    // enable
    GL_C(glEnableVertexAttribArray(0));
    GL_C(glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexVbo));
    GL_C(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

    GL_C(glEnableVertexAttribArray(1));
    GL_C(glBindBuffer(GL_ARRAY_BUFFER, mesh.normalVbo));
    GL_C(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

}

// GLFW scroll callback.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    cameraZoom += yoffset;
}

// GLFW key callback.
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    switch(key){
    case GLFW_KEY_ESCAPE:
	/* Exit program on Escape */
	glfwSetWindowShouldClose(window, GLFW_TRUE);
	break;
    }
}

void InitGlfw() {
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TESS", NULL, NULL);
    if (! window ) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetKeyCallback(window, KeyCallback);

    glfwMakeContextCurrent(window);

    // load GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Bind and create VAO, otherwise, we can't do anything in OpenGL.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}

int main(int argc, char** argv)
{

    InitGlfw();

    GLuint normalShader =  LoadNormalShader(LoadFile("simple.vs") ,
					    LoadFile("simple.fs"));

    printf("LOAD\n");

    GLuint tessShader =  LoadTessShader(
	LoadFile("tess.vs"),
	LoadFile("tess.fs"),
	LoadFile("tess.tcs"),
	LoadFile("tess.tes")
	);


	glPatchParameteri(GL_PATCH_VERTICES, 3);


    // projection matrix.
    glm::mat4 projectionMatrix = glm::perspective(0.9f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);



    LoadModel();



    double prevMouseX = 0;
    double prevMouseY = 0;

    double curMouseX = 0;
    double curMouseY = 0;

    GL_C(glEnable(GL_CULL_FACE));
    GL_C(glEnable(GL_DEPTH_TEST));

    bool isTess = false;

    while (!glfwWindowShouldClose(window)) {
	//  	GL_C(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));


	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	GL_C(glViewport(0, 0, fbWidth, fbHeight));
	GL_C(glClearColor(0.0f, 0.0f, 0.3f, 0.0f));
        GL_C(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	if(isTess)
	    GL_C(glUseProgram(tessShader));
	else
	    GL_C(glUseProgram(normalShader));


	updateViewMatrix();

	    glm::mat4 MVP = projectionMatrix * viewMatrix;

	if(isTess) {

	    GL_C(glUniformMatrix4fv(glGetUniformLocation(tessShader, "uMvp"), 1, GL_FALSE, glm::value_ptr(MVP) ));
	    GL_C(glUniformMatrix4fv(glGetUniformLocation(tessShader, "uView"),1, GL_FALSE,  glm::value_ptr(viewMatrix)  ));

	} else {

	    GL_C(glUniformMatrix4fv(glGetUniformLocation(normalShader, "uMvp"), 1, GL_FALSE, glm::value_ptr(MVP) ));
	    GL_C(glUniformMatrix4fv(glGetUniformLocation(normalShader, "uView"),1, GL_FALSE,  glm::value_ptr(viewMatrix)  ));



	}




	GL_C(glDrawElements(
		 isTess ?  GL_PATCHES: GL_TRIANGLES,

		 mesh.faces.size() , GL_UNSIGNED_INT, 0));


	prevMouseX = curMouseX;
	prevMouseY = curMouseY;
	glfwGetCursorPos(window, &curMouseX, &curMouseY);

	const float MOUSE_SENSITIVITY = 0.005;

	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS) {

	    cameraYaw += (curMouseX - prevMouseX ) * MOUSE_SENSITIVITY;
	    cameraPitch += (curMouseY - prevMouseY ) * MOUSE_SENSITIVITY;
	}

        /* display and process events through callbacks */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
