#include<random>

#include <imgui/imgui.h>
#include "renderer.h"

Renderer::Renderer()
{
	Renderer(800, 600, std::make_shared<Scene>());
}

Renderer::Renderer(unsigned int width, unsigned int height, std::shared_ptr<Scene> scene)
{
	TARGET_WIDTH = width;
	TARGET_HEIGHT = height;
	this->scene = scene;

	camera = std::make_unique<Camera>(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f, width / (float)height, Camera::ProjectionType::Perspective);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	initMatrices();
	initLights();

	gBufferPass = std::make_shared<GBufferPass>(width, height, scene);
	lightingPass = std::make_shared<DeferredLightingPass>(width, height, gBufferPass);
	hdrPass = std::make_shared<HDRPass>(width, height, lightingPass);
}

void Renderer::render()
{
	updateMatrices();
	updateLights();

	gBufferPass->Render();
	for (std::shared_ptr<PreprocessPass> p : lightingPass->preprocessPasses) p->Render();
	lightingPass->Render();
	for (std::shared_ptr<PostprocessPass> p : hdrPass->postprocessPasses) p->Render();
	hdrPass->Render();
}

void Renderer::renderUI()
{
	ImGui::Begin("Renderer options");

	if (ImGui::IsItemActive()) { skyboxLoadSuccess = waiting; }

	ImGui::SeparatorText("Preprocess effects");
	if (ImGui::Checkbox("SSAO", &ssaoOn)) {
		toggleSSAO();
	}
	if (ImGui::Checkbox("SSR", &ssrOn)) {
		toggleSSR();
	}


	ImGui::SeparatorText("Postprocess effects");
	if (ImGui::Checkbox("Bloom", &bloomOn)) {
		toggleBloom();
	}

	ImGui::InputText("Skybox directory", skyboxDir, 128);
	ImGui::InputText("Extension", skyboxImgExtension, 4);
	if (ImGui::Checkbox("Skybox", &skyboxOn)) {
		toggleSkybox();
	}
	if (skyboxLoadSuccess == LoadSuccess::failed)
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Loading failed");

	if (ImGui::Combo("HDR Mode", &hdrPass->hdrMode, hdrPass->hdrModes, IM_ARRAYSIZE(hdrPass->hdrModes))) {
		hdrPass->updateHDRMode();
	}

	if (hdrPass->hdrMode == HDRPass::HDRmode::exposure) {
		if (ImGui::DragFloat("Exposure", &hdrPass->exposureVal, 0.001f, 0.0f, 100.0f))
			hdrPass->updateExposure();
	}

	ImGui::End();
}

void Renderer::initMatrices()
{
	glGenBuffers(1, &uboMatrix);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrix);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrix);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Camera::Matrices), &camera->matrices, GL_STATIC_DRAW);
}

void Renderer::updateMatrices()
{
	if (camera->projectionIsDirty) {
		updateProjectionMatrix();
		camera->projectionIsDirty = false;
	}
	if (camera->viewIsDirty) {
		updateViewMatrix();
		camera->viewIsDirty = false;
	}
}

void Renderer::updateProjectionMatrix()
{
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrix);
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Camera::Matrices, Camera::Matrices::projection), sizeof(glm::mat4), &camera->matrices.projection);
}

void Renderer::updateViewMatrix()
{
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrix);
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Camera::Matrices, Camera::Matrices::view), sizeof(glm::mat4), &camera->matrices.view);
}

void Renderer::initLights()
{
	glGenBuffers(1, &uboLights);
	glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboLights);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneLights), &scene->lights, GL_STATIC_DRAW);
}

void Renderer::updateLights() {
	glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneLights), &scene->lights, GL_STATIC_DRAW);
}

void Renderer::framebuffer_size_callback(int width, int height)
{
	TARGET_WIDTH = width;
	TARGET_HEIGHT = height;

	gBufferPass->ResizeBuffers(width, height);
	for (std::shared_ptr<PreprocessPass> p : lightingPass->preprocessPasses) p->ResizeBuffers(width, height);
	lightingPass->ResizeBuffers(width, height);
	for (std::shared_ptr<PostprocessPass> p : hdrPass->postprocessPasses) p->ResizeBuffers(width, height);
	hdrPass->ResizeBuffers(width, height);

	camera->framebuffer_size_callback(width, height);
}

void Renderer::toggleSSAO()
{
	if (ssaoOn) {
		ssao = std::make_shared<SSAOPass>(TARGET_WIDTH, TARGET_HEIGHT, gBufferPass);
		lightingPass->AddPreprocessPass(ssao);
	}
	else {
		lightingPass->RemovePreprocessPass(ssao);
		ssao.reset();
	}
}

void Renderer::toggleSSR()
{
	if (ssrOn) {
		ssr = std::make_shared<SSRPass>(TARGET_WIDTH, TARGET_HEIGHT, gBufferPass);
		lightingPass->AddPreprocessPass(ssr);
	}
	else {
		lightingPass->RemovePreprocessPass(ssr);
		ssr.reset();
	}
}

void Renderer::toggleBloom()
{
	if (bloomOn) {
		bloom = std::make_shared<BloomPass>(TARGET_WIDTH, TARGET_HEIGHT, lightingPass);
		hdrPass->AddPostprocessPass(bloom);
	}
	else {
		hdrPass->RemovePostprocessPass(bloom);
		bloom.reset();
	}
}

void Renderer::toggleSkybox()
{
	if (skyboxOn) {
		std::string faces[6];
		for (int i = 0; i < 6; i++) {
			faces[i] = std::string(skyboxDir) + "/" + SkyboxPass::skybox_faces[i] + "." + std::string(skyboxImgExtension);
		}
		skybox = std::make_shared<SkyboxPass>(0, 0, lightingPass, faces);
		if (skybox->loadFailed) {
			skyboxLoadSuccess = LoadSuccess::failed;
			skyboxOn = false;
			skybox.reset();
		}
		else {
			skyboxLoadSuccess = LoadSuccess::successful;
			hdrPass->AddPostprocessPass(skybox);
		}
	}
	else {
		hdrPass->RemovePostprocessPass(skybox);
		skybox.reset();
	}
}
