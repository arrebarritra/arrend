#include "lightcube.h"

LightCubePass::LightCubePass(unsigned int width, unsigned int height) : RenderPass(width, height)
{
	lightCubeShader = std::make_unique<Shader>("src/shaders/lightcube.vert", "src/shaders/lightcube.frag");
	lightCubeShader->use();
	for (const auto& light : scene->lights.pointLights) {
		lightCubeShader->setMat4("model", glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.25f)), glm::vec3(light.pos)));
		lightCubeShader->setVec3("lightColor", light.color);
	}
	// bind matrix uniform block
	lightCubeShader->bindUniformBlock("Matrices", 0);
	lightCubeShader->bindUniformBlock("Lights", 1);
}

LightCubePass::~LightCubePass()
{
}

void LightCubePass::Render()
{
	lightCubeShader->use();
	for (unsigned int i = 0; i < (int)(sizeof(SceneLights::pointLights) / sizeof(PointLight)); i++) {
		lightCubeShader->setInt("lightIndex", i);
		RenderCube();
	}
}

void LightCubePass::ResizeBuffers(unsigned int width, unsigned int height)
{
}
