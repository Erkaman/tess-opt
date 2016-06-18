#include <stdio.h>

#include "gl_util.hpp"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

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
const int GUI_WIDTH = 200;

GLFWwindow* window;

float cameraYaw = 4.2f;
float cameraPitch = 0.5f;
float cameraZoom = 3.0;

glm::vec3 cameraPos;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

// use tesselation
bool useTess = true;
int tessLevel = 1;
bool drawWireframe = false;

GLuint tessShader;
GLuint normalShader;


double prevMouseX = 0;
double prevMouseY = 0;

double curMouseX = 0;
double curMouseY = 0;


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

    glfwMakeContextCurrent(window);

    // load GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Bind and create VAO, otherwise, we can't do anything in OpenGL.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GL_C(glEnable(GL_CULL_FACE));
    GL_C(glEnable(GL_DEPTH_TEST));

}

void Render() {
    int fbWidth, fbHeight;
    int wWidth, wHeight;


    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glfwGetWindowSize(window, &wWidth, &wHeight);

//    printf("ratio: %f\n",   );

    float ratio = fbWidth / (float)wWidth;

    int s = ratio * GUI_WIDTH;


    GL_C(glViewport(s, 0, fbWidth-s, fbHeight));
    GL_C(glClearColor(0.0f, 0.0f, 0.3f, 1.0f));
    GL_C(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    if(useTess)
	GL_C(glUseProgram(tessShader));
    else
	GL_C(glUseProgram(normalShader));


    updateViewMatrix();

    glm::mat4 MVP = projectionMatrix * viewMatrix;

    GLuint shader;
    if(useTess) {
	shader = tessShader;
    } else {
	shader = normalShader;
    }


    GL_C(glUniformMatrix4fv(glGetUniformLocation(shader, "uMvp"), 1, GL_FALSE, glm::value_ptr(MVP) ));
    GL_C(glUniformMatrix4fv(glGetUniformLocation(shader, "uView"),1, GL_FALSE,  glm::value_ptr(viewMatrix)  ));

    if(useTess) {
	GL_C(glUniform1f(glGetUniformLocation(shader, "uTessLevel"), (float)tessLevel  ));
    }
    GL_C(glUniform1i(glGetUniformLocation(shader, "uDrawWireframe"), drawWireframe ? 1 : 0  ));


    if(drawWireframe)
	GL_C(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
    else
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    GL_C(glDrawElements(
	     useTess ?  GL_PATCHES: GL_TRIANGLES,

	     mesh.faces.size() , GL_UNSIGNED_INT, 0));

    // no wireframe for  ImGui.
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    {
     	ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH,WINDOW_HEIGHT));
	ImGui::SetNextWindowPos(ImVec2(0, 0));


	ImGui::PushStyleColor(  ImGuiCol_WindowBg,  ImVec4(0.0, 0.0, 0.0, 1.0) ); // make non-transparent window.
	ImGui::Begin("Another Window", NULL,
		     ImGuiWindowFlags_NoResize |
		     ImGuiWindowFlags_NoMove |
		     ImGuiWindowFlags_NoCollapse);
	{


	    static float f = 0.0f;
	    ImGui::Text("Gui");

	    ImGui::Checkbox("Wireframe", &drawWireframe);

	    ImGui::Checkbox("Use Tessellation", &useTess);

	    if(useTess) {
		ImGui::SliderInt("TessLevel", &tessLevel, 1, 5);

	    }



	}
	ImGui::End();
	ImGui::PopStyleColor( );
    }
    ImGui::Render();


}

void HandleInput() {
    ImGuiIO& io = ImGui::GetIO();

    if(io.KeysDown[GLFW_KEY_ESCAPE]) {
	glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if(!io.WantCaptureMouse) { // if not interacting with ImGui, we handle our own input.

	cameraZoom += GetMouseWheel();


	prevMouseX = curMouseX;
	prevMouseY = curMouseY;
	glfwGetCursorPos(window, &curMouseX, &curMouseY);

	const float MOUSE_SENSITIVITY = 0.005;

	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS) {

	    cameraYaw += (curMouseX - prevMouseX ) * MOUSE_SENSITIVITY;
	    cameraPitch += (curMouseY - prevMouseY ) * MOUSE_SENSITIVITY;
	}
    }
}

int main(int argc, char** argv)
{
    InitGlfw();
    ImGui_ImplGlfwGL3_Init(window, true);

    normalShader =  LoadNormalShader(LoadFile("simple.vs") ,
				     LoadFile("simple.fs"));

    printf("LOAD\n");

    tessShader =  LoadTessShader(
	LoadFile("tess.vs"),
	LoadFile("tess.fs"),
	LoadFile("tess.tcs"),
	LoadFile("tess.tes")
	);


    glPatchParameteri(GL_PATCH_VERTICES, 3);


    // projection matrix.
    projectionMatrix = glm::perspective(0.9f, (float)(WINDOW_WIDTH-GUI_WIDTH) / WINDOW_HEIGHT, 0.1f, 1000.0f);



    LoadModel();



    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();




	Render();

	HandleInput();



        /* display and process events through callbacks */
        glfwSwapBuffers(window);

    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
