#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <deque>

#include "renderer.h"
#include "model.h"
#include "mesh.h"
#include "material.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void glfw_onError(int error, const char* description);
void processInput(GLFWwindow* window);

// settings
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

// renderer
std::unique_ptr<Renderer> renderer;
std::shared_ptr<Scene> scene;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
const int nFramerateCaptures = 256;
std::vector<float> framerates;
float sumFramerates = 0.0f;
float averageFrameRateWindow = 0.0f;
float maxFramerateWindow = 0.0f;
int stepsSinceMaxFramerate = 0;
float minFramerateWindow = 0.0f;
int stepsSinceMinFramerate = 0;

std::map<std::string, bool> prepassEnabled;
std::map<std::string, bool> postpassEnabled;

ImGuiIO* io;

int main() {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "arrend", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetErrorCallback(glfw_onError);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	framerates.reserve(nFramerateCaptures);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	scene = std::make_shared<Scene>();

	PointLight light1;
	light1.pos = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
	light1.color = glm::vec4(2.0f);
	light1.Linear = 0.35f;
	light1.Quadratic = 0.44f;
	scene->lights.pointLights[0] = light1;

	DirLight light2;
	light2.dir = glm::normalize(glm::vec4(-1.0f, -2.0f, 0.5f, 0.0f));
	light2.color = glm::vec4(0.5f);
	scene->lights.dirLights[0] = light2;

	SpotLight light3;
	light3.pos = glm::vec4(0.0f, 5.0f, 0.0f, 1.0f);
	light3.dir = glm::normalize(glm::vec4(1.0f, -1.0f, -1.0f, 0.0f));
	light3.color = glm::vec4(2.0f);
	light3.outerCutOff = glm::cos(glm::radians(30.0f));
	light3.cutOff = glm::cos(glm::radians(20.0f));
	light3.Linear = 0.0f;
	light3.Quadratic = 0.0f;
	scene->lights.spotLights[0] = light3;

	renderer = std::make_unique<Renderer>(SCR_WIDTH, SCR_HEIGHT, scene);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");

	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		// --------------------
		float currentTime = static_cast<float>(glfwGetTime());
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		// framerate calculations for display
		// ----------------------------------
		float framerate = 1.0f / deltaTime;
		framerates.push_back(framerate);
		sumFramerates += framerate;
		if (framerates.size() > nFramerateCaptures) {
			sumFramerates -= framerates[0];
			framerates.erase(framerates.begin());
		}
		averageFrameRateWindow = sumFramerates / framerates.size();

		if (framerate > maxFramerateWindow) {
			maxFramerateWindow = framerate;
			stepsSinceMaxFramerate = 0;
		}
		if (stepsSinceMaxFramerate > framerates.size() - 1) {
			int maxFramerateIndex = distance(framerates.begin(), std::max_element(framerates.begin(), framerates.end()));
			maxFramerateWindow = framerates[maxFramerateIndex];
			stepsSinceMaxFramerate = framerates.size() - 1 - maxFramerateIndex;
		}
		stepsSinceMaxFramerate++;

		if (framerate < minFramerateWindow) {
			minFramerateWindow = framerate;
			stepsSinceMinFramerate = 0;
		}
		if (stepsSinceMinFramerate > framerates.size() - 1) {
			int minFramerateIndex = distance(framerates.begin(), std::min_element(framerates.begin(), framerates.end()));
			minFramerateWindow = framerates[minFramerateIndex];
			stepsSinceMinFramerate = framerates.size() - 1 - minFramerateIndex;
		}
		stepsSinceMinFramerate++;

		// input
		// -----
		processInput(window);

		// render
		// ------
		renderer->render();

		// ui
		// --
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		renderer->renderUI();
		scene->renderUI();
		Model::renderLoadInfoUI();

		ImGui::Begin("Performance");
		ImGui::Text("Frame time: %f ms, Frame rate: %.3f FPS", deltaTime * 1000.0f, framerates[framerates.size() - 1]);
		ImGui::PlotLines("##framerate", &framerates[0], framerates.size(), 0, NULL, 0.0f, maxFramerateWindow, ImVec2(300, 100));
		ImGui::SameLine();
		ImGui::Text("Max FPS: %f\n\n\nAverage FPS: %.1f\n\n\nMin FPS: %f", maxFramerateWindow, averageFrameRateWindow, minFramerateWindow);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	renderer->framebuffer_size_callback(width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (!io->WantCaptureMouse && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		renderer->camera->ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!io->WantCaptureMouse)
		renderer->camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

void glfw_onError(int error, const char* description)
{
	std::cout << "Error " << error << ": " << description;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (!io->WantCaptureKeyboard) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			renderer->camera->ProcessKeyboard(Camera::FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			renderer->camera->ProcessKeyboard(Camera::BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			renderer->camera->ProcessKeyboard(Camera::LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			renderer->camera->ProcessKeyboard(Camera::RIGHT, deltaTime);
	}

}