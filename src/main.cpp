#include "gl_util.hpp"

/*
 ImGui
 */
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

using std::string;
using std::vector;
using glm::vec3;

/*
  Global variables below.
*/


struct Mesh {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<GLuint> faces;

    GLuint indexVbo;
    GLuint vertexVbo;
    GLuint normalVbo;
} mesh;

GLuint vao;

GpuProfiler* profiler;

const int WINDOW_WIDTH = 960;
const int WINDOW_HEIGHT = 650;
const int GUI_WIDTH = 250;

GLFWwindow* window;

float cameraYaw = 5.21f;
float cameraPitch = 0.28f;
float cameraZoom = 2.8f;
glm::vec3 cameraPos;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

GLuint tessShader;
GLuint normalShader;

double prevMouseX = 0;
double prevMouseY = 0;

double curMouseX = 0;
double curMouseY = 0;

const int RENDER_SPECULAR = 0;
const int RENDER_PROCEDURAL_TEXTURE = 1;

/*
  These variables are manipulated by ImGui:
 */
int renderMode = RENDER_PROCEDURAL_TEXTURE;
bool useTess = false;
int tessLevel = 1;
bool drawWireframe = false;
bool doVertexCalculation = false;


/*
  Update view matrix according pitch and yaw. Is called every frame.
*/
void UpdateViewMatrix() {

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
    GL_C(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* mesh.faces.size(), mesh.faces.data(), GL_STATIC_DRAW));

    GL_C(glGenBuffers(1, &mesh.vertexVbo));
    GL_C(glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexVbo));
    GL_C(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*mesh.vertices.size(), mesh.vertices.data() , GL_STATIC_DRAW));


    GL_C(glGenBuffers(1, &mesh.normalVbo));
    GL_C(glBindBuffer(GL_ARRAY_BUFFER, mesh.normalVbo));
    GL_C(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*mesh.normals.size(), mesh.normals.data() , GL_STATIC_DRAW));

    GL_C(glEnableVertexAttribArray(0));
    GL_C(glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexVbo));
    GL_C(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

    GL_C(glEnableVertexAttribArray(1));
    GL_C(glBindBuffer(GL_ARRAY_BUFFER, mesh.normalVbo));
    GL_C(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

}

void InitGlfw() {
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TESS", NULL, NULL);
    if (! window ) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
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

    // important that we do this, otherwise it won't work on retina!
    float ratio = fbWidth / (float)wWidth; //
    int s = ratio * GUI_WIDTH;

    // a tiny left part of the window is dedicated to GUI. So shift the viewport to the right some.
    GL_C(glViewport(s, 0, fbWidth-s, fbHeight));
    GL_C(glClearColor(0.0f, 0.0f, 0.3f, 1.0f));
    GL_C(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // update matrices.
    UpdateViewMatrix();
    glm::mat4 MVP = projectionMatrix * viewMatrix;

    GLuint shader;
    if(useTess) {
	shader = tessShader;
    } else {
	shader = normalShader;
    }
    GL_C(glUseProgram(shader));



    //
    // Set uniforms
    //
    GL_C(glUniformMatrix4fv(glGetUniformLocation(shader, "uMvp"), 1, GL_FALSE, glm::value_ptr(MVP) ));
    GL_C(glUniformMatrix4fv(glGetUniformLocation(shader, "uView"),1, GL_FALSE,  glm::value_ptr(viewMatrix)  ));
    GL_C(glUniform1i(glGetUniformLocation(shader, "uDrawWireframe"), drawWireframe ? 1 : 0  ));
    GL_C(glUniform1i(glGetUniformLocation(shader, "uRenderSpecular"), renderMode==RENDER_SPECULAR ? 1 : 0  ));

    if(useTess) {
	GL_C(glUniform1f(glGetUniformLocation(shader, "uTessLevel"), (float)tessLevel  ));
    } else {
	GL_C(glUniform1i(glGetUniformLocation(shader, "uDoVertexCalculation"),  doVertexCalculation ? 1 : 0 ));
    }



    if(drawWireframe)
	GL_C(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
    else
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    profiler->Begin();

    GL_C(glDrawElements(
	     useTess ?  GL_PATCHES: GL_TRIANGLES,

	     mesh.faces.size() , GL_UNSIGNED_INT, 0));

    profiler->End();

    // no wireframe for rendering  ImGui.
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // render GUI
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
	    } else {

		ImGui::Checkbox("Do Vertex Calculation", &doVertexCalculation);

	    }

	    ImGui::Text("Render Mode");
	    ImGui::RadioButton("Specular", &renderMode, RENDER_SPECULAR);
	    ImGui::RadioButton("Procedural Texture", &renderMode, RENDER_PROCEDURAL_TEXTURE);



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

	// zoom with mouse-wheel.
	cameraZoom += GetMouseWheel();

	prevMouseX = curMouseX;
	prevMouseY = curMouseY;
	glfwGetCursorPos(window, &curMouseX, &curMouseY);

	const float MOUSE_SENSITIVITY = 0.005;

	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

	// we change yaw and pitch by dragging with the mouse.
	if (state == GLFW_PRESS) {
	    cameraYaw += (curMouseX - prevMouseX ) * MOUSE_SENSITIVITY;
	    cameraPitch += (curMouseY - prevMouseY ) * MOUSE_SENSITIVITY;
	}
    }
}

int main(int argc, char** argv)
{

    InitGlfw();

    // init ImGui
    ImGui_ImplGlfwGL3_Init(window, true);

    normalShader =  LoadNormalShader(LoadFile("simple.vs") ,
				     LoadFile("simple.fs"));

    tessShader =  LoadTessShader(
	LoadFile("tess.vs"),
	LoadFile("tess.fs"),
	LoadFile("tess.tcs"),
	LoadFile("tess.tes")
	);

    // our patches are simply triangles in our case.
    GL_C(glPatchParameteri(GL_PATCH_VERTICES, 3));

    // setup projection matrix.
    projectionMatrix = glm::perspective(0.9f, (float)(WINDOW_WIDTH-GUI_WIDTH) / WINDOW_HEIGHT, 0.1f, 1000.0f);

    LoadModel();

    profiler = new GpuProfiler;

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

	Render();

	HandleInput();

	// set window title.
	string windowTitle =  "Teapot render time: " + std::to_string(profiler->GetAverageTime());
	glfwSetWindowTitle(window, windowTitle.c_str());

        /* display and process events through callbacks */
        glfwSwapBuffers(window);

	// update profiler.
	profiler->EndFrame();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
// screencapture -R410,70,710,650 -T 2 out4.png
