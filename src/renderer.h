#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "scene.h"
#include "shader.h"

#include "renderpass.h"
#include "preprocesspass.h"
#include "postprocesspass.h"

#include "gbuffer.h"
#include "deferredlighting.h"
#include "hdrpass.h"

#include "ssaopass.h"
#include "ssrpass.h"
#include "bloompass.h"
#include "skybox.h"

class Renderer {
private:
	enum LoadSuccess {
		waiting, failed, successful
	};

	// Render passes
	std::shared_ptr<GBufferPass> gBufferPass;
	std::shared_ptr<DeferredLightingPass> lightingPass;
	std::shared_ptr<HDRPass> hdrPass;
	
	// Optional passes
	std::shared_ptr<SSAOPass> ssao;
	std::shared_ptr<SSRPass> ssr;
	std::shared_ptr<BloomPass> bloom;
	std::shared_ptr<SkyboxPass> skybox;

	// skybox UI info
	char skyboxDir[128] = "textures/skybox";
	char skyboxImgExtension[4] = "jpg";
	LoadSuccess skyboxLoadSuccess = waiting;

	// Matrix ubo
	unsigned int uboMatrix;

	void initMatrices();
	void updateMatrices();
	void updateProjectionMatrix();
	void updateViewMatrix();

	// Light ubo
	unsigned int uboLights;
	void initLights();
	void updateLights();

public:
	unsigned int TARGET_WIDTH, TARGET_HEIGHT;

	std::shared_ptr<Scene> scene;
	std::shared_ptr<Camera> camera;

	// UI settings
	// Preprocess
	bool ssaoOn, ssrOn;
	// Postprocess
	bool bloomOn;
	// Skybox
	bool skyboxOn;

	Renderer();
	Renderer(unsigned int width, unsigned int height, std::shared_ptr<Scene> scene);

	void render();
	void renderUI();
	void framebuffer_size_callback(int width, int height);

	void toggleSSAO();
	void toggleSSR();
	void toggleBloom();
	void toggleSkybox();
};

#endif